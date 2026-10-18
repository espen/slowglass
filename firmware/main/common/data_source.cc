/**
 * @file data_source.cc
 * @brief Registry for periodic network data sources
 */

#include "data_source.h"

#include <esp_log.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <atomic>
#include <cstring>
#include <mutex>
#include <string>
#include <vector>

#include "settings.h"

static const char* kTag = "DataSource";
static const char* kNvsNamespace = "datasrc";
static constexpr uint32_t kFetchTaskStack = 16384;

namespace {

struct Entry {
    DataSourceDef def;
    esp_timer_handle_t timer = nullptr;
    std::atomic<bool> in_progress{false};
    std::atomic<int64_t> last_fetch_us{-1};  // start of last fetch; -1 = never
    int64_t armed_period_us = 0;              // guarded by s_mutex

    explicit Entry(const DataSourceDef& d) : def(d) {}
};

// Registration happens during single-threaded init; fetches only read the list.
std::vector<Entry*>& Registry() {
    static std::vector<Entry*> registry;
    return registry;
}

bool s_started = false;
// Only the source behind the on-screen page polls; empty = none.
std::string s_active;
std::mutex s_mutex;  // guards s_started, s_active and timer arming

Entry* Find(const char* name) {
    if (!name) return nullptr;
    for (auto* e : Registry()) {
        if (strcmp(e->def.name, name) == 0) return e;
    }
    return nullptr;
}

void FetchTask(void* arg) {
    auto* entry = static_cast<Entry*>(arg);
    if (entry->def.fetch) {
        entry->def.fetch();
    }
    entry->in_progress.store(false, std::memory_order_release);
    vTaskDelete(nullptr);
}

bool StartFetch(Entry* entry) {
    bool expected = false;
    if (!entry->in_progress.compare_exchange_strong(expected, true)) {
        return false;  // already fetching
    }
    char task_name[24];
    snprintf(task_name, sizeof(task_name), "ds_%s", entry->def.name);
    if (xTaskCreate(FetchTask, task_name, kFetchTaskStack, entry, 5, nullptr) != pdPASS) {
        ESP_LOGE(kTag, "Failed to create fetch task for %s", entry->def.name);
        entry->in_progress.store(false, std::memory_order_release);
        return false;
    }
    entry->last_fetch_us.store(esp_timer_get_time(), std::memory_order_release);
    return true;
}

}  // namespace

void data_source_register(const DataSourceDef& def) {
    if (!def.name || !def.name[0] || !def.fetch) {
        ESP_LOGE(kTag, "Rejecting invalid data source registration");
        return;
    }
    if (Find(def.name)) {
        ESP_LOGW(kTag, "Data source '%s' already registered", def.name);
        return;
    }
    Registry().push_back(new Entry(def));
    ESP_LOGI(kTag, "Registered data source '%s' (default %d min)",
             def.name, def.default_interval_minutes);
}

int data_source_interval_minutes(const char* name) {
    Entry* e = Find(name);
    if (!e) return 0;
    Settings nvs(kNvsNamespace, false);
    int minutes = nvs.GetInt(e->def.name, e->def.default_interval_minutes);
    if (minutes < 0) minutes = 0;
    return minutes;
}

namespace {

// Caller holds s_mutex. Stops every inactive source's timer; arms the active
// one and fetches it if its data is older than its interval (or missing).
void ApplyActiveLocked() {
    for (auto* e : Registry()) {
        const bool active = s_started && s_active == e->def.name;
        const int minutes = active ? data_source_interval_minutes(e->def.name) : 0;
        if (minutes <= 0) {
            if (e->timer != nullptr && esp_timer_is_active(e->timer)) {
                esp_timer_stop(e->timer);
                ESP_LOGI(kTag, "Paused '%s'", e->def.name);
            }
            continue;
        }
        if (e->timer == nullptr) {
            esp_timer_create_args_t args = {};
            args.callback = [](void* arg) { StartFetch(static_cast<Entry*>(arg)); };
            args.arg = e;
            args.dispatch_method = ESP_TIMER_TASK;
            args.name = e->def.name;
            args.skip_unhandled_events = true;
            if (esp_timer_create(&args, &e->timer) != ESP_OK) {
                ESP_LOGE(kTag, "Failed to create timer for %s", e->def.name);
                continue;
            }
        }
        const int64_t period_us = (int64_t)minutes * 60 * 1000 * 1000;
        if (esp_timer_is_active(e->timer) && e->armed_period_us != period_us) {
            esp_timer_stop(e->timer);  // interval changed in NVS: re-arm
        }
        if (!esp_timer_is_active(e->timer)) {
            esp_timer_start_periodic(e->timer, period_us);
            e->armed_period_us = period_us;
            ESP_LOGI(kTag, "Started '%s' (every %d min)", e->def.name, minutes);
        }
        const int64_t last = e->last_fetch_us.load(std::memory_order_acquire);
        if (last < 0 || esp_timer_get_time() - last >= period_us) {
            StartFetch(e);
        }
    }
}

}  // namespace

void data_sources_start() {
    // Idempotent: on re-call (network back, or a source registered after the
    // first start, e.g. MakePlans pairing completing mid-session) this arms
    // the active source if needed and refreshes it if stale.
    std::lock_guard<std::mutex> lock(s_mutex);
    s_started = true;
    ApplyActiveLocked();
}

void data_sources_reload() {
    std::lock_guard<std::mutex> lock(s_mutex);
    ApplyActiveLocked();
}

void data_sources_set_active(const char* name) {
    std::lock_guard<std::mutex> lock(s_mutex);
    const std::string next = name ? name : "";
    if (next == s_active) return;
    s_active = next;
    ESP_LOGI(kTag, "Active source: '%s'", s_active.c_str());
    ApplyActiveLocked();
}

bool data_sources_busy() {
    for (auto* e : Registry()) {
        if (e->in_progress.load(std::memory_order_acquire)) return true;
    }
    return false;
}

size_t data_sources_count() {
    return Registry().size();
}

const char* data_source_name(size_t index) {
    if (index >= Registry().size()) return nullptr;
    return Registry()[index]->def.name;
}

bool data_source_fetch_now(const char* name) {
    Entry* e = Find(name);
    if (!e || data_source_interval_minutes(name) <= 0) return false;
    return StartFetch(e);
}

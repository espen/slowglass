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

    explicit Entry(const DataSourceDef& d) : def(d) {}
};

// Registration happens during single-threaded init; fetches only read the list.
std::vector<Entry*>& Registry() {
    static std::vector<Entry*> registry;
    return registry;
}

bool s_started = false;

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

int data_sources_min_interval_minutes(int fallback_minutes) {
    int best = 0;
    for (auto* e : Registry()) {
        const int m = data_source_interval_minutes(e->def.name);
        if (m > 0 && (best == 0 || m < best)) best = m;
    }
    return best > 0 ? best : fallback_minutes;
}

void data_sources_start() {
    // Idempotent: on re-call (network back, or a source registered after the
    // first start, e.g. MakePlans pairing completing mid-session) this arms
    // timers that are still missing and refreshes everything enabled.
    const bool first_start = !s_started;
    s_started = true;

    for (auto* e : Registry()) {
        const int minutes = data_source_interval_minutes(e->def.name);
        if (minutes <= 0) {
            if (first_start) {
                ESP_LOGI(kTag, "Data source '%s' disabled by config", e->def.name);
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
            if (esp_timer_create(&args, &e->timer) == ESP_OK) {
                esp_timer_start_periodic(e->timer, (int64_t)minutes * 60 * 1000 * 1000);
                ESP_LOGI(kTag, "Started '%s' (every %d min)", e->def.name, minutes);
            } else {
                ESP_LOGE(kTag, "Failed to create timer for %s", e->def.name);
            }
        }
        StartFetch(e);
    }
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

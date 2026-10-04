/**
 * @file widget_registry.cc
 * @brief Widget registry implementation
 */

#include "widgets/widget_registry.h"

#include "ui/renderers/rawdraw/page_renderer.h"

#include <esp_log.h>

#include <algorithm>
#include <cstring>
#include <memory>

namespace widgets {

namespace {

const char* kTag = "WidgetRegistry";

struct RendererSlot {
    ui::RawDrawPageId id;
    std::unique_ptr<rawdraw::PageRenderer> renderer;
};

std::vector<WidgetDef>& Defs() {
    static std::vector<WidgetDef> defs;
    return defs;
}

std::vector<RendererSlot>& Renderers() {
    static std::vector<RendererSlot> renderers;
    return renderers;
}

void EnsureRegistered() {
    static bool done = false;
    if (done) return;
    done = true;  // set first: RegisterAll() -> Register() must not recurse
    RegisterAll();
    ESP_LOGI(kTag, "%d widgets registered", static_cast<int>(Defs().size()));
}

}  // namespace

void Register(WidgetDef def) {
    if (def.name == nullptr || def.pages.empty()) {
        ESP_LOGW(kTag, "Rejecting widget with no name or no pages");
        return;
    }
    for (const auto& existing : Defs()) {
        if (std::strcmp(existing.name, def.name) == 0) {
            ESP_LOGW(kTag, "Duplicate widget '%s' ignored", def.name);
            return;
        }
    }
    Defs().push_back(std::move(def));
}

const std::vector<WidgetDef>& All() {
    EnsureRegistered();
    return Defs();
}

const WidgetPage* FindPage(ui::RawDrawPageId id) {
    for (const auto& def : All()) {
        for (const auto& page : def.pages) {
            if (page.id == id) return &page;
        }
    }
    return nullptr;
}

const WidgetPage* FindPageByName(const char* name) {
    if (name == nullptr) return nullptr;
    for (const auto& def : All()) {
        for (const auto& page : def.pages) {
            if (page.config_name != nullptr && std::strcmp(page.config_name, name) == 0) {
                return &page;
            }
        }
    }
    return nullptr;
}

void CreateRenderers() {
    Renderers().clear();
    for (const auto& def : All()) {
        for (const auto& page : def.pages) {
            if (page.create_renderer == nullptr) continue;
            rawdraw::PageRenderer* renderer = page.create_renderer();
            if (renderer == nullptr) {
                ESP_LOGW(kTag, "Widget '%s' returned null renderer", def.name);
                continue;
            }
            Renderers().push_back({page.id, std::unique_ptr<rawdraw::PageRenderer>(renderer)});
        }
    }
    ESP_LOGI(kTag, "%d widget renderers created", static_cast<int>(Renderers().size()));
}

rawdraw::PageRenderer* RendererFor(ui::RawDrawPageId id) {
    for (const auto& slot : Renderers()) {
        if (slot.id == id) return slot.renderer.get();
    }
    return nullptr;
}

void InitAll() {
    for (const auto& def : All()) {
        if (def.init != nullptr) def.init();
    }
}

void OnNetworkUp(const WidgetContext& ctx) {
    for (const auto& def : All()) {
        if (def.on_network_up != nullptr) def.on_network_up(ctx);
    }
}

size_t HttpHandlerCount() {
    size_t count = 0;
    for (const auto& def : All()) {
        count += def.http_handler_count;
    }
    return count;
}

bool RegisterHttpHandlers(httpd_handle_t server) {
    for (const auto& def : All()) {
        if (def.register_http != nullptr && !def.register_http(server)) {
            ESP_LOGE(kTag, "Widget '%s' failed to register HTTP handlers", def.name);
            return false;
        }
    }
    return true;
}

void OnLanServerStarted(const std::string& url_base) {
    for (const auto& def : All()) {
        if (def.on_lan_server_started != nullptr) def.on_lan_server_started(url_base);
    }
}

int MinutesToNextWake(int fallback_minutes) {
    int minutes = fallback_minutes;
    for (const auto& def : All()) {
        if (def.minutes_to_next_wake != nullptr) {
            minutes = std::min(minutes, def.minutes_to_next_wake(fallback_minutes));
        }
    }
    return minutes;
}

}  // namespace widgets

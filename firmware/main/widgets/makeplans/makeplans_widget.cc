/**
 * @file makeplans_widget.cc
 * @brief MakePlans door-sign widget
 *
 * Owns the full-bleed door-sign page, the "makeplans" data source (5-minute
 * default, registered by makeplans_api_init once paired), the pairing web
 * UI (/makeplans + /api/makeplans), and the battery-wake hook that flips the
 * sign exactly on booking boundaries.
 */

#include "widgets/widget.h"

#include "common/makeplans_api.h"
#include "ui/renderers/rawdraw/makeplans_renderer.h"
#include "widgets/makeplans/makeplans_http.h"

namespace widgets {

namespace {

rawdraw::MakePlansRenderer* s_renderer = nullptr;

void OnNetworkUp(const WidgetContext& ctx) {
    if (makeplans_api_is_ready()) return;
    makeplans_api_init([ctx](const MakePlansSchedule& schedule) {
        if (s_renderer != nullptr) {
            s_renderer->Update(schedule);
        }
        if (ctx.current_page && ctx.current_page() == ui::RawDrawPageId::MakePlans &&
            ctx.request_active_page_refresh) {
            ctx.request_active_page_refresh();
        }
    });
}

void OnLanServerStarted(const std::string& url_base) {
    if (s_renderer != nullptr) {
        s_renderer->SetLanUrl(url_base + "/makeplans");
    }
}

}  // namespace

void RegisterMakePlansWidget() {
    WidgetDef def;
    def.name = "makeplans";
    def.pages = {
        {ui::RawDrawPageId::MakePlans, "doorsign", "Door Sign", nullptr,
         []() -> rawdraw::PageRenderer* {
             s_renderer = new rawdraw::MakePlansRenderer();
             return s_renderer;
         }},
    };
    def.on_network_up = OnNetworkUp;
    def.register_http = makeplans_register_http;
    def.http_handler_count = kMakePlansHttpHandlerCount;
    def.on_lan_server_started = OnLanServerStarted;
    // Wake at the next booking boundary if that comes sooner than the poll
    // interval, so the panel flips on time instead of up to a poll late.
    def.minutes_to_next_wake = makeplans_minutes_to_next_boundary;
    Register(std::move(def));
}

}  // namespace widgets

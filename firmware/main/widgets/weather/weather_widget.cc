/**
 * @file weather_widget.cc
 * @brief Weather widget: MET Norway forecast dashboard (+ detail page)
 *
 * Owns the full-bleed weather dashboard and its hourly-detail page, and the
 * "weather" data source (60-minute default, registered by weather_api_init).
 */

#include "widgets/widget.h"

#include "widgets/weather/weather_api.h"
#include "widgets/weather/weather_http.h"
#include "widgets/weather/weather_renderer.h"
#include "widgets/weather/weather_detail_renderer.h"

namespace widgets {

namespace {

// Raw pointers to the renderers owned by the widget registry, so the data
// callback below can feed them without going through the UI manager.
rawdraw::WeatherRenderer* s_renderer = nullptr;
rawdraw::WeatherDetailRenderer* s_detail_renderer = nullptr;

// Seed a freshly created renderer with the forecast persisted across the
// deep-sleep reboot, so the first paint shows the previous fetch instead of
// "Waiting for weather data" (shown only on a true first-ever render).
void SeedFromCache() {
    WeatherData cached;
    if (!weather_api_load_cached(&cached)) return;
    if (s_renderer != nullptr) {
        s_renderer->SetCityName(cached.city.c_str());
        s_renderer->Update(cached);
    }
    if (s_detail_renderer != nullptr) {
        s_detail_renderer->Update(cached);
    }
}

void OnNetworkUp(const WidgetContext& ctx) {
    if (weather_api_is_ready()) return;
    weather_api_init([ctx](const WeatherData& weather) {
        if (s_renderer != nullptr) {
            s_renderer->SetCityName(weather.city.c_str());
            s_renderer->Update(weather);
        }
        if (s_detail_renderer != nullptr) {
            s_detail_renderer->Update(weather);
        }
        // Queue a re-render if a weather page is on screen (the 1 s UI pump
        // picks it up; request_full_refresh alone only sets a flag and never
        // repaints, which left the panel on the waiting screen).
        if (ctx.current_page && ctx.request_active_page_refresh) {
            const ui::RawDrawPageId page = ctx.current_page();
            if (page == ui::RawDrawPageId::Weather ||
                page == ui::RawDrawPageId::WeatherDetail) {
                ctx.request_active_page_refresh();
            }
        }
    });
}

}  // namespace

void RegisterWeatherWidget() {
    WidgetDef def;
    def.name = "weather";
    def.pages = {
        {ui::RawDrawPageId::Weather, "weather", "Weather", nullptr,
         []() -> rawdraw::PageRenderer* {
             s_renderer = new rawdraw::WeatherRenderer();
             SeedFromCache();
             return s_renderer;
         }},
        {ui::RawDrawPageId::WeatherDetail, nullptr, "Weather Detail", nullptr,
         []() -> rawdraw::PageRenderer* {
             s_detail_renderer = new rawdraw::WeatherDetailRenderer();
             SeedFromCache();
             return s_detail_renderer;
         }},
    };
    def.on_network_up = OnNetworkUp;
    def.register_http = weather_register_http;
    def.http_handler_count = kWeatherHttpHandlerCount;
    Register(std::move(def));
}

}  // namespace widgets

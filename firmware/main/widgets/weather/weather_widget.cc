/**
 * @file weather_widget.cc
 * @brief Weather widget: MET Norway forecast dashboard (+ detail page)
 *
 * Owns the full-bleed weather dashboard and its hourly-detail page, and the
 * "weather" data source (60-minute default, registered by weather_api_init).
 */

#include "widgets/widget.h"

#include "widgets/weather/weather_api.h"
#include "widgets/weather/weather_renderer.h"
#include "widgets/weather/weather_detail_renderer.h"

namespace widgets {

namespace {

// Raw pointers to the renderers owned by the widget registry, so the data
// callback below can feed them without going through the UI manager.
rawdraw::WeatherRenderer* s_renderer = nullptr;
rawdraw::WeatherDetailRenderer* s_detail_renderer = nullptr;

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
        if (ctx.current_page && ctx.current_page() == ui::RawDrawPageId::Weather &&
            ctx.request_full_refresh) {
            ctx.request_full_refresh();
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
             return s_renderer;
         }},
        {ui::RawDrawPageId::WeatherDetail, nullptr, "Weather Detail", nullptr,
         []() -> rawdraw::PageRenderer* {
             s_detail_renderer = new rawdraw::WeatherDetailRenderer();
             return s_detail_renderer;
         }},
    };
    def.on_network_up = OnNetworkUp;
    Register(std::move(def));
}

}  // namespace widgets

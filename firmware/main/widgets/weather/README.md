# Weather widget

Full-bleed weather dashboard backed by the free MET Norway Locationforecast
API (no key; identifying User-Agent per their terms), with location from IP
geolocation (fallback: New York) and an hourly-detail page.

## Files

- `weather_widget.cc` — the `WidgetDef`: both pages, network-up wiring.
- `weather_api.{h,cc}` — forecast client; registers the `weather` data
  source (60-minute default).
- `weather_renderer.{h,cc}` — the dashboard page.
- `weather_detail_renderer.{h,cc}` — hourly detail page (no quick-switch
  entry; reachable programmatically).
- `weather_card.{h,cc}` — reusable weather card drawing helpers.

## Configuration

Refresh interval override: NVS `datasrc/weather` (minutes, 0 = disabled).
Planned: NVS `weather/lat|lon|city` location override (see
.agents/plans/platform-refactor.md).

## Platform dependencies

`widgets/widget.h`, `common/data_source.h`, `settings.h`, the rawdraw
drawing library, ESP-IDF (http client, cJSON, esp_timer).

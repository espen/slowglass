# Weather widget

Full-bleed weather dashboard backed by the free MET Norway Locationforecast
API (no key; identifying User-Agent per their terms), with location from IP
geolocation (fallback: New York) and an hourly-detail page.

## Files

- `weather_widget.cc` — the `WidgetDef`: both pages, network-up wiring,
  HTTP registration.
- `weather_api.{h,cc}` — forecast client; registers the `weather` data
  source (60-minute default); location override storage.
- `weather_http.{h,cc}` — `/weather` location config page + `GET/POST
  /api/weather`.
- `weather_renderer.{h,cc}` — the dashboard page.
- `weather_detail_renderer.{h,cc}` — hourly detail page (no quick-switch
  entry; reachable programmatically).
- `weather_card.{h,cc}` — reusable weather card drawing helpers.

## Configuration

Refresh interval override: NVS `datasrc/weather` (minutes, 0 = disabled).

Location override (wins over IP geolocation — essential behind a VPN):
NVS `weather/lat|lon|city|tzmin`, set from `http://<device>/weather` or
`POST /api/weather {"lat":59.91,"lon":10.75,"city":"Oslo","utc_offset_min":120}`
(`{"clear":true}` returns to IP mode; `utc_offset_min` drives
today/tomorrow bucketing and is prefilled from the browser's timezone).

## Platform dependencies

`widgets/widget.h`, `common/data_source.h`, `settings.h`, the rawdraw
drawing library, ESP-IDF (http client, cJSON, esp_timer).

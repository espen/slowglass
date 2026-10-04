# MakePlans door sign widget

Shows today's schedule for one MakePlans resource (room) on the e-paper
panel as a full-bleed door sign, with occupancy-boundary wakeups on battery
and a passive NFC tag that opens the room's booking page.

## Files

- `makeplans_widget.cc` — the `WidgetDef`: page registration, network-up
  wiring, HTTP registration, LAN-URL hook, battery-wake hook.
- `makeplans_api.{h,cc}` — room display API client (pairing + 5-minute
  schedule polling via the `makeplans` data source).
- `makeplans_renderer.{h,cc}` — the door-sign page renderer.
- `makeplans_http.{h,cc}` — `/makeplans` pairing page + `GET/POST
  /api/makeplans`.

## Configuration

NVS namespace `makeplans`: `account`, `resource`, `cookie` (pairing),
`room`, `tzoff` (cached), `nfc_url` (override), `nfcdone`.
Pair via `http://<device>/makeplans` (LAN or AP mode) with a 6-digit code
from MakePlans admin → resource → Room display. Make it the home page with
`POST /api/pages {"home":"doorsign"}`.

## Platform dependencies

`widgets/widget.h`, `common/data_source.h`, `common/httpd_helpers.h`,
`common/nfc_tag.h`, `settings.h` (NVS), the rawdraw drawing library,
ESP-IDF (http client/server, cJSON, esp_log). No board or application
includes — see `scripts/check_widget_deps.sh`.

# Porting a widget to another firmware

Each directory under `firmware/main/widgets/` is self-contained: copy it,
provide the small set of contracts below, and the widget runs. Nothing in a
widget references `application.h`, `boards/*`, or the UI manager
(`scripts/check_widget_deps.sh` enforces this).

## What the target firmware must provide

1. **The widget contract** — `widgets/widget.h` + `widgets/widget_registry.{h,cc}`
   and `ui/page_id.h`. These are tiny and have no further dependencies;
   copy them as-is and call the widget's `Register*Widget()` from your own
   init, then drive the hooks:
   - call `widgets::CreateRenderers()` at UI startup and render via
     `widgets::RendererFor(page)`,
   - call `widgets::OnNetworkUp(ctx)` when the network connects,
   - call `widgets::RegisterHttpHandlers(server)` if you run esp_http_server,
   - feed `widgets::MinutesToNextWake()` into your sleep scheduling.

2. **The rawdraw drawing library** — `main/rawdraw/` (1bpp framebuffer
   primitives, fonts, theme) and the `rawdraw::PageRenderer` base class
   (`ui/renderers/rawdraw/page_renderer.h`). This is the largest
   dependency; it is plain C++ over a byte buffer with no board coupling.

3. **Platform services** (all small, in `main/common/` + `main/`):
   - `common/data_source.{h,cc}` — periodic fetch registry (esp_timer +
     task spawning; pure ESP-IDF).
   - `settings.{h,cc}` — the NVS key/value wrapper.
   - `common/httpd_helpers.{h,cc}` — JSON request/response helpers
     (only for widgets with HTTP endpoints).
   - `common/nfc_tag.h` — only for the MakePlans widget's NFC feature;
     stub it to return false on hardware without a tag.

4. **ESP-IDF** — esp_http_client (+ crt bundle), esp_http_server, cJSON,
   esp_timer, esp_log, NVS.

## What stays behind

The host firmware's page navigation, status bar, button routing, power
management, and web server remain its own; widgets only plug renderers and
hooks into them. Fonts referenced by a renderer (`SourceHanSansSC_*`,
`font_zectrix_*`) must exist in the target or be swapped in the renderer.

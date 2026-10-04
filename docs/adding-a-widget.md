# Adding a widget

A widget is a self-contained feature directory under `firmware/main/widgets/`
that owns its UI page(s), data fetching, and HTTP endpoints. The platform
discovers it only through the widget registry.

## Checklist

1. **Page id** — add an entry to the enum in `main/ui/page_id.h`.
   (An id whose widget isn't compiled in is harmless.)

2. **Directory** — create `main/widgets/<name>/` containing at least
   `<name>_widget.cc`:

   ```cpp
   #include "widgets/widget.h"
   #include "widgets/<name>/<name>_renderer.h"

   namespace widgets {
   namespace {
   rawdraw::MyRenderer* s_renderer = nullptr;  // if hooks need typed access
   }

   void RegisterMyWidget() {
       WidgetDef def;
       def.name = "<name>";
       def.pages = {
           {ui::RawDrawPageId::MyPage, "<configname>", "My Title", nullptr,
            []() -> rawdraw::PageRenderer* {
                s_renderer = new rawdraw::MyRenderer();
                return s_renderer;
            }},
       };
       // Optional hooks, all nullptr by default:
       // def.init                 — boot-time setup
       // def.on_network_up        — wire API callback -> renderer (idempotent!)
       // def.register_http + def.http_handler_count — web endpoints
       // def.on_lan_server_started — e.g. show a config URL on the page
       // def.minutes_to_next_wake — shorten battery deep-sleep
       Register(std::move(def));
   }
   }  // namespace widgets
   ```

   The renderer subclasses `rawdraw::PageRenderer`
   (`ui/renderers/rawdraw/page_renderer.h`). Override `WantsFullBleed()`
   for a chrome-free dashboard and `GetStatusBarCentralText()` for a
   custom status-bar line.

3. **Register** — add `Register<Name>Widget();` (with declaration) to
   `main/widgets/widgets_init.cc`, inside a `#if CONFIG_WIDGET_<NAME>`
   guard.

4. **Kconfig** — add a `config WIDGET_<NAME>` bool (default y) to the
   "Widgets" menu in `main/Kconfig.projbuild`.

5. **Build** — add the sources to `main/CMakeLists.txt` inside the
   matching `if(CONFIG_WIDGET_<NAME>)` block. Forgetting a source file
   shows up as a linker error.

6. **Data fetching** — if the widget polls a network API, register a
   data source (`common/data_source.h`) from its init/api code:
   `data_source_register({"<name>", minutes, FetchFn});`. The battery
   sleep cadence and the `/api/pages` intervals endpoint pick it up
   automatically, and per-source NVS overrides come free.

7. **Verify** — `scripts/check_widget_deps.sh` must stay green: widget
   code never includes `application.h`, `boards/*`, or UI-manager
   internals. `idf.py menuconfig` → Widgets shows the new toggle.

## What you get for free

- Page config: the widget's `config_name` works in
  `POST /api/pages {"home":..., "enabled":[...]}` and the quick-switch
  menu, with no table edits.
- Status bar title, quick-switch entry (plus optional icon).
- HTTP endpoints served in both AP-setup and LAN mode, with
  `max_uri_handlers` sized from the declared handler counts.
- Compile-time exclusion: disabling the Kconfig option removes the
  widget's code, pages, endpoints, data source, and battery wakeups.

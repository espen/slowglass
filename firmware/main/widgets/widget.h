/**
 * @file widget.h
 * @brief The widget contract — everything a self-contained widget provides
 *
 * A widget is a feature that owns one or more UI pages plus whatever data
 * fetching, HTTP endpoints, and config it needs: weather, the MakePlans door
 * sign, news, a calendar. Each widget lives in its own directory under
 * main/widgets/<name>/ and plugs into the platform exclusively through this
 * struct; the platform never names a widget directly.
 *
 * PORTABILITY RULE: widget code may include only
 *   - widgets/widget.h (this file)
 *   - ui/page_id.h
 *   - the rawdraw drawing library and ui/renderers/rawdraw/page_renderer.h
 *   - common/data_source.h, common/httpd_helpers.h, settings.h (NVS)
 *   - ESP-IDF headers
 * Never application.h, board headers, or ui/rawdraw_ui_manager.h. Moving a
 * widget
 * to another firmware then means copying its directory and providing these
 * few contracts.
 *
 * Adding a widget:
 *   1. Create main/widgets/<name>/<name>_widget.cc defining
 *      widgets::Register<Name>Widget() that calls widgets::Register(def).
 *   2. Add that call to widgets/widgets_init.cc (one line, Kconfig-guarded).
 *   3. Add the sources to main/CMakeLists.txt (Kconfig-guarded block).
 *   4. Add the page id to ui/page_id.h.
 */

#ifndef WIDGETS_WIDGET_H
#define WIDGETS_WIDGET_H

#include "ui/page_id.h"

#include <esp_http_server.h>

#include <functional>
#include <string>
#include <vector>

namespace rawdraw {
class PageRenderer;
}

namespace widgets {

/**
 * @brief Narrow window into the platform, handed to widget hooks.
 *
 * Deliberately tiny: this is the entire UI surface a widget may touch from
 * its data callbacks. Widgets that need their own renderer keep the pointer
 * they returned from create_renderer.
 */
struct WidgetContext {
    std::function<ui::RawDrawPageId()> current_page;
    std::function<void()> request_full_refresh;         // full EPD refresh flag
    std::function<void()> request_active_page_refresh;  // queued re-render
};

/**
 * @brief One UI page a widget contributes.
 */
struct WidgetPage {
    ui::RawDrawPageId id;
    /** Stable name for NVS page config / the /api/pages endpoint / the
     *  quick-switch menu. nullptr for detail pages that are only reachable
     *  from the widget's own main page. */
    const char* config_name;
    /** Status-bar / quick-switch title ("Door Sign"). */
    const char* title;
    /** Quick-switch icon glyph (FontAwesome UTF-8 string) or nullptr. */
    const char* icon;
    /** Create the page renderer. Called once at UI startup; the registry
     *  owns the returned object. Store the pointer in a file-static if the
     *  widget's hooks need typed access to it. */
    rawdraw::PageRenderer* (*create_renderer)();
};

/**
 * @brief A widget's full registration record.
 *
 * All hooks are optional (nullptr = not needed). Hooks must be idempotent
 * where the platform may call them repeatedly (on_network_up fires on every
 * reconnect).
 */
struct WidgetDef {
    const char* name = nullptr;
    std::vector<WidgetPage> pages;

    /** Boot-time setup before networking (register data sources, read NVS). */
    void (*init)() = nullptr;

    /** Network became available (also on reconnect). Wire API callbacks to
     *  the renderer here; guard one-time init internally. */
    void (*on_network_up)(const WidgetContext& ctx) = nullptr;

    /** Register this widget's HTTP endpoints on the device's web server
     *  (runs in both AP-setup and LAN mode). Return false on failure. */
    bool (*register_http)(httpd_handle_t server) = nullptr;
    /** Exact number of URI handlers register_http installs — used to size
     *  max_uri_handlers so handlers can never be silently dropped. */
    size_t http_handler_count = 0;

    /** The LAN web server came up at url_base ("http://192.168.1.60"). */
    void (*on_lan_server_started)(const std::string& url_base) = nullptr;

    /** Battery deep-sleep hook: minutes until this widget next needs the
     *  device awake, or fallback_minutes if nothing sooner. The platform
     *  takes the minimum across widgets and data sources. */
    int (*minutes_to_next_wake)(int fallback_minutes) = nullptr;
};

/** Register a widget. Call from widgets_init.cc; duplicate names ignored. */
void Register(WidgetDef def);

}  // namespace widgets

#endif  // WIDGETS_WIDGET_H

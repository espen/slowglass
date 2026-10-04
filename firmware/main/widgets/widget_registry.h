/**
 * @file widget_registry.h
 * @brief Central registry of compiled-in widgets
 *
 * The platform (UI manager, page config, web server, application) talks to
 * widgets only through these lookups — never by naming a widget directly.
 * Registration itself happens in widgets_init.cc, one explicit line per
 * widget, so the linker keeps every widget object file and a widget can be
 * excluded at compile time by omitting that line.
 */

#ifndef WIDGETS_WIDGET_REGISTRY_H
#define WIDGETS_WIDGET_REGISTRY_H

#include "widgets/widget.h"

#include <cstddef>

namespace widgets {

/** Implemented in widgets_init.cc: registers every compiled-in widget.
 *  Called lazily by the accessors below; safe to call repeatedly. */
void RegisterAll();

/** All registered widgets, registration order. */
const std::vector<WidgetDef>& All();

/** Page lookup by id / config name. nullptr when no widget owns it. */
const WidgetPage* FindPage(ui::RawDrawPageId id);
const WidgetPage* FindPageByName(const char* name);

/** (Re)create every widget page renderer. Called once by the UI manager's
 *  constructor; the registry owns the renderer objects. */
void CreateRenderers();

/** Renderer for a widget page, or nullptr (core page / not compiled in). */
rawdraw::PageRenderer* RendererFor(ui::RawDrawPageId id);

/** Run every widget's init() hook (boot time, before networking). */
void InitAll();

/** Run every widget's on_network_up() hook (idempotent per widget). */
void OnNetworkUp(const WidgetContext& ctx);

/** Sum of all widgets' http_handler_count (for max_uri_handlers sizing). */
size_t HttpHandlerCount();

/** Run every widget's register_http(). Returns false on first failure. */
bool RegisterHttpHandlers(httpd_handle_t server);

/** Notify widgets that the LAN web server is up at url_base. */
void OnLanServerStarted(const std::string& url_base);

/** Minimum of all widgets' minutes_to_next_wake(fallback). */
int MinutesToNextWake(int fallback_minutes);

}  // namespace widgets

#endif  // WIDGETS_WIDGET_REGISTRY_H

/**
 * @file makeplans_http.h
 * @brief MakePlans door-sign HTTP endpoints (pairing page + config API)
 */

#ifndef WIDGETS_MAKEPLANS_HTTP_H
#define WIDGETS_MAKEPLANS_HTTP_H

#include <esp_http_server.h>

#include <cstddef>

namespace widgets {

/** URI handlers installed by makeplans_register_http (for server sizing). */
constexpr size_t kMakePlansHttpHandlerCount = 3;

/** Register /makeplans (pairing page) and GET/POST /api/makeplans. */
bool makeplans_register_http(httpd_handle_t server);

}  // namespace widgets

#endif  // WIDGETS_MAKEPLANS_HTTP_H

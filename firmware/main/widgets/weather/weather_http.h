/**
 * @file weather_http.h
 * @brief Weather widget HTTP endpoints (location config page + API)
 */

#ifndef WIDGETS_WEATHER_HTTP_H
#define WIDGETS_WEATHER_HTTP_H

#include <esp_http_server.h>

#include <cstddef>

namespace widgets {

/** URI handlers installed by weather_register_http (for server sizing). */
constexpr size_t kWeatherHttpHandlerCount = 3;

/** Register /weather (location config page) and GET/POST /api/weather. */
bool weather_register_http(httpd_handle_t server);

}  // namespace widgets

#endif  // WIDGETS_WEATHER_HTTP_H

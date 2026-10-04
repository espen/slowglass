/**
 * @file httpd_helpers.h
 * @brief Small shared helpers for esp_http_server request handlers
 *
 * Extracted from ap_transfer_server so widget HTTP endpoints can use the
 * same JSON request/response conventions without depending on the server.
 */

#ifndef COMMON_HTTPD_HELPERS_H
#define COMMON_HTTPD_HELPERS_H

#include <cJSON.h>
#include <esp_http_server.h>

namespace web {

/** Read and parse a JSON request body (max 2KB). Caller owns the result;
 *  returns nullptr on empty/oversized/invalid body. */
cJSON* ReadJsonBody(httpd_req_t* req);

/** Force-close the request's socket after responding. The embedded server
 *  has few sockets; lingering keep-alive connections starve it. */
void CloseCurrentSession(httpd_req_t* req);

/** Send a JSON response with Connection: close and drop the session. */
void SendJson(httpd_req_t* req, const char* json);

}  // namespace web

#endif  // COMMON_HTTPD_HELPERS_H

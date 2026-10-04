/**
 * @file httpd_helpers.cc
 * @brief Shared esp_http_server handler helpers
 */

#include "common/httpd_helpers.h"

#include <esp_log.h>

#include <cstdlib>

namespace web {

namespace {
const char* kTag = "HttpdHelpers";
}

cJSON* ReadJsonBody(httpd_req_t* req) {
    if (!req || req->content_len == 0 || req->content_len > 2048) return nullptr;
    char* buf = static_cast<char*>(calloc(1, req->content_len + 1));
    if (!buf) return nullptr;
    size_t received = 0;
    while (received < req->content_len) {
        int ret = httpd_req_recv(req, buf + received, req->content_len - received);
        if (ret <= 0) {
            free(buf);
            return nullptr;
        }
        received += static_cast<size_t>(ret);
    }
    cJSON* root = cJSON_Parse(buf);
    free(buf);
    return root;
}

void CloseCurrentSession(httpd_req_t* req) {
    if (!req || !req->handle) return;
    const int sockfd = httpd_req_to_sockfd(req);
    if (sockfd < 0) return;
    esp_err_t err = httpd_sess_trigger_close(req->handle, sockfd);
    if (err != ESP_OK && err != ESP_ERR_NOT_FOUND) {
        ESP_LOGW(kTag, "httpd_sess_trigger_close(%d) failed: %s",
                 sockfd, esp_err_to_name(err));
    }
}

void SendJson(httpd_req_t* req, const char* json) {
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Connection", "close");
    httpd_resp_send(req, json, HTTPD_RESP_USE_STRLEN);
    CloseCurrentSession(req);
}

}  // namespace web

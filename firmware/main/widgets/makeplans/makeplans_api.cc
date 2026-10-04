/**
 * @file makeplans_api.cc
 * @brief MakePlans room display client implementation
 *
 * Data flow (https://developer.makeplans.com/guide/room-display/):
 *  - Fetch: GET https://{account}.makeplans.com/api/v1/bookings/room_display/
 *    events?resource_id={id} with the pairing cookie. 200 = JSON array of
 *    {"booking":{..}} / {"event":{..}} items from the start of today;
 *    404 = cookie missing/revoked (surface as "unpaired").
 *  - Pairing: GET /room_display/{id}/setup (CSRF token + session cookie),
 *    POST /room_display/{id}/verify_token with the 6-digit code; a 302 with
 *    Set-Cookie room_display_token_{id}=... means success. The cookie value
 *    is opaque — stored verbatim in NVS and replayed on every fetch.
 *
 * All comparisons use UTC epochs (feed timestamps carry their own offsets),
 * so only a synced clock matters, not the device timezone. Display strings
 * (HH:MM) come straight from the feed and are room-local by construction.
 */

#include "makeplans_api.h"

#include "common/data_source.h"
#include "common/nfc_tag.h"
#include "settings.h"

#include <esp_log.h>
#include <esp_http_client.h>
#include <esp_heap_caps.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <cJSON.h>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <cstdio>
#include <ctime>

static const char* kTag = "MakePlans";
static const char* kUserAgent = "zectrix-note4c-doorsign/0.1 (personal devkit)";

// Pinned trust anchors for *.makeplans.com (Let's Encrypt hierarchy).
// The server presents its intermediates out of order, which the global
// esp_crt_bundle path cannot follow (mbedtls only searches FORWARD in the
// presented list for a parent). Explicit trust anchors are consulted at
// every step of the chain walk, making the presented order irrelevant.
static const char kIsrgRootsPem[] =
    // Root YE (cross-signed by ISRG Root X2; same subject+key as the self-signed root)
    "-----BEGIN CERTIFICATE-----\n"
    "MIICpjCCAiugAwIBAgIRAIchZfw0tuX7qK3Vs3BftTowCgYIKoZIzj0EAwMwTzEL\n"
    "MAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2VhcmNo\n"
    "IEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDIwHhcNMjYwNTEzMDAwMDAwWhcN\n"
    "MzIwOTAyMjM1OTU5WjAuMQswCQYDVQQGEwJVUzENMAsGA1UEChMESVNSRzEQMA4G\n"
    "A1UEAxMHUm9vdCBZRTB2MBAGByqGSM49AgEGBSuBBAAiA2IABDwS/6vhrcVqcbBo\n"
    "+wgdI3fwn9x7DNJJOY/lTOti0vkwuRN87RhEhTH17E7XyFjWsPYhIPt/wzOqxTd2\n"
    "b+4ZJNy9ID04YywF9U5zasDVyGSNErVNtz8uSGh5izW87j77GaOB6zCB6DAOBgNV\n"
    "HQ8BAf8EBAMCAQYwEwYDVR0lBAwwCgYIKwYBBQUHAwEwDwYDVR0TAQH/BAUwAwEB\n"
    "/zAdBgNVHQ4EFgQUo8gmWo6hTNA1Y/ybI8g6rlbzT1YwHwYDVR0jBBgwFoAUfEKW\n"
    "rt5LSDv6kviejM9ti6lyN5UwMgYIKwYBBQUHAQEEJjAkMCIGCCsGAQUFBzAChhZo\n"
    "dHRwOi8veDIuaS5sZW5jci5vcmcvMBMGA1UdIAQMMAowCAYGZ4EMAQIBMCcGA1Ud\n"
    "HwQgMB4wHKAaoBiGFmh0dHA6Ly94Mi5jLmxlbmNyLm9yZy8wCgYIKoZIzj0EAwMD\n"
    "aQAwZgIxAMU19WCtmxVND8UHBZRoma49Z7jPs64Dma0eTu1OChVbB/2J7GV3nvYK\n"
    "Ax54uk1G9QIxAO0miLVJu8PLNiXXXkiE/gsK3CTRTF/aeo4bMX42Zw40csRU6AC2\n"
    "6hSW1/IWaas6dg==\n"
    "-----END CERTIFICATE-----\n"
    // ISRG Root X2
    "-----BEGIN CERTIFICATE-----\n"
    "MIICGzCCAaGgAwIBAgIQQdKd0XLq7qeAwSxs6S+HUjAKBggqhkjOPQQDAzBPMQswCQYDVQQGEwJV\n"
    "UzEpMCcGA1UEChMgSW50ZXJuZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElT\n"
    "UkcgUm9vdCBYMjAeFw0yMDA5MDQwMDAwMDBaFw00MDA5MTcxNjAwMDBaME8xCzAJBgNVBAYTAlVT\n"
    "MSkwJwYDVQQKEyBJbnRlcm5ldCBTZWN1cml0eSBSZXNlYXJjaCBHcm91cDEVMBMGA1UEAxMMSVNS\n"
    "RyBSb290IFgyMHYwEAYHKoZIzj0CAQYFK4EEACIDYgAEzZvVn4CDCuwJSvMWSj5cz3es3mcFDR0H\n"
    "ttwW+1qLFNvicWDEukWVEYmO6gbf9yoWHKS5xcUy4APgHoIYOIvXRdgKam7mAHf7AlF9ItgKbppb\n"
    "d9/w+kHsOdx1ymgHDB/qo0IwQDAOBgNVHQ8BAf8EBAMCAQYwDwYDVR0TAQH/BAUwAwEB/zAdBgNV\n"
    "HQ4EFgQUfEKWrt5LSDv6kviejM9ti6lyN5UwCgYIKoZIzj0EAwMDaAAwZQIwe3lORlCEwkSHRhtF\n"
    "cP9Ymd70/aTSVaYgLXTWNLxBo1BfASdWtL4ndQavEi51mI38AjEAi/V3bNTIZargCyzuFJ0nN6T5\n"
    "U6VR5CmD1/iQMVtCnwr1/q4AaOeMSQ+2b1tbFfLn\n"
    "-----END CERTIFICATE-----\n"
    // ISRG Root X1
    "-----BEGIN CERTIFICATE-----\n"
    "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\n"
    "m+kXQ99b21/+jh5Xos1AnX5iItreGCc=\n"
    "-----END CERTIFICATE-----\n";

static const char* kNvsNamespace = "makeplans";
static const char* kKeyAccount = "account";
static const char* kKeyResource = "resource";
static const char* kKeyCookie = "cookie";
static const char* kKeyRoom = "room";
static const char* kKeyTzOffset = "tzoff";
static const char* kKeyNfcUrl = "nfc_url";      // override; "" = default URL
static const char* kKeyNfcWritten = "nfcdone";  // URL currently on the tag

// Below this the RTC clearly hasn't synced; don't judge occupied/free.
static const int64_t kMinValidEpoch = 1600000000;  // 2020-09-13

// ============================================================
// Static state
// ============================================================

static MakePlansCallback s_callback;
static bool s_initialized = false;
static bool s_registered = false;
static MakePlansSchedule s_last;
static bool s_has_data = false;
static esp_timer_handle_t s_boundary_timer = nullptr;

// ============================================================
// Occupancy boundaries (booking start/end and their -10 min warning
// points). The renderer computes state at render time, but something must
// TRIGGER a render exactly when the state flips — on USB that's this
// one-shot timer; on battery the sleep cycle asks via
// makeplans_minutes_to_next_boundary().
// ============================================================

static const int64_t kSoonWindowSec = 10 * 60;  // matches the renderer's chip

static int64_t NextBoundaryEpoch(const MakePlansSchedule& s, int64_t now) {
    int64_t best = 0;
    auto consider = [&](int64_t t) {
        if (t > now && (best == 0 || t < best)) best = t;
    };
    for (const auto& e : s.entries) {
        consider(e.start_epoch - kSoonWindowSec);
        consider(e.start_epoch);
        consider(e.end_epoch - kSoonWindowSec);
        consider(e.end_epoch);
    }
    return best;
}

static void ArmBoundaryTimer();

static void BoundaryTimerCb(void*) {
    // Re-deliver the cached schedule: the UI recomputes occupancy against
    // the clock and redraws. No network involved.
    if (s_callback && s_has_data && !s_last.unpaired) {
        ESP_LOGI(kTag, "Occupancy boundary reached; refreshing panel state");
        s_callback(s_last);
    }
    ArmBoundaryTimer();
}

static void ArmBoundaryTimer() {
    if (!s_has_data || s_last.unpaired) return;
    const int64_t now = (int64_t)time(nullptr);
    if (now < kMinValidEpoch) return;

    if (s_boundary_timer == nullptr) {
        esp_timer_create_args_t args = {};
        args.callback = BoundaryTimerCb;
        args.dispatch_method = ESP_TIMER_TASK;
        args.name = "mp_boundary";
        args.skip_unhandled_events = true;
        if (esp_timer_create(&args, &s_boundary_timer) != ESP_OK) {
            ESP_LOGE(kTag, "Failed to create boundary timer");
            return;
        }
    }
    esp_timer_stop(s_boundary_timer);

    const int64_t boundary = NextBoundaryEpoch(s_last, now);
    if (boundary == 0) return;  // nothing ahead today
    const int64_t delay_sec = boundary - now + 1;  // land just past the edge
    esp_timer_start_once(s_boundary_timer, (uint64_t)delay_sec * 1000000ULL);
    ESP_LOGI(kTag, "Next panel flip scheduled in %lld s", (long long)delay_sec);
}

int makeplans_minutes_to_next_boundary(int fallback_minutes) {
    if (!s_has_data || s_last.unpaired) return fallback_minutes;
    const int64_t now = (int64_t)time(nullptr);
    if (now < kMinValidEpoch) return fallback_minutes;
    const int64_t boundary = NextBoundaryEpoch(s_last, now);
    if (boundary == 0) return fallback_minutes;
    const int64_t minutes = (boundary - now + 59) / 60;
    return (int)(minutes < 1 ? 1 : minutes);
}

// ============================================================
// HTTP helper (body capture + optional Set-Cookie capture)
// ============================================================

struct HttpCtx {
    char* buf = nullptr;
    size_t cap = 0;
    size_t len = 0;
    bool overflow = false;
    std::vector<std::string>* set_cookies = nullptr;
};

static esp_err_t HttpEvent(esp_http_client_event_t* evt) {
    auto* ctx = static_cast<HttpCtx*>(evt->user_data);
    if (!ctx) return ESP_OK;
    switch (evt->event_id) {
        case HTTP_EVENT_ON_HEADER:
            if (ctx->set_cookies && evt->header_key && evt->header_value &&
                strcasecmp(evt->header_key, "Set-Cookie") == 0) {
                ctx->set_cookies->push_back(evt->header_value);
            }
            break;
        case HTTP_EVENT_ON_DATA:
            if (ctx->buf) {
                if (ctx->len + evt->data_len < ctx->cap) {
                    memcpy(ctx->buf + ctx->len, evt->data, evt->data_len);
                    ctx->len += evt->data_len;
                } else {
                    ctx->overflow = true;
                }
            }
            break;
        default:
            break;
    }
    return ESP_OK;
}

// Perform a request; returns HTTP status, or -1 on transport failure.
// extra_headers: {key, value} pairs. body != nullptr makes it a POST with
// Content-Type application/x-www-form-urlencoded.
static int HttpDo(const std::string& url, HttpCtx* ctx,
                  const std::vector<std::pair<std::string, std::string>>& extra_headers,
                  const char* post_body, bool follow_redirects) {
    esp_http_client_config_t config = {};
    config.url = url.c_str();
    config.method = post_body ? HTTP_METHOD_POST : HTTP_METHOD_GET;
    config.event_handler = HttpEvent;
    config.user_data = ctx;
    config.timeout_ms = 15000;
    config.disable_auto_redirect = !follow_redirects;
    config.max_redirection_count = follow_redirects ? 3 : 0;
    config.user_agent = kUserAgent;
    config.cert_pem = kIsrgRootsPem;

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client) {
        ESP_LOGE(kTag, "Failed to init HTTP client");
        return -1;
    }
    for (const auto& h : extra_headers) {
        esp_http_client_set_header(client, h.first.c_str(), h.second.c_str());
    }
    if (post_body) {
        esp_http_client_set_header(client, "Content-Type",
                                   "application/x-www-form-urlencoded");
        esp_http_client_set_post_field(client, post_body, strlen(post_body));
    }

    esp_err_t err = esp_http_client_perform(client);
    if (err != ESP_OK) {
        ESP_LOGE(kTag, "HTTP request failed: %s (%s)", esp_err_to_name(err), url.c_str());
        esp_http_client_cleanup(client);
        return -1;
    }
    int status = esp_http_client_get_status_code(client);
    esp_http_client_cleanup(client);
    if (ctx && ctx->buf) ctx->buf[ctx->len] = '\0';
    if (ctx && ctx->overflow) {
        ESP_LOGW(kTag, "Response truncated at %u bytes (%s)", (unsigned)ctx->cap, url.c_str());
    }
    return status;
}

// ============================================================
// Small parsing helpers
// ============================================================

// "session_id=abc123; path=/; HttpOnly" -> "session_id=abc123"
static std::string CookiePair(const std::string& set_cookie) {
    size_t semi = set_cookie.find(';');
    std::string pair = set_cookie.substr(0, semi);
    while (!pair.empty() && pair.back() == ' ') pair.pop_back();
    return pair;
}

static std::string UrlEncode(const std::string& in) {
    std::string out;
    char hex[8];
    for (unsigned char c : in) {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            out += (char)c;
        } else {
            snprintf(hex, sizeof(hex), "%%%02X", c);
            out += hex;
        }
    }
    return out;
}

// Extract the Rails CSRF token from the setup page HTML. Handles both the
// hidden form input and the <meta name="csrf-token"> tag.
static std::string ExtractCsrfToken(const char* html) {
    if (!html) return "";
    const char* anchor = strstr(html, "authenticity_token");
    if (anchor) {
        const char* value = strstr(anchor, "value=\"");
        if (value && value - anchor < 300) {
            value += 7;
            const char* end = strchr(value, '"');
            if (end) return std::string(value, end - value);
        }
    }
    const char* meta = strstr(html, "name=\"csrf-token\"");
    if (meta) {
        const char* content = strstr(meta, "content=\"");
        if (content && content - meta < 300) {
            content += 9;
            const char* end = strchr(content, '"');
            if (end) return std::string(content, end - content);
        }
    }
    return "";
}

// Parse "2026-09-29T10:00:00+02:00" (or ...Z) to a UTC epoch; optionally
// reports the embedded offset. Returns 0 on failure.
static int64_t ParseIso8601(const char* s, int* offset_out) {
    int y, mo, d, h, mi, sec;
    if (!s || sscanf(s, "%d-%d-%dT%d:%d:%d", &y, &mo, &d, &h, &mi, &sec) != 6) return 0;
    // Days-from-civil (Howard Hinnant), same as the weather client
    int64_t yy = y;
    yy -= mo <= 2;
    int64_t era = (yy >= 0 ? yy : yy - 399) / 400;
    int64_t yoe = yy - era * 400;
    int64_t doy = (153 * (mo + (mo > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    int64_t doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    int64_t days = era * 146097 + doe - 719468;
    int64_t epoch = days * 86400 + h * 3600 + mi * 60 + sec;

    // Skip past seconds (and fractional seconds) to the offset
    const char* p = s + 19;
    if (*p == '.') { ++p; while (isdigit((unsigned char)*p)) ++p; }
    int off = 0;
    if (*p == '+' || *p == '-') {
        int oh = 0, om = 0;
        sscanf(p + 1, "%d:%d", &oh, &om);
        off = (oh * 3600 + om * 60) * (*p == '-' ? -1 : 1);
    }
    if (offset_out) *offset_out = off;
    return epoch - off;
}

// "Fri 3 Oct" + "14:30" for an epoch shifted by the room's offset.
static void FormatRoomLocal(int64_t epoch_utc, int offset_sec,
                            std::string* hhmm, std::string* date_str) {
    int64_t local = epoch_utc + offset_sec;
    int64_t days = local / 86400;
    int64_t rem = local % 86400;
    if (rem < 0) { rem += 86400; days -= 1; }
    char buf[20];
    if (hhmm) {
        snprintf(buf, sizeof(buf), "%02d:%02d", (int)(rem / 3600), (int)((rem % 3600) / 60));
        *hhmm = buf;
    }
    if (date_str) {
        // civil-from-days (Howard Hinnant)
        int64_t z = days + 719468;
        int64_t era = (z >= 0 ? z : z - 146096) / 146097;
        int64_t doe = z - era * 146097;
        int64_t yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
        int64_t doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
        int64_t mp = (5 * doy + 2) / 153;
        int64_t d = doy - (153 * mp + 2) / 5 + 1;
        int64_t m = mp + (mp < 10 ? 3 : -9);
        static const char* kMonths[] = {"Jan","Feb","Mar","Apr","May","Jun",
                                        "Jul","Aug","Sep","Oct","Nov","Dec"};
        static const char* kWeekdays[] = {"Thu","Fri","Sat","Sun","Mon","Tue","Wed"};
        int wd = (int)(((days % 7) + 7) % 7);  // day 0 (1970-01-01) was a Thursday
        snprintf(buf, sizeof(buf), "%s %d %s", kWeekdays[wd], (int)d, kMonths[(m - 1) % 12]);
        *date_str = buf;
    }
}

// "2026-09-29T10:00:00+02:00" -> "10:00" (room-local by construction)
static std::string HhmmFromIso(const char* s) {
    if (!s || strlen(s) < 16) return "";
    return std::string(s + 11, 5);
}

// ============================================================
// NVS config
// ============================================================

bool makeplans_is_configured() {
    Settings nvs(kNvsNamespace, false);
    return !nvs.GetString(kKeyAccount).empty() &&
           !nvs.GetString(kKeyResource).empty() &&
           !nvs.GetString(kKeyCookie).empty();
}

std::string makeplans_get_account() {
    Settings nvs(kNvsNamespace, false);
    return nvs.GetString(kKeyAccount);
}

std::string makeplans_get_resource() {
    Settings nvs(kNvsNamespace, false);
    return nvs.GetString(kKeyResource);
}

// ============================================================
// NFC: keep the passive tag serving the room's booking URL.
// EEPROM is only rewritten when the target URL changes; the RF side is
// phone-powered, so an already-written tag works through deep sleep.
// Runs on the 16KB fetch/pairing tasks (I2C + small buffers only).
// ============================================================

static std::string NfcTargetUrl() {
    Settings nvs(kNvsNamespace, false);
    std::string url = nvs.GetString(kKeyNfcUrl);
    if (url.empty()) {
        const std::string account = nvs.GetString(kKeyAccount);
        if (account.empty()) return "";
        url = "https://" + account + ".makeplans.com/";
    }
    return url;
}

static void SyncNfcTag() {
    if (!makeplans_is_configured()) return;
    const std::string url = NfcTargetUrl();
    if (url.empty()) return;
    {
        Settings nvs(kNvsNamespace, false);
        if (nvs.GetString(kKeyNfcWritten) == url) return;  // tag already current
    }

    if (nfc_tag_write_uri(url)) {
        Settings nvs(kNvsNamespace, true);
        nvs.SetString(kKeyNfcWritten, url);
        ESP_LOGI(kTag, "NFC tag now serves %s", url.c_str());
    }
}

bool makeplans_nfc_active() {
    Settings nvs(kNvsNamespace, false);
    const std::string written = nvs.GetString(kKeyNfcWritten);
    return !written.empty() && written == NfcTargetUrl();
}

void makeplans_unpair() {
    Settings nvs(kNvsNamespace, true);
    nvs.EraseKey(kKeyCookie);
    ESP_LOGI(kTag, "Unpaired (cookie erased)");
}

// ============================================================
// Schedule fetch
// ============================================================

static const size_t kResponseCapacity = 96 * 1024;
static char* s_response_buf = nullptr;

static bool EnsureResponseBuffer() {
    if (s_response_buf) return true;
    s_response_buf = static_cast<char*>(
        heap_caps_malloc(kResponseCapacity, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
    if (!s_response_buf) s_response_buf = static_cast<char*>(malloc(kResponseCapacity));
    if (!s_response_buf) ESP_LOGE(kTag, "Failed to allocate response buffer");
    return s_response_buf != nullptr;
}

static bool ParseEvents(const char* json, MakePlansSchedule* out) {
    cJSON* root = cJSON_Parse(json);
    if (!cJSON_IsArray(root)) {
        ESP_LOGE(kTag, "Events response is not a JSON array");
        cJSON_Delete(root);
        return false;
    }

    bool have_offset = false;
    cJSON* item = nullptr;
    cJSON_ArrayForEach(item, root) {
        cJSON* booking = cJSON_GetObjectItem(item, "booking");
        cJSON* event = cJSON_GetObjectItem(item, "event");
        cJSON* obj = booking ? booking : event;
        if (!cJSON_IsObject(obj)) continue;

        const char* from_key = booking ? "booked_from" : "starts_at";
        const char* to_key = booking ? "booked_to" : "ends_at";
        cJSON* from = cJSON_GetObjectItem(obj, from_key);
        cJSON* to = cJSON_GetObjectItem(obj, to_key);
        if (!cJSON_IsString(from) || !cJSON_IsString(to)) continue;

        MakePlansEntry entry;
        int offset = 0;
        entry.start_epoch = ParseIso8601(from->valuestring, &offset);
        entry.end_epoch = ParseIso8601(to->valuestring, nullptr);
        if (entry.start_epoch == 0 || entry.end_epoch == 0) continue;
        if (!have_offset) {
            out->utc_offset_sec = offset;
            have_offset = true;
        }
        entry.start_hhmm = HhmmFromIso(from->valuestring);
        entry.end_hhmm = HhmmFromIso(to->valuestring);

        cJSON* title = cJSON_GetObjectItem(obj, "title");
        cJSON* service = cJSON_GetObjectItem(obj, "service");
        cJSON* service_title = service ? cJSON_GetObjectItem(service, "title") : nullptr;
        if (cJSON_IsString(title) && title->valuestring[0]) {
            entry.title = title->valuestring;
        } else if (cJSON_IsString(service_title) && service_title->valuestring[0]) {
            entry.title = service_title->valuestring;
        } else {
            entry.title = booking ? "Booked" : "Event";
        }

        if (booking) {
            cJSON* person = cJSON_GetObjectItem(obj, "person");
            cJSON* name = person ? cJSON_GetObjectItem(person, "name") : nullptr;
            if (cJSON_IsString(name)) entry.person = name->valuestring;
            cJSON* state = cJSON_GetObjectItem(obj, "state");
            entry.tentative = cJSON_IsString(state) &&
                              strcmp(state->valuestring, "confirmed") != 0;
        }

        if (out->room_title.empty()) {
            cJSON* resource = cJSON_GetObjectItem(obj, "resource");
            cJSON* rtitle = resource ? cJSON_GetObjectItem(resource, "title") : nullptr;
            if (cJSON_IsString(rtitle)) out->room_title = rtitle->valuestring;
        }

        out->entries.push_back(std::move(entry));
    }
    cJSON_Delete(root);

    std::sort(out->entries.begin(), out->entries.end(),
              [](const MakePlansEntry& a, const MakePlansEntry& b) {
                  return a.start_epoch < b.start_epoch;
              });
    return true;
}

static void StampFetchTime(MakePlansSchedule* out) {
    out->fetched_epoch = (int64_t)time(nullptr);
    if (out->fetched_epoch >= kMinValidEpoch) {
        FormatRoomLocal(out->fetched_epoch, out->utc_offset_sec,
                        &out->fetched_hhmm, &out->fetched_date);
    }
}

static void DoFetch() {
    if (!s_initialized || !makeplans_is_configured()) return;
    if (!EnsureResponseBuffer()) return;

    std::string account, resource, cookie, cached_room;
    int cached_offset = 0;
    {
        Settings nvs(kNvsNamespace, false);
        account = nvs.GetString(kKeyAccount);
        resource = nvs.GetString(kKeyResource);
        cookie = nvs.GetString(kKeyCookie);
        cached_room = nvs.GetString(kKeyRoom);
        cached_offset = nvs.GetInt(kKeyTzOffset, 0);
    }

    const std::string url = "https://" + account +
        ".makeplans.com/api/v1/bookings/room_display/events?resource_id=" + resource;
    ESP_LOGI(kTag, "Fetching schedule: %s", url.c_str());

    HttpCtx ctx;
    ctx.buf = s_response_buf;
    ctx.cap = kResponseCapacity;
    const int status = HttpDo(url, &ctx,
        {{"Accept", "application/json"},
         {"Cookie", "room_display_token_" + resource + "=" + cookie}},
        nullptr, true);

    if (status == 404) {
        // Cookie revoked or expired. Keep the cookie in NVS (a transient
        // server-side problem shouldn't force a re-pair) but tell the UI.
        ESP_LOGW(kTag, "Schedule fetch returned 404 — pairing invalid?");
        MakePlansSchedule schedule;
        schedule.unpaired = true;
        schedule.room_title = cached_room;
        schedule.utc_offset_sec = cached_offset;
        StampFetchTime(&schedule);
        s_last = schedule;
        s_has_data = true;
        if (s_boundary_timer) esp_timer_stop(s_boundary_timer);
        if (s_callback) s_callback(schedule);
        return;
    }
    if (status != 200) {
        ESP_LOGE(kTag, "Schedule fetch failed: status=%d", status);
        return;  // keep showing the last good schedule
    }

    MakePlansSchedule schedule;
    schedule.utc_offset_sec = cached_offset;
    if (!ParseEvents(s_response_buf, &schedule)) return;
    if (schedule.room_title.empty()) schedule.room_title = cached_room;
    StampFetchTime(&schedule);

    // Cache room title + offset so reboots and empty days render correctly.
    if ((!schedule.room_title.empty() && schedule.room_title != cached_room) ||
        schedule.utc_offset_sec != cached_offset) {
        Settings nvs(kNvsNamespace, true);
        if (!schedule.room_title.empty()) nvs.SetString(kKeyRoom, schedule.room_title);
        nvs.SetInt(kKeyTzOffset, schedule.utc_offset_sec);
    }

    s_last = schedule;
    s_has_data = true;
    ESP_LOGI(kTag, "Schedule: %d entries for '%s' (offset %+d h)",
             (int)schedule.entries.size(), schedule.room_title.c_str(),
             schedule.utc_offset_sec / 3600);
    if (s_callback) s_callback(schedule);
    ArmBoundaryTimer();
    SyncNfcTag();  // no-op unless the target URL changed
}

// ============================================================
// Pairing (runs in a dedicated 16KB-stack worker; TLS needs the room)
// ============================================================

struct PairJob {
    std::string account;
    std::string resource;
    std::string code;
    MakePlansPairResult result = MakePlansPairResult::NetworkError;
    SemaphoreHandle_t done = nullptr;
};

static MakePlansPairResult PairInner(const PairJob& job) {
    const std::string base = "https://" + job.account + ".makeplans.com";
    const std::string setup_url = base + "/room_display/" + job.resource + "/setup";
    const std::string verify_url = base + "/room_display/" + job.resource + "/verify_token";

    const size_t kPairBufCap = 32 * 1024;
    char* buf = static_cast<char*>(
        heap_caps_malloc(kPairBufCap, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
    if (!buf) buf = static_cast<char*>(malloc(kPairBufCap));
    if (!buf) {
        ESP_LOGE(kTag, "Pairing: failed to allocate %u-byte buffer", (unsigned)kPairBufCap);
        return MakePlansPairResult::NetworkError;
    }

    // Step 1: setup page -> CSRF token + session cookie(s)
    std::vector<std::string> setup_cookies;
    HttpCtx ctx;
    ctx.buf = buf;
    ctx.cap = kPairBufCap;
    ctx.set_cookies = &setup_cookies;
    int status = HttpDo(setup_url, &ctx, {{"Accept", "text/html"}}, nullptr, true);
    if (status < 0) { free(buf); return MakePlansPairResult::NetworkError; }
    if (status != 200) {
        ESP_LOGE(kTag, "Pairing setup page: status=%d", status);
        free(buf);
        return MakePlansPairResult::ServerError;
    }
    const std::string csrf = ExtractCsrfToken(buf);
    if (csrf.empty()) {
        ESP_LOGE(kTag, "Pairing: no CSRF token in setup page");
        free(buf);
        return MakePlansPairResult::ServerError;
    }
    std::string session_cookie;
    for (const auto& sc : setup_cookies) {
        if (!session_cookie.empty()) session_cookie += "; ";
        session_cookie += CookiePair(sc);
    }

    // Step 2: POST the 6-digit code. Success = 302 + room_display_token cookie.
    const std::string body =
        "token=" + UrlEncode(job.code) + "&authenticity_token=" + UrlEncode(csrf);
    std::vector<std::string> verify_cookies;
    ctx = HttpCtx{};
    ctx.buf = buf;
    ctx.cap = kPairBufCap;
    ctx.set_cookies = &verify_cookies;
    std::vector<std::pair<std::string, std::string>> headers = {{"Accept", "text/html"}};
    if (!session_cookie.empty()) headers.push_back({"Cookie", session_cookie});
    status = HttpDo(verify_url, &ctx, headers, body.c_str(), false);
    free(buf);
    if (status < 0) return MakePlansPairResult::NetworkError;

    const std::string cookie_name = "room_display_token_" + job.resource + "=";
    std::string token_value;
    for (const auto& sc : verify_cookies) {
        const std::string pair = CookiePair(sc);
        if (pair.compare(0, cookie_name.size(), cookie_name) == 0) {
            token_value = pair.substr(cookie_name.size());
            break;
        }
    }

    if (token_value.empty()) {
        if (status == 200) {
            ESP_LOGW(kTag, "Pairing rejected (form re-rendered): bad/expired code");
            return MakePlansPairResult::BadCode;
        }
        ESP_LOGE(kTag, "Pairing: status=%d but no room_display_token cookie", status);
        return MakePlansPairResult::ServerError;
    }

    {
        Settings nvs(kNvsNamespace, true);
        nvs.SetString(kKeyAccount, job.account);
        nvs.SetString(kKeyResource, job.resource);
        nvs.SetString(kKeyCookie, token_value);
    }
    ESP_LOGI(kTag, "Paired: account=%s resource=%s (cookie %u bytes)",
             job.account.c_str(), job.resource.c_str(), (unsigned)token_value.size());
    return MakePlansPairResult::Ok;
}

static void PairTask(void* arg) {
    auto* job = static_cast<PairJob*>(arg);
    job->result = PairInner(*job);
    xSemaphoreGive(job->done);
    vTaskDelete(nullptr);
}

static void EnsureRegistered() {
    if (s_registered) return;
    data_source_register({"makeplans", 5, []() { DoFetch(); }});
    s_registered = true;
}

MakePlansPairResult makeplans_pair(const std::string& account,
                                   const std::string& resource_id,
                                   const std::string& code) {
    PairJob job;
    job.account = account;
    job.resource = resource_id;
    job.code = code;
    job.done = xSemaphoreCreateBinary();
    if (!job.done) return MakePlansPairResult::NetworkError;

    if (xTaskCreate(PairTask, "mp_pair", 16384, &job, 5, nullptr) != pdPASS) {
        vSemaphoreDelete(job.done);
        ESP_LOGE(kTag, "Failed to create pairing task");
        return MakePlansPairResult::NetworkError;
    }
    xSemaphoreTake(job.done, portMAX_DELAY);
    vSemaphoreDelete(job.done);

    if (job.result == MakePlansPairResult::Ok) {
        EnsureRegistered();
        data_sources_start();  // arms the 5-min timer if this is the first pair
    }
    return job.result;
}

// ============================================================
// Public API
// ============================================================

void makeplans_api_init(MakePlansCallback callback) {
    if (s_initialized) {
        ESP_LOGW(kTag, "Already initialized");
        return;
    }
    s_callback = callback;
    s_initialized = true;
    // Only paired devices join the 5-minute cadence; this keeps the battery
    // sleep cycle of a non-door-sign device unaffected.
    if (makeplans_is_configured()) {
        EnsureRegistered();
    }
}

bool makeplans_api_is_ready() {
    return s_initialized;
}

bool makeplans_api_fetch_now() {
    if (!s_initialized || !s_registered) return false;
    return data_source_fetch_now("makeplans");
}

const MakePlansSchedule* makeplans_api_get_last() {
    return s_has_data ? &s_last : nullptr;
}

/**
 * @file makeplans_http.cc
 * @brief MakePlans door-sign pairing page + /api/makeplans endpoint
 *
 * Served by the device web server in both AP-setup and LAN mode. The pairing
 * page is a pure client of /api/makeplans — the same calls work from curl.
 */

#include "widgets/makeplans/makeplans_http.h"

#include "common/data_source.h"
#include "common/httpd_helpers.h"
#include "widgets/makeplans/makeplans_api.h"
#include "settings.h"

#include <esp_log.h>

#include <cctype>
#include <cstdio>
#include <cstring>
#include <string>

namespace widgets {

namespace {

const char* kTag = "MakePlansHttp";
constexpr int kMaxIntervalMinutes = 240;

// Effective poll interval; read from NVS directly so it is shown (and
// settable) before pairing registers the data source.
int IntervalMinutes() {
    Settings nvs("datasrc", false);
    return nvs.GetInt("makeplans", kMakePlansDefaultIntervalMinutes);
}

// Pairing page for the MakePlans door sign, served at /makeplans. Pure
// client of /api/makeplans — same calls that work from curl.
const char kMakePlansHtml[] = R"HTML(
<!DOCTYPE html><html><head><meta charset="UTF-8"><title>Door Sign Pairing</title>
<meta name="viewport" content="width=device-width,initial-scale=1,maximum-scale=1">
<style>
*{box-sizing:border-box}body{margin:0;background:#ece8dc;color:#171717;font-family:-apple-system,BlinkMacSystemFont,"Segoe UI",sans-serif;font-size:14px}.app{max-width:440px;margin:0 auto;padding:12px}.brand{font-weight:800;font-size:18px;margin-bottom:10px}.panel{background:#fff;border:2px solid #111;border-radius:6px;box-shadow:3px 3px 0 #111;margin-bottom:12px;padding:12px}.status{font-weight:700}.status.ok{color:#0a7d2c}.status.err{color:#c81e1e}label{display:block;font-weight:700;margin:10px 0 4px}input,select{width:100%;border:2px solid #111;border-radius:4px;padding:8px;font-size:15px;background:#fafafa}.hint{color:#555;font-size:12px;margin-top:3px}.row{display:flex;gap:8px;margin-top:14px}.btn{flex:1;border:2px solid #111;background:#ff3b30;color:#fff;border-radius:5px;padding:10px;font-weight:800;font-size:14px;box-shadow:2px 2px 0 #111}.btn.secondary{background:#fff;color:#111;flex:0 0 auto}.btn:disabled{opacity:.45}.note{border:2px solid #111;background:#fffbe6;border-radius:6px;padding:10px;box-shadow:3px 3px 0 #111;line-height:1.5;font-size:13px}
</style></head><body><main class="app">
<div class="brand">Door Sign Pairing</div>
<div class="panel"><div class="status" id="status">Loading...</div></div>
<form class="panel" id="form">
<label for="account">MakePlans account</label>
<input id="account" autocapitalize="off" autocorrect="off" placeholder="youraccount" pattern="[a-z0-9\-]+" required>
<div class="hint">The subdomain: <b>youraccount</b>.makeplans.com</div>
<label for="resource">Resource ID</label>
<input id="resource" inputmode="numeric" pattern="[0-9]+" required>
<label for="code">Pairing code</label>
<input id="code" inputmode="numeric" autocomplete="one-time-code" maxlength="16" required>
<div class="row"><button class="btn" id="pair">Pair</button>
<button type="button" class="btn secondary" id="unpair">Unpair</button></div>
</form>
<div class="panel">
<label for="interval" style="margin-top:0">Refresh every</label>
<select id="interval"><option value="5">5 minutes</option><option value="10">10 minutes</option>
<option value="15">15 minutes</option><option value="30">30 minutes</option>
<option value="60">60 minutes</option></select>
<div class="hint" id="interval_hint">How often the sign fetches new bookings. Shorter drains the battery faster; the sign still flips on time at booking start/end.</div>
</div>
<div class="note"><b>Where is the code?</b> In MakePlans admin open the resource
(room) &rarr; Room display &rarr; generate a pairing code. The 6-digit code is
valid for 10 minutes. After pairing, the sign fetches today's schedule at the
interval above; set it as the home page via <code>/api/pages</code>.</div>
<script>
var S=document.getElementById('status');
function show(t,cls){S.textContent=t;S.className='status'+(cls?' '+cls:'')}
function refresh(){fetch('/api/makeplans').then(function(r){return r.json()}).then(function(j){
if(j.paired){show('Paired: '+j.account+'.makeplans.com, resource '+j.resource,'ok');
document.getElementById('account').value=j.account;
document.getElementById('resource').value=j.resource;}
else show('Not paired','err');
var I=document.getElementById('interval'),v=String(j.interval_min);
if(!I.querySelector('option[value="'+v+'"]')){var o=document.createElement('option');
o.value=v;o.textContent=v+' minutes';I.appendChild(o)}I.value=v;}).catch(function(){show('Device unreachable','err')})}
document.getElementById('form').addEventListener('submit',function(e){e.preventDefault();
show('Pairing... (takes a few seconds)');
document.getElementById('pair').disabled=true;
fetch('/api/makeplans',{method:'POST',body:JSON.stringify({
account:document.getElementById('account').value.trim(),
resource_id:document.getElementById('resource').value.trim(),
code:document.getElementById('code').value.trim()})})
.then(function(r){return r.json()}).then(function(j){
document.getElementById('pair').disabled=false;
if(j.success){show('Paired! Fetching the schedule now.','ok')}
else{var m={bad_code:'Wrong or expired code - generate a new one',
network_error:'Device could not reach makeplans.com',
server_error:'Unexpected reply from makeplans.com',
invalid_input:'Check account / resource / code format'};
show('Failed: '+(m[j.error]||j.error),'err')}})
.catch(function(){document.getElementById('pair').disabled=false;
show('Request failed - is the device still on this network?','err')})});
document.getElementById('interval').addEventListener('change',function(){
var H=document.getElementById('interval_hint');
fetch('/api/makeplans',{method:'POST',body:JSON.stringify({interval_min:parseInt(this.value,10)})})
.then(function(r){return r.json()}).then(function(j){H.textContent=j.success?'Saved.':'Could not save: '+j.error})
.catch(function(){H.textContent='Could not save - device unreachable'})});
document.getElementById('unpair').addEventListener('click',function(){
fetch('/api/makeplans',{method:'POST',body:JSON.stringify({unpair:true})})
.then(function(){refresh()})});
refresh();
</script></main></body></html>
)HTML";

// GET /makeplans — the pairing web page (talks to /api/makeplans below).
esp_err_t MakePlansPageHandler(httpd_req_t* req) {
    httpd_resp_set_type(req, "text/html");
    httpd_resp_set_hdr(req, "Cache-Control", "no-store");
    httpd_resp_set_hdr(req, "Connection", "close");
    esp_err_t ret = httpd_resp_send(req, kMakePlansHtml, strlen(kMakePlansHtml));
    web::CloseCurrentSession(req);
    return ret;
}

// GET/POST /api/makeplans — door-sign pairing + status.
// GET returns {"paired":..,"account":..,"resource":..,"success":true}.
// POST {"account":"x","resource_id":1,"code":"123456"} runs the pairing flow
// against {account}.makeplans.com (blocking, a few seconds; the TLS work runs
// in its own worker task). POST {"unpair":true} forgets the cookie.
// POST {"interval_min":15} sets the poll interval (1-240, applies live). The
// long-lived pairing cookie itself never crosses this API — only the
// short-lived 6-digit code does.
esp_err_t MakePlansConfigHandler(httpd_req_t* req) {
    if (req->method == HTTP_POST) {
        cJSON* root = web::ReadJsonBody(req);
        if (!root) {
            web::SendJson(req, "{\"success\":false,\"error\":\"bad_json\"}");
            return ESP_FAIL;
        }

        cJSON* unpair = cJSON_GetObjectItemCaseSensitive(root, "unpair");
        cJSON* fetch = cJSON_GetObjectItemCaseSensitive(root, "fetch");
        cJSON* nfc_url = cJSON_GetObjectItemCaseSensitive(root, "nfc_url");
        cJSON* interval = cJSON_GetObjectItemCaseSensitive(root, "interval_min");
        if (interval != nullptr) {
            if (!cJSON_IsNumber(interval) || interval->valueint < 1 ||
                interval->valueint > kMaxIntervalMinutes) {
                cJSON_Delete(root);
                web::SendJson(req, "{\"success\":false,\"error\":\"invalid_interval\"}");
                return ESP_OK;
            }
            {
                Settings nvs("datasrc", true);
                nvs.SetInt("makeplans", interval->valueint);
            }
            data_sources_reload();
            ESP_LOGI(kTag, "MakePlans interval set: %d min", interval->valueint);
        }
        if (cJSON_IsString(nfc_url)) {
            // Booking URL the NFC tag should serve ("" = default
            // https://{account}.makeplans.com/). Tag rewrites on next fetch.
            const std::string url = nfc_url->valuestring;
            if (url.size() > 200 ||
                (!url.empty() && url.compare(0, 8, "https://") != 0)) {
                cJSON_Delete(root);
                web::SendJson(req, "{\"success\":false,\"error\":\"invalid_nfc_url\"}");
                return ESP_OK;
            }
            Settings nvs("makeplans", true);
            nvs.SetString("nfc_url", url);
            ESP_LOGI(kTag, "MakePlans NFC URL set: %s", url.empty() ? "(default)" : url.c_str());
        }
        if (cJSON_IsTrue(fetch)) {
            // Remote refresh: same as a long-press on the device.
            const bool started = makeplans_api_fetch_now();
            cJSON_Delete(root);
            web::SendJson(req, started ? "{\"success\":true,\"fetching\":true}"
                                       : "{\"success\":false,\"error\":\"not_ready\"}");
            return ESP_OK;
        }
        if (cJSON_IsTrue(unpair)) {
            makeplans_unpair();
            cJSON_Delete(root);
        } else if (cJSON_GetObjectItemCaseSensitive(root, "code") == nullptr) {
            // Config-only POST (e.g. just nfc_url): no pairing attempt.
            cJSON_Delete(root);
        } else {
            std::string account, resource, code;
            cJSON* account_item = cJSON_GetObjectItemCaseSensitive(root, "account");
            if (cJSON_IsString(account_item)) account = account_item->valuestring;
            cJSON* resource_item = cJSON_GetObjectItemCaseSensitive(root, "resource_id");
            if (cJSON_IsString(resource_item)) {
                resource = resource_item->valuestring;
            } else if (cJSON_IsNumber(resource_item)) {
                resource = std::to_string(resource_item->valueint);
            }
            cJSON* code_item = cJSON_GetObjectItemCaseSensitive(root, "code");
            if (cJSON_IsString(code_item)) code = code_item->valuestring;
            cJSON_Delete(root);

            // account becomes a hostname label, resource a path segment:
            // restrict both to harmless characters.
            auto valid_chars = [](const std::string& s, bool digits_only) {
                if (s.empty() || s.size() > 63) return false;
                for (char c : s) {
                    if (digits_only ? !isdigit((unsigned char)c)
                                    : !(islower((unsigned char)c) ||
                                        isdigit((unsigned char)c) || c == '-')) {
                        return false;
                    }
                }
                return true;
            };
            if (!valid_chars(account, false) || !valid_chars(resource, true) ||
                code.empty() || code.size() > 16) {
                web::SendJson(req, "{\"success\":false,\"error\":\"invalid_input\"}");
                return ESP_OK;
            }

            ESP_LOGI(kTag, "MakePlans pairing requested: account=%s resource=%s",
                     account.c_str(), resource.c_str());
            const MakePlansPairResult result = makeplans_pair(account, resource, code);
            const char* error = nullptr;
            switch (result) {
                case MakePlansPairResult::Ok: break;
                case MakePlansPairResult::BadCode: error = "bad_code"; break;
                case MakePlansPairResult::NetworkError: error = "network_error"; break;
                case MakePlansPairResult::ServerError: error = "server_error"; break;
            }
            if (error) {
                char response[64];
                snprintf(response, sizeof(response),
                         "{\"success\":false,\"error\":\"%s\"}", error);
                web::SendJson(req, response);
                return ESP_OK;
            }
        }
    }

    std::string json = "{\"paired\":";
    json += makeplans_is_configured() ? "true" : "false";
    json += ",\"account\":\"" + makeplans_get_account() + "\"";
    json += ",\"resource\":\"" + makeplans_get_resource() + "\"";
    json += ",\"interval_min\":" + std::to_string(IntervalMinutes());
    if (const MakePlansSchedule* last = makeplans_api_get_last()) {
        auto escape = [](const std::string& s) {
            std::string out;
            for (char c : s) {
                if (c == '"' || c == '\\') out += '\\';
                out += c;
            }
            return out;
        };
        json += ",\"room\":\"" + escape(last->room_title) + "\"";
        json += ",\"entries\":" + std::to_string(last->entries.size());
        json += ",\"last_fetch\":\"" + last->fetched_hhmm + "\"";
        json += ",\"last_fetch_epoch\":" + std::to_string(last->fetched_epoch);
        json += ",\"unpaired_response\":";
        json += last->unpaired ? "true" : "false";
    }
    {
        Settings nvs("makeplans", false);
        const std::string nfc_override = nvs.GetString("nfc_url");
        json += ",\"nfc_url\":\"" + nfc_override + "\"";
        json += ",\"nfc_active\":";
        json += makeplans_nfc_active() ? "true" : "false";
    }
    json += ",\"success\":true}";
    web::SendJson(req, json.c_str());
    return ESP_OK;
}

}  // namespace

bool makeplans_register_http(httpd_handle_t server) {
    httpd_uri_t page_uri = {
        .uri = "/makeplans",
        .method = HTTP_GET,
        .handler = MakePlansPageHandler,
        .user_ctx = nullptr
    };
    if (httpd_register_uri_handler(server, &page_uri) != ESP_OK) return false;

    httpd_uri_t get_uri = {
        .uri = "/api/makeplans",
        .method = HTTP_GET,
        .handler = MakePlansConfigHandler,
        .user_ctx = nullptr
    };
    if (httpd_register_uri_handler(server, &get_uri) != ESP_OK) return false;

    httpd_uri_t post_uri = {
        .uri = "/api/makeplans",
        .method = HTTP_POST,
        .handler = MakePlansConfigHandler,
        .user_ctx = nullptr
    };
    if (httpd_register_uri_handler(server, &post_uri) != ESP_OK) return false;

    return true;
}

}  // namespace widgets

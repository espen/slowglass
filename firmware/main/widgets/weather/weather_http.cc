/**
 * @file weather_http.cc
 * @brief Weather location config page (/weather) + GET/POST /api/weather
 *
 * The page is a pure client of /api/weather — the same calls work from
 * curl:
 *   GET  /api/weather                      -> current override + resolved city
 *   POST /api/weather {"lat":59.91,"lon":10.75,"city":"Oslo",
 *                      "utc_offset_min":120}
 *   POST /api/weather {"clear":true}       -> back to IP geolocation
 *   POST /api/weather {"fetch":true}       -> refetch now
 *   POST /api/weather {"refresh_location":true} -> drop the stored IP
 *        geolocation, re-detect, and refetch (IP geolocation otherwise runs
 *        once and the result is kept forever)
 *
 * The UTC offset drives today/tomorrow day bucketing; the page prefills it
 * from the browser's own timezone, which is almost always what you want.
 */

#include "widgets/weather/weather_http.h"

#include "common/httpd_helpers.h"
#include "widgets/weather/weather_api.h"

#include <esp_log.h>

#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>

namespace widgets {

namespace {

const char* kTag = "WeatherHttp";

const char kWeatherHtml[] = R"HTML(
<!DOCTYPE html><html><head><meta charset="UTF-8"><title>Weather Location</title>
<meta name="viewport" content="width=device-width,initial-scale=1,maximum-scale=1">
<style>
*{box-sizing:border-box}body{margin:0;background:#ece8dc;color:#171717;font-family:-apple-system,BlinkMacSystemFont,"Segoe UI",sans-serif;font-size:14px}.app{max-width:440px;margin:0 auto;padding:12px}.brand{font-weight:800;font-size:18px;margin-bottom:10px}.panel{background:#fff;border:2px solid #111;border-radius:6px;box-shadow:3px 3px 0 #111;margin-bottom:12px;padding:12px}.status{font-weight:700}.status.ok{color:#0a7d2c}.status.err{color:#c81e1e}label{display:block;font-weight:700;margin:10px 0 4px}input{width:100%;border:2px solid #111;border-radius:4px;padding:8px;font-size:15px;background:#fafafa}.hint{color:#555;font-size:12px;margin-top:3px}.row{display:flex;gap:8px;margin-top:14px}.cols{display:flex;gap:8px}.cols>div{flex:1}.btn{flex:1;border:2px solid #111;background:#ff3b30;color:#fff;border-radius:5px;padding:10px;font-weight:800;font-size:14px;box-shadow:2px 2px 0 #111}.btn.secondary{background:#fff;color:#111;flex:0 0 auto}.btn:disabled{opacity:.45}.note{border:2px solid #111;background:#fffbe6;border-radius:6px;padding:10px;box-shadow:3px 3px 0 #111;line-height:1.5;font-size:13px}
</style></head><body><main class="app">
<div class="brand">Weather Location</div>
<div class="panel"><div class="status" id="status">Loading...</div>
<div class="row" id="relocrow" style="display:none">
<button type="button" class="btn secondary" id="reloc">Re-detect location</button></div></div>
<form class="panel" id="form">
<label for="city">City label</label>
<input id="city" placeholder="Oslo" maxlength="48" required>
<div class="hint">Shown on the dashboard header</div>
<div class="cols"><div>
<label for="lat">Latitude</label>
<input id="lat" inputmode="decimal" placeholder="59.9139" required>
</div><div>
<label for="lon">Longitude</label>
<input id="lon" inputmode="decimal" placeholder="10.7522" required>
</div></div>
<label for="tz">UTC offset (minutes)</label>
<input id="tz" inputmode="numeric">
<div class="hint">Prefilled from this browser's timezone; used to decide
when "today" becomes "tomorrow" on the forecast</div>
<div class="row"><button class="btn" id="save">Save</button>
<button type="button" class="btn secondary" id="clear">Use auto (IP)</button></div>
</form>
<div class="note"><b>Finding coordinates:</b> open your place on
openstreetmap.org or Google Maps and copy the two numbers from the URL.
Set once; the override survives reboots and re-flashing. Auto (IP) mode is
unreliable behind a VPN - it locates the VPN exit, not the device.</div>
<script>
var S=document.getElementById('status');
function el(id){return document.getElementById(id)}
function show(t,cls){S.textContent=t;S.className='status'+(cls?' '+cls:'')}
el('tz').value=-new Date().getTimezoneOffset();
function refresh(){fetch('/api/weather').then(function(r){return r.json()}).then(function(j){
el('relocrow').style.display=j.override?'none':'flex';
if(j.override){show('Override: '+(j.city||'(no label)')+' ('+j.lat+', '+j.lon+')','ok');
el('city').value=j.city||'';el('lat').value=j.lat;el('lon').value=j.lon;
if(typeof j.utc_offset_min==='number')el('tz').value=j.utc_offset_min;}
else{show('Auto (IP geolocation): currently '+(j.resolved_city||'unresolved'),'');}
}).catch(function(){show('Device unreachable','err')})}
el('reloc').addEventListener('click',function(){
show('Re-detecting location...');el('reloc').disabled=true;
fetch('/api/weather',{method:'POST',body:JSON.stringify({refresh_location:true})})
.then(function(r){return r.json()}).then(function(j){
el('reloc').disabled=false;
if(j.success){show('Re-detecting location - fetching weather...','ok');setTimeout(refresh,6000)}
else{show('Failed: '+(j.error||'unknown'),'err')}})
.catch(function(){el('reloc').disabled=false;show('Request failed','err')})});
el('form').addEventListener('submit',function(e){e.preventDefault();
var lat=parseFloat(el('lat').value.replace(',','.'));
var lon=parseFloat(el('lon').value.replace(',','.'));
var tz=parseInt(el('tz').value,10);
if(isNaN(lat)||isNaN(lon)){show('Latitude/longitude must be numbers','err');return}
if(isNaN(tz))tz=0;
show('Saving...');el('save').disabled=true;
fetch('/api/weather',{method:'POST',body:JSON.stringify({
lat:lat,lon:lon,city:el('city').value.trim(),utc_offset_min:tz})})
.then(function(r){return r.json()}).then(function(j){
el('save').disabled=false;
if(j.success){show('Saved - fetching '+el('city').value.trim()+' weather now','ok')}
else{show('Failed: '+(j.error||'unknown'),'err')}})
.catch(function(){el('save').disabled=false;show('Request failed','err')})});
el('clear').addEventListener('click',function(){
fetch('/api/weather',{method:'POST',body:JSON.stringify({clear:true})})
.then(function(){el('city').value='';el('lat').value='';el('lon').value='';
el('tz').value=-new Date().getTimezoneOffset();refresh()})});
refresh();
</script></main></body></html>
)HTML";

std::string JsonEscape(const std::string& s) {
    std::string out;
    for (char c : s) {
        if (c == '"' || c == '\\') out += '\\';
        out += c;
    }
    return out;
}

// GET /weather — the location config page (talks to /api/weather below).
esp_err_t WeatherPageHandler(httpd_req_t* req) {
    httpd_resp_set_type(req, "text/html");
    httpd_resp_set_hdr(req, "Cache-Control", "no-store");
    httpd_resp_set_hdr(req, "Connection", "close");
    esp_err_t ret = httpd_resp_send(req, kWeatherHtml, strlen(kWeatherHtml));
    web::CloseCurrentSession(req);
    return ret;
}

// GET/POST /api/weather — location override config.
esp_err_t WeatherConfigHandler(httpd_req_t* req) {
    if (req->method == HTTP_POST) {
        cJSON* root = web::ReadJsonBody(req);
        if (!root) {
            web::SendJson(req, "{\"success\":false,\"error\":\"bad_json\"}");
            return ESP_FAIL;
        }

        cJSON* clear = cJSON_GetObjectItemCaseSensitive(root, "clear");
        cJSON* fetch = cJSON_GetObjectItemCaseSensitive(root, "fetch");
        cJSON* reloc = cJSON_GetObjectItemCaseSensitive(root, "refresh_location");
        if (cJSON_IsTrue(fetch)) {
            const bool started = weather_api_fetch_now();
            cJSON_Delete(root);
            web::SendJson(req, started ? "{\"success\":true,\"fetching\":true}"
                                       : "{\"success\":false,\"error\":\"not_ready\"}");
            return ESP_OK;
        }
        if (cJSON_IsTrue(reloc)) {
            weather_refresh_location();
            cJSON_Delete(root);
            web::SendJson(req, "{\"success\":true,\"refreshing\":true}");
            return ESP_OK;
        }
        if (cJSON_IsTrue(clear)) {
            weather_clear_location_override();
            cJSON_Delete(root);
        } else {
            cJSON* lat = cJSON_GetObjectItemCaseSensitive(root, "lat");
            cJSON* lon = cJSON_GetObjectItemCaseSensitive(root, "lon");
            cJSON* city = cJSON_GetObjectItemCaseSensitive(root, "city");
            cJSON* tz = cJSON_GetObjectItemCaseSensitive(root, "utc_offset_min");
            if (!cJSON_IsNumber(lat) || !cJSON_IsNumber(lon)) {
                cJSON_Delete(root);
                web::SendJson(req, "{\"success\":false,\"error\":\"invalid_input\"}");
                return ESP_OK;
            }
            const double lat_v = lat->valuedouble;
            const double lon_v = lon->valuedouble;
            const std::string city_v =
                cJSON_IsString(city) ? city->valuestring : std::string();
            const int tz_v = cJSON_IsNumber(tz) ? tz->valueint : 0;
            cJSON_Delete(root);

            if (!weather_set_location_override(lat_v, lon_v, city_v, tz_v)) {
                web::SendJson(req, "{\"success\":false,\"error\":\"invalid_input\"}");
                return ESP_OK;
            }
        }
    }

    const WeatherLocationOverride ov = weather_get_location_override();
    char buf[96];
    std::string json = "{\"override\":";
    json += ov.set ? "true" : "false";
    if (ov.set) {
        snprintf(buf, sizeof(buf), ",\"lat\":%.4f,\"lon\":%.4f,\"utc_offset_min\":%d",
                 ov.lat, ov.lon, ov.utc_offset_min);
        json += buf;
        json += ",\"city\":\"" + JsonEscape(ov.city) + "\"";
    }
    json += ",\"resolved_city\":\"";
    json += JsonEscape(weather_api_get_city());
    json += "\",\"success\":true}";
    web::SendJson(req, json.c_str());
    return ESP_OK;
}

}  // namespace

bool weather_register_http(httpd_handle_t server) {
    httpd_uri_t page_uri = {
        .uri = "/weather",
        .method = HTTP_GET,
        .handler = WeatherPageHandler,
        .user_ctx = nullptr
    };
    if (httpd_register_uri_handler(server, &page_uri) != ESP_OK) return false;

    httpd_uri_t get_uri = {
        .uri = "/api/weather",
        .method = HTTP_GET,
        .handler = WeatherConfigHandler,
        .user_ctx = nullptr
    };
    if (httpd_register_uri_handler(server, &get_uri) != ESP_OK) return false;

    httpd_uri_t post_uri = {
        .uri = "/api/weather",
        .method = HTTP_POST,
        .handler = WeatherConfigHandler,
        .user_ctx = nullptr
    };
    if (httpd_register_uri_handler(server, &post_uri) != ESP_OK) return false;

    return true;
}

}  // namespace widgets

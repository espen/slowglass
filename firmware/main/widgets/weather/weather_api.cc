/**
 * @file weather_api.cc
 * @brief MET Norway (api.met.no) weather client implementation
 *
 * Data flow:
 * 1. Geolocate once via ip-api.com (HTTP, no key). Fallback: New York City.
 * 2. GET https://api.met.no/weatherapi/locationforecast/2.0/compact?lat=..&lon=..
 *    with an identifying User-Agent (MET Norway ToS) and the certificate bundle.
 * 3. Walk properties.timeseries: entry 0 = "now"; per-entry local calendar day
 *    (UTC time + geolocated offset) buckets today/tomorrow min/max temperatures.
 *
 * The compact response is ~40 KB, so the body is buffered on the heap
 * (PSRAM-capable) instead of the old 4 KB static buffer.
 */

#include "weather_api.h"

#include "common/data_source.h"

#include <esp_log.h>
#include <esp_http_client.h>
#include <esp_crt_bundle.h>
#include <esp_timer.h>
#include <esp_heap_caps.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <cJSON.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cmath>

static const char* kTag = "WeatherApi";

// MET Norway ToS requires an identifying User-Agent with contact info.
// Replace the URL with your own repo/contact before distributing builds.
static const char* kUserAgent = "slowglass/0.1 (https://github.com/espen/slowglass)";

// Fallback location: New York City
static const double kFallbackLat = 40.7128;
static const double kFallbackLon = -74.0060;
static const char* kFallbackCity = "New York";
static const int kFallbackUtcOffsetSec = -5 * 3600;

// ============================================================
// Static state
// ============================================================

static WeatherCallback s_callback;
static bool s_initialized = false;
static bool s_in_progress = false;
static WeatherData s_last_data;

static bool s_have_location = false;
static double s_lat = kFallbackLat;
static double s_lon = kFallbackLon;
static int s_utc_offset_sec = kFallbackUtcOffsetSec;
static char s_city[64] = {0};

// ============================================================
// Weather icon + text mapping (MET Norway symbol_code)
// ============================================================

WeatherIcon ParseWeatherIcon(const char* text) {
    if (!text || !text[0]) return WeatherIcon::Unknown;

    // symbol_code prefixes and their English condition texts. MET symbol
    // codes carry a _day/_night suffix; only clear/partly-cloudy conditions
    // render differently at night.
    const bool night = strstr(text, "_night") != nullptr;
    if (strstr(text, "clearsky") || strstr(text, "fair") ||
        strstr(text, "Clear") || strstr(text, "Fair"))
        return night ? WeatherIcon::ClearNight : WeatherIcon::Sunny;
    if (strstr(text, "partlycloudy") || strstr(text, "Partly"))
        return night ? WeatherIcon::PartlyCloudyNight : WeatherIcon::PartlyCloudy;
    if (strstr(text, "cloudy") || strstr(text, "Cloudy")) return WeatherIcon::Cloudy;
    if (strstr(text, "snow") || strstr(text, "Snow")) return WeatherIcon::Snow;
    if (strstr(text, "sleet") || strstr(text, "Sleet")) return WeatherIcon::Rain;
    if (strstr(text, "rain") || strstr(text, "drizzle") || strstr(text, "thunder") ||
        strstr(text, "Rain") || strstr(text, "Drizzle") || strstr(text, "Thunder")) return WeatherIcon::Rain;
    if (strstr(text, "fog") || strstr(text, "Fog")) return WeatherIcon::Fog;

    return WeatherIcon::Unknown;
}

// English condition text from a symbol_code like "lightrainshowers_day"
static std::string SymbolToEnglish(const std::string& symbol) {
    // Strip _day / _night / _polartwilight variants
    std::string base = symbol;
    size_t underscore = base.find('_');
    if (underscore != std::string::npos) base = base.substr(0, underscore);

    struct Entry { const char* code; const char* text; };
    static const Entry kTable[] = {
        {"clearsky", "Clear sky"},
        {"fair", "Fair"},
        {"partlycloudy", "Partly cloudy"},
        {"cloudy", "Cloudy"},
        {"fog", "Fog"},
        {"lightrainshowers", "Light rain showers"},
        {"rainshowers", "Rain showers"},
        {"heavyrainshowers", "Heavy rain showers"},
        {"lightrain", "Light rain"},
        {"rain", "Rain"},
        {"heavyrain", "Heavy rain"},
        {"lightsleet", "Light sleet"},
        {"sleet", "Sleet"},
        {"heavysleet", "Heavy sleet"},
        {"lightsnowshowers", "Light snow showers"},
        {"snowshowers", "Snow showers"},
        {"lightsnow", "Light snow"},
        {"snow", "Snow"},
        {"heavysnow", "Heavy snow"},
    };
    for (const auto& e : kTable) {
        if (base == e.code) return e.text;
    }
    if (base.find("thunder") != std::string::npos) return "Thunderstorm";
    if (!base.empty()) {
        base[0] = static_cast<char>(toupper(base[0]));
        return base;
    }
    return "Unknown";
}

// ============================================================
// Feels-like temperature (the API has no apparent-temperature field;
// computed the same way yr.no does — see .agents/plans/weather-feels-like.md)
// ============================================================

static double ComputeFeelsLike(double temp_c, double wind_ms, double rh_pct) {
    const double v_kmh = wind_ms * 3.6;
    // JAG/TI wind chill (Environment Canada / NWS metric form).
    // Valid for T <= 10°C and 10 m wind >= 4.8 km/h — both inputs match
    // what MET provides (2 m temperature, 10 m wind).
    if (temp_c <= 10.0 && v_kmh >= 4.8) {
        const double v16 = pow(v_kmh, 0.16);
        return 13.12 + 0.6215 * temp_c - 11.37 * v16 + 0.3965 * temp_c * v16;
    }
    // NOAA Rothfusz heat-index regression (°F), valid T >= 80°F, RH >= 40%.
    // The NWS low/high-humidity adjustment terms are skipped: those corners
    // (RH < 13% or > 85% while hot) don't occur here and cost < 1°F.
    if (temp_c >= 26.7 && rh_pct >= 40.0) {
        const double tf = temp_c * 9.0 / 5.0 + 32.0;
        const double rh = rh_pct;
        const double hi = -42.379 + 2.04901523 * tf + 10.14333127 * rh
                        - 0.22475541 * tf * rh - 6.83783e-3 * tf * tf
                        - 5.481717e-2 * rh * rh + 1.22874e-3 * tf * tf * rh
                        + 8.5282e-4 * tf * rh * rh - 1.99e-6 * tf * tf * rh * rh;
        return (hi - 32.0) * 5.0 / 9.0;
    }
    // 10–26.7°C or calm air: no model applies (same behaviour as yr/NWS).
    return temp_c;
}

// ============================================================
// Time helpers
// ============================================================

// Parse "2026-09-27T18:00:00Z" to a Unix epoch (UTC). Returns 0 on failure.
static int64_t ParseIso8601Utc(const char* s) {
    int y, mo, d, h, mi, sec;
    if (!s || sscanf(s, "%d-%d-%dT%d:%d:%d", &y, &mo, &d, &h, &mi, &sec) != 6) return 0;
    // Days-from-civil algorithm (Howard Hinnant), valid for our date range
    int64_t yy = y;
    yy -= mo <= 2;
    int64_t era = (yy >= 0 ? yy : yy - 399) / 400;
    int64_t yoe = yy - era * 400;
    int64_t doy = (153 * (mo + (mo > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    int64_t doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    int64_t days = era * 146097 + doe - 719468;
    return days * 86400 + h * 3600 + mi * 60 + sec;
}

static void FormatLocal(int64_t epoch_utc, int offset_sec, char* hhmm, size_t hhmm_len,
                        char* date_str, size_t date_len) {
    int64_t local = epoch_utc + offset_sec;
    int64_t days = local / 86400;
    int64_t rem = local % 86400;
    if (rem < 0) { rem += 86400; days -= 1; }
    if (hhmm) snprintf(hhmm, hhmm_len, "%02d:%02d", (int)(rem / 3600), (int)((rem % 3600) / 60));
    if (date_str) {
        // civil-from-days (Howard Hinnant)
        int64_t z = days + 719468;
        int64_t era = (z >= 0 ? z : z - 146096) / 146097;
        int64_t doe = z - era * 146097;
        int64_t yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
        int64_t yy = yoe + era * 400;
        int64_t doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
        int64_t mp = (5 * doy + 2) / 153;
        int64_t d = doy - (153 * mp + 2) / 5 + 1;
        int64_t m = mp + (mp < 10 ? 3 : -9);
        yy += (m <= 2);
        static const char* kMonths[] = {"Jan","Feb","Mar","Apr","May","Jun",
                                        "Jul","Aug","Sep","Oct","Nov","Dec"};
        static const char* kWeekdays[] = {"Thu","Fri","Sat","Sun","Mon","Tue","Wed"};
        int wd = (int)(((days % 7) + 7) % 7);  // day 0 (1970-01-01) was a Thursday
        snprintf(date_str, date_len, "%s %d %s", kWeekdays[wd], (int)d, kMonths[(m - 1) % 12]);
    }
}

static int64_t LocalDayIndex(int64_t epoch_utc, int offset_sec) {
    int64_t local = epoch_utc + offset_sec;
    int64_t days = local / 86400;
    if (local % 86400 < 0) days -= 1;
    return days;
}

// ============================================================
// HTTP client (heap-buffered; the met.no compact response is ~40 KB)
// ============================================================

static const size_t kResponseCapacity = 96 * 1024;
static char* s_response_buf = nullptr;
static size_t s_response_len = 0;

static esp_err_t HttpEventHandler(esp_http_client_event_t* evt) {
    switch (evt->event_id) {
        case HTTP_EVENT_ON_DATA:
            if (s_response_buf && s_response_len + evt->data_len < kResponseCapacity) {
                memcpy(s_response_buf + s_response_len, evt->data, evt->data_len);
                s_response_len += evt->data_len;
            }
            break;
        default:
            break;
    }
    return ESP_OK;
}

static bool HttpGet(const char* url, bool https) {
    if (!s_response_buf) {
        s_response_buf = static_cast<char*>(
            heap_caps_malloc(kResponseCapacity, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
        if (!s_response_buf) {
            s_response_buf = static_cast<char*>(malloc(kResponseCapacity));
        }
        if (!s_response_buf) {
            ESP_LOGE(kTag, "Failed to allocate response buffer");
            return false;
        }
    }
    s_response_len = 0;

    esp_http_client_config_t config = {};
    config.url = url;
    config.method = HTTP_METHOD_GET;
    config.event_handler = HttpEventHandler;
    config.timeout_ms = 15000;
    config.disable_auto_redirect = false;
    config.user_agent = kUserAgent;
    if (https) {
        config.crt_bundle_attach = esp_crt_bundle_attach;
    }

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client) {
        ESP_LOGE(kTag, "Failed to init HTTP client");
        return false;
    }
    esp_http_client_set_header(client, "Accept", "application/json");

    esp_err_t err = esp_http_client_perform(client);
    if (err != ESP_OK) {
        ESP_LOGE(kTag, "HTTP request failed: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        return false;
    }

    int status = esp_http_client_get_status_code(client);
    if (status != 200 && status != 203) {
        ESP_LOGE(kTag, "HTTP status: %d for %s", status, url);
        esp_http_client_cleanup(client);
        return false;
    }

    s_response_buf[s_response_len] = '\0';
    esp_http_client_cleanup(client);
    return true;
}

// ============================================================
// Geolocation (ip-api.com; free endpoint is HTTP-only)
// ============================================================

static void Geolocate() {
    if (s_have_location) return;

    // Defaults in case anything below fails
    s_lat = kFallbackLat;
    s_lon = kFallbackLon;
    s_utc_offset_sec = kFallbackUtcOffsetSec;
    strncpy(s_city, kFallbackCity, sizeof(s_city) - 1);

    if (!HttpGet("http://ip-api.com/json/?fields=status,city,lat,lon,offset", false)) {
        ESP_LOGW(kTag, "IP geolocation request failed; using fallback %s", s_city);
        s_have_location = true;  // don't retry every fetch
        return;
    }

    cJSON* root = cJSON_Parse(s_response_buf);
    if (!root) {
        ESP_LOGW(kTag, "IP geolocation parse failed; using fallback %s", s_city);
        s_have_location = true;
        return;
    }

    cJSON* status = cJSON_GetObjectItem(root, "status");
    cJSON* lat = cJSON_GetObjectItem(root, "lat");
    cJSON* lon = cJSON_GetObjectItem(root, "lon");
    cJSON* offset = cJSON_GetObjectItem(root, "offset");
    cJSON* city = cJSON_GetObjectItem(root, "city");

    if (cJSON_IsString(status) && strcmp(status->valuestring, "success") == 0 &&
        cJSON_IsNumber(lat) && cJSON_IsNumber(lon)) {
        s_lat = lat->valuedouble;
        s_lon = lon->valuedouble;
        if (cJSON_IsNumber(offset)) s_utc_offset_sec = offset->valueint;
        if (cJSON_IsString(city) && city->valuestring[0]) {
            strncpy(s_city, city->valuestring, sizeof(s_city) - 1);
            s_city[sizeof(s_city) - 1] = '\0';
        }
        ESP_LOGI(kTag, "Geolocated: %s (%.4f, %.4f) UTC%+d", s_city, s_lat, s_lon,
                 s_utc_offset_sec / 3600);
    } else {
        ESP_LOGW(kTag, "IP geolocation unsuccessful; using fallback %s", s_city);
    }

    cJSON_Delete(root);
    s_have_location = true;
}

// ============================================================
// MET Norway locationforecast parsing
// ============================================================

static bool ParseForecast(const char* json, WeatherData* out) {
    cJSON* root = cJSON_Parse(json);
    if (!root) {
        ESP_LOGE(kTag, "Failed to parse forecast JSON");
        return false;
    }

    cJSON* props = cJSON_GetObjectItem(root, "properties");
    cJSON* series = props ? cJSON_GetObjectItem(props, "timeseries") : nullptr;
    if (!cJSON_IsArray(series) || cJSON_GetArraySize(series) == 0) {
        ESP_LOGE(kTag, "No timeseries in response");
        cJSON_Delete(root);
        return false;
    }

    auto instant_detail = [](cJSON* entry, const char* key, double* value) -> bool {
        cJSON* data = cJSON_GetObjectItem(entry, "data");
        cJSON* instant = data ? cJSON_GetObjectItem(data, "instant") : nullptr;
        cJSON* details = instant ? cJSON_GetObjectItem(instant, "details") : nullptr;
        cJSON* item = details ? cJSON_GetObjectItem(details, key) : nullptr;
        if (cJSON_IsNumber(item)) { *value = item->valuedouble; return true; }
        return false;
    };

    auto symbol_code = [](cJSON* entry) -> const char* {
        cJSON* data = cJSON_GetObjectItem(entry, "data");
        if (!data) return nullptr;
        for (const char* period : {"next_1_hours", "next_6_hours", "next_12_hours"}) {
            cJSON* block = cJSON_GetObjectItem(data, period);
            cJSON* summary = block ? cJSON_GetObjectItem(block, "summary") : nullptr;
            cJSON* code = summary ? cJSON_GetObjectItem(summary, "symbol_code") : nullptr;
            if (cJSON_IsString(code) && code->valuestring[0]) return code->valuestring;
        }
        return nullptr;
    };

    // --- Current conditions from the first entry ---
    cJSON* now_entry = cJSON_GetArrayItem(series, 0);
    cJSON* now_time = cJSON_GetObjectItem(now_entry, "time");
    int64_t now_epoch = ParseIso8601Utc(cJSON_IsString(now_time) ? now_time->valuestring : nullptr);
    int64_t today = LocalDayIndex(now_epoch, s_utc_offset_sec);

    double temp_now = 0, humidity = 0, wind_speed = 0, wind_dir_deg = -1;
    if (!instant_detail(now_entry, "air_temperature", &temp_now)) {
        ESP_LOGE(kTag, "No current temperature in response");
        cJSON_Delete(root);
        return false;
    }
    instant_detail(now_entry, "relative_humidity", &humidity);
    instant_detail(now_entry, "wind_speed", &wind_speed);
    instant_detail(now_entry, "wind_from_direction", &wind_dir_deg);

    out->city = s_city;
    out->temp_int = (int32_t)lround(temp_now);
    char buf[32];
    snprintf(buf, sizeof(buf), "%d", (int)out->temp_int);
    out->temp = buf;
    snprintf(buf, sizeof(buf), "%d", (int)lround(humidity));
    out->humidity = buf;
    snprintf(buf, sizeof(buf), "%.1f", wind_speed);
    out->wind_scale = buf;
    if (wind_dir_deg >= 0) {
        static const char* kDirs[] = {"N","NE","E","SE","S","SW","W","NW"};
        out->wind_dir = kDirs[((int)lround(wind_dir_deg / 45.0)) % 8];
    }

    const char* now_symbol = symbol_code(now_entry);
    out->weather_icon = now_symbol ? now_symbol : "";
    out->weather_text = SymbolToEnglish(out->weather_icon);

    // Feels-like: shown only when it rounds >= 2° away from the actual temp,
    // so the renderer can simply check for an empty string.
    const int32_t feels = (int32_t)lround(ComputeFeelsLike(temp_now, wind_speed, humidity));
    if (std::abs(feels - out->temp_int) >= 2) {
        snprintf(buf, sizeof(buf), "%d", (int)feels);
        out->feels_like = buf;
    } else {
        out->feels_like.clear();
    }

    char hhmm[8], date_str[20];
    FormatLocal(now_epoch, s_utc_offset_sec, hhmm, sizeof(hhmm), date_str, sizeof(date_str));
    out->update_time = hhmm;
    out->date_string = date_str;

    // --- Today / tomorrow min-max from the whole series ---
    struct DayAgg {
        double min = 1000, max = -1000;
        std::string midday_symbol;
        int64_t best_midday_dist = 1 << 30;
        bool any = false;
    } days[2];

    out->hourly.clear();

    cJSON* entry = nullptr;
    cJSON_ArrayForEach(entry, series) {
        cJSON* t = cJSON_GetObjectItem(entry, "time");
        int64_t epoch = ParseIso8601Utc(cJSON_IsString(t) ? t->valuestring : nullptr);
        if (epoch == 0) continue;

        // --- Hourly timeline: entries after "now", next 12 h ---
        // (MET keeps 1 h resolution for the first ~2 days, so these arrive
        // as consecutive hours: hourly[0] = +1 h, hourly[1] = +2 h, ...)
        if (epoch > now_epoch && epoch <= now_epoch + 12 * 3600) {
            double h_temp;
            if (instant_detail(entry, "air_temperature", &h_temp)) {
                WeatherHourly h;
                int64_t h_local = ((epoch + s_utc_offset_sec) % 86400 + 86400) % 86400;
                h.hour_local = (int)(h_local / 3600);
                h.temp = (int32_t)lround(h_temp);
                const char* h_sym = symbol_code(entry);
                h.icon_code = h_sym ? h_sym : "";
                cJSON* h_data = cJSON_GetObjectItem(entry, "data");
                cJSON* h_n1 = h_data ? cJSON_GetObjectItem(h_data, "next_1_hours") : nullptr;
                cJSON* h_det = h_n1 ? cJSON_GetObjectItem(h_n1, "details") : nullptr;
                cJSON* h_pr = h_det ? cJSON_GetObjectItem(h_det, "precipitation_amount") : nullptr;
                if (cJSON_IsNumber(h_pr)) h.precip_mm = (float)h_pr->valuedouble;
                out->hourly.push_back(h);
            }
        }

        int64_t day = LocalDayIndex(epoch, s_utc_offset_sec);
        if (day != today && day != today + 1) continue;
        DayAgg& agg = days[day - today];

        double temp;
        if (instant_detail(entry, "air_temperature", &temp)) {
            if (temp < agg.min) agg.min = temp;
            if (temp > agg.max) agg.max = temp;
            agg.any = true;
        }
        // Prefer the symbol closest to 12:00 local as the day's representative
        int64_t local_sec = ((epoch + s_utc_offset_sec) % 86400 + 86400) % 86400;
        int64_t dist = std::abs((long long)(local_sec - 12 * 3600));
        const char* sym = symbol_code(entry);
        if (sym && dist < agg.best_midday_dist) {
            agg.best_midday_dist = dist;
            agg.midday_symbol = sym;
        }
    }

    out->forecast.clear();
    const char* labels[2] = {"Today", "Tomorrow"};
    for (int i = 0; i < 2; i++) {
        if (!days[i].any) continue;
        WeatherForecastDay item;
        item.label = labels[i];
        item.icon_code = days[i].midday_symbol;
        item.weather_text = SymbolToEnglish(days[i].midday_symbol);
        item.temp_min = (int32_t)lround(days[i].min);
        item.temp_max = (int32_t)lround(days[i].max);
        out->forecast.push_back(item);
    }

    cJSON_Delete(root);
    return true;
}

// ============================================================
// Fetch orchestration
// ============================================================

static void DoFetch(void* arg) {
    (void)arg;
    if (!s_initialized || s_in_progress) return;
    s_in_progress = true;

    Geolocate();

    char url[160];
    snprintf(url, sizeof(url),
             "https://api.met.no/weatherapi/locationforecast/2.0/compact?lat=%.4f&lon=%.4f",
             s_lat, s_lon);
    ESP_LOGI(kTag, "Fetching forecast: %s", url);

    WeatherData data;
    if (!HttpGet(url, true) || !ParseForecast(s_response_buf, &data)) {
        ESP_LOGE(kTag, "Failed to fetch or parse forecast");
        s_in_progress = false;
        return;
    }

    s_last_data = data;
    ESP_LOGI(kTag, "Weather: %s %s°C (feels %s) in %s, forecast days=%d, hourly=%d",
             data.weather_text.c_str(), data.temp.c_str(),
             data.feels_like.empty() ? "-" : data.feels_like.c_str(),
             data.city.c_str(), (int)data.forecast.size(), (int)data.hourly.size());

    if (s_callback) {
        s_callback(data);
    }
    s_in_progress = false;
}

// ============================================================
// Public API
// ============================================================

void weather_api_init(WeatherCallback callback) {
    if (s_initialized) {
        ESP_LOGW(kTag, "Already initialized");
        return;
    }
    s_callback = callback;
    s_initialized = true;

    // Scheduling (task spawning, periodic timer, NVS interval override under
    // "datasrc"/"weather") is owned by the data-source registry.
    data_source_register({"weather", 60, []() { DoFetch(nullptr); }});
}

bool weather_api_fetch_now() {
    if (!s_initialized) return false;
    return data_source_fetch_now("weather");
}

bool weather_api_is_ready() {
    return s_initialized;
}

const WeatherData* weather_api_get_last_data() {
    return &s_last_data;
}

const char* weather_api_get_city() {
    return s_city;
}

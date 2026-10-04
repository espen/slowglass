/**
 * @file weather_api.h
 * @brief MET Norway (api.met.no) weather client for ESP32
 *
 * Fetches forecast data from the free MET Norway Locationforecast 2.0 API
 * (the service behind yr.no). No API key required; requests carry an
 * identifying User-Agent per the MET Norway terms of service.
 *
 * Location is resolved automatically from the device's public IP address
 * (ip-api.com). If geolocation fails, falls back to New York City.
 *
 * API: https://api.met.no/weatherapi/locationforecast/2.0/documentation
 *
 * Usage:
 * 1. weather_api_init(callback) once the network is up
 * 2. Callback receives WeatherData on success
 * 3. Timer triggers hourly auto-refresh
 */

#ifndef WEATHER_API_H
#define WEATHER_API_H

#include <stdint.h>
#include <stdbool.h>
#include <string>
#include <functional>
#include <vector>

// ============================================================
// Weather data model
// ============================================================

/**
 * @brief One forecast day derived from the MET Norway timeseries
 */
struct WeatherForecastDay {
    std::string label;        // "Today" / "Tomorrow"
    std::string weather_text; // "Clear sky" / "Rain" / ...
    std::string icon_code;    // MET Norway symbol_code (e.g. "rain", "clearsky_day")
    int32_t temp_min = 0;
    int32_t temp_max = 0;
};

/**
 * @brief One hourly forecast entry (timeseries entries after "now")
 */
struct WeatherHourly {
    int hour_local = 0;       // Local hour of day, 0-23
    int32_t temp = 0;         // Rounded air temperature
    std::string icon_code;    // MET Norway symbol_code
    float precip_mm = 0.0f;   // Precipitation for the hour (next_1_hours)
};

struct WeatherData {
    std::string city;         // Resolved city name (e.g. "Oslo", "New York")
    std::string date_string;  // Local date (e.g. "Sun 27 Sep")
    std::string temp;         // Current temperature (e.g. "14")
    std::string feels_like;   // Perceived temp (wind chill / heat index), rounded.
                              // Empty when it differs from temp by < 2° — the
                              // renderer shows the line only when non-empty.
    std::string weather_icon; // symbol_code for current weather (e.g. "clearsky_day")
    std::string weather_text; // Condition in English (e.g. "Clear sky")
    std::string wind_dir;     // Wind direction (e.g. "SW")
    std::string wind_scale;   // Wind speed in m/s (e.g. "3.4")
    std::string humidity;     // Relative humidity percentage (e.g. "45")
    std::string update_time;  // Local HH:MM of the data point (e.g. "14:30")
    std::string air_quality;  // Unused with MET Norway; kept for compatibility
    int32_t air_aqi = -1;
    int32_t temp_int = 0;     // Numeric temperature for icon/color selection
    std::vector<WeatherForecastDay> forecast; // [0]=Today, [1]=Tomorrow
    std::vector<WeatherHourly> hourly;        // Next ~12 h; [0] = +1 h, hourly steps
};

/**
 * @brief Weather icon classes for rendering
 */
enum class WeatherIcon {
    Sunny,       // clearsky / fair (day)
    ClearNight,  // clearsky / fair (night)
    PartlyCloudy,// partlycloudy (day)
    PartlyCloudyNight, // partlycloudy (night)
    Cloudy,      // cloudy (kept name for compatibility; used as "cloudy")
    Overcast,    // legacy alias, treated like Cloudy
    Rain,        // rain / drizzle / sleet / showers / thunder
    Snow,        // snow
    Fog,         // fog
    Unknown,     // Fallback
};

/**
 * @brief Map a MET Norway symbol_code (or English condition text) to an icon
 */
WeatherIcon ParseWeatherIcon(const char* symbol_or_text);

// ============================================================
// API interface
// ============================================================

/**
 * @brief Callback type for weather data delivery
 */
using WeatherCallback = std::function<void(const WeatherData&)>;

/**
 * @brief Initialize the weather client
 *
 * Resolves location from the public IP (fallback: New York City),
 * fetches immediately, then auto-refreshes hourly via esp_timer.
 * Call after the network is connected.
 */
void weather_api_init(WeatherCallback callback);

/**
 * @brief Trigger a manual weather fetch
 * @return true if request started, false if already in progress
 */
bool weather_api_fetch_now();

/**
 * @brief Check if the client is initialized
 */
bool weather_api_is_ready();

/**
 * @brief Get the last fetched weather data
 */
const WeatherData* weather_api_get_last_data();

/**
 * @brief Resolved city name ("" until geolocation has run)
 */
const char* weather_api_get_city();

// ============================================================
// Location override (NVS namespace "weather")
// ============================================================
//
// When set, the override wins over IP geolocation — the only reliable
// option behind a VPN or misregistered egress IP. Configured from the
// /weather page or POST /api/weather.

struct WeatherLocationOverride {
    bool set = false;
    double lat = 0.0;
    double lon = 0.0;
    std::string city;        // display label on the dashboard
    int utc_offset_min = 0;  // local-time offset for day bucketing
};

WeatherLocationOverride weather_get_location_override();

/** Persist an override and re-resolve + refetch immediately.
 *  Returns false on out-of-range values. */
bool weather_set_location_override(double lat, double lon,
                                   const std::string& city,
                                   int utc_offset_min);

/** Remove the override; next fetch falls back to IP geolocation. */
void weather_clear_location_override();

#endif  // WEATHER_API_H

/**
 * @file data_source.h
 * @brief Registry for periodic network data sources
 *
 * A data source is anything that fetches remote data on a schedule and feeds a
 * page: weather, transit departures, a booking API. This registry generalizes
 * the pattern the weather client established:
 *
 *  - each fetch runs in its own short-lived task (TLS + JSON parsing need far
 *    more stack than esp_timer / WiFi-event tasks provide)
 *  - one periodic esp_timer per source
 *  - refresh interval is per source, overridable in NVS (namespace "datasrc",
 *    key = source name, value = minutes; 0 disables the source)
 *
 * Adding an app = write a fetch function + renderer, then:
 *   data_source_register({"transit", 10, TransitFetch});
 * and call data_sources_start() once the network is up (idempotent).
 */

#ifndef DATA_SOURCE_H
#define DATA_SOURCE_H

#include <functional>

struct DataSourceDef {
    const char* name;              // stable id; also the NVS override key
    int default_interval_minutes;  // refresh cadence when NVS has no override
    std::function<void()> fetch;   // runs in a dedicated 16KB-stack task
};

/** Register a source. Call before data_sources_start(); duplicate names ignored. */
void data_source_register(const DataSourceDef& def);

/** Start timers + initial fetch for all registered sources. Idempotent;
 *  call when the network becomes available. */
void data_sources_start();

/** Trigger one source immediately (no-op if unknown, disabled, or mid-fetch).
 *  Returns true if a fetch was started. */
bool data_source_fetch_now(const char* name);

/** Effective interval for one source (NVS override applied); 0 = disabled. */
int data_source_interval_minutes(const char* name);

/** Smallest enabled interval across sources — the cadence the battery sleep
 *  cycle should honor. Returns fallback_minutes if nothing is enabled. */
int data_sources_min_interval_minutes(int fallback_minutes);

/** Enumeration for config UIs. */
size_t data_sources_count();
const char* data_source_name(size_t index);  // nullptr if out of range

#endif  // DATA_SOURCE_H

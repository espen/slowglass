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
 *  - one periodic esp_timer per source, running only for the active source
 *    (the one behind the page on screen)
 *  - refresh interval is per source, overridable in NVS (namespace "datasrc",
 *    key = source name, value = minutes; 0 disables the source)
 *
 * Adding an app = write a fetch function + renderer, then:
 *   data_source_register({"transit", 10, TransitFetch});
 * call data_sources_set_active() on page changes, and data_sources_start()
 * once the network is up (idempotent).
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

/** Network is up: arm the active source's timer and fetch it if its data is
 *  stale. Idempotent; call when the network becomes available. */
void data_sources_start();

/** Only the source behind the on-screen page polls. Set it on every page
 *  change (nullptr/unknown = none); others' timers stop, and the new one
 *  fetches if older than its interval once the network is up. */
void data_sources_set_active(const char* name);

/** Re-apply NVS intervals after a change (re-arms the active timer with the
 *  new period). No fetch or timer before the network is up. */
void data_sources_reload();

/** Trigger one source immediately (no-op if unknown, disabled, or mid-fetch).
 *  Returns true if a fetch was started. */
bool data_source_fetch_now(const char* name);

/** True while any source has a fetch task running. */
bool data_sources_busy();

/** Effective interval for one source (NVS override applied); 0 = disabled. */
int data_source_interval_minutes(const char* name);

/** Enumeration for config UIs. */
size_t data_sources_count();
const char* data_source_name(size_t index);  // nullptr if out of range

#endif  // DATA_SOURCE_H

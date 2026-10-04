/**
 * @file makeplans_api.h
 * @brief MakePlans room display client (door sign data source)
 *
 * Talks to the official MakePlans room display API
 * (https://developer.makeplans.com/guide/room-display/):
 *
 *  - Pairing: a short-lived 6-digit code from MakePlans admin is exchanged
 *    for a signed cookie valid 365 days. The code is entered via the LAN
 *    config endpoint (/api/makeplans); the cookie never leaves the device.
 *  - Data: GET /api/v1/bookings/room_display/events?resource_id=N returns
 *    today's bookings + events for one resource. Recommended poll: 5 min.
 *
 * Config lives in NVS namespace "makeplans": account (subdomain), resource
 * (id), cookie (opaque signed value), room (cached resource title), tzoff
 * (last seen UTC offset of the room, seconds).
 *
 * The data source only registers once paired, so an unconfigured device
 * never wakes on the 5-minute door-sign cadence.
 */

#ifndef MAKEPLANS_API_H
#define MAKEPLANS_API_H

#include <stdint.h>
#include <string>
#include <functional>
#include <vector>

// One schedule entry, normalized from a MakePlans booking or event.
struct MakePlansEntry {
    int64_t start_epoch = 0;   // UTC
    int64_t end_epoch = 0;     // UTC
    std::string start_hhmm;    // room-local, straight from the feed string
    std::string end_hhmm;
    std::string title;         // booking service title / event title
    std::string person;        // booking person name ("" for events)
    bool tentative = false;    // booking state != confirmed
};

struct MakePlansSchedule {
    std::string room_title;            // resource.title from the feed
    std::vector<MakePlansEntry> entries;  // today's entries, sorted by start
    int utc_offset_sec = 0;            // room tz offset from feed timestamps
    int64_t fetched_epoch = 0;         // UTC time of the fetch
    std::string fetched_hhmm;          // room-local HH:MM of the fetch
    std::string fetched_date;          // room-local "Fri 3 Oct"
    bool unpaired = false;             // server returned 404: cookie invalid
};

using MakePlansCallback = std::function<void(const MakePlansSchedule&)>;

enum class MakePlansPairResult {
    Ok,
    BadCode,        // verify_token re-rendered the form (wrong/expired code)
    NetworkError,   // request failed / no response
    ServerError,    // unexpected status or missing token/cookie in response
};

/**
 * @brief Install the schedule callback; registers the "makeplans" data
 * source (5-minute default) if the device is already paired. Call once the
 * network is up, before data_sources_start().
 */
void makeplans_api_init(MakePlansCallback callback);
bool makeplans_api_is_ready();

/** Trigger an immediate fetch (no-op when unpaired or mid-fetch). */
bool makeplans_api_fetch_now();

/** account + resource + cookie all present in NVS. */
bool makeplans_is_configured();
std::string makeplans_get_account();
std::string makeplans_get_resource();

/** Last fetched schedule (nullptr until the first successful fetch). */
const MakePlansSchedule* makeplans_api_get_last();

/**
 * @brief Minutes until the next occupancy boundary, or fallback_minutes.
 *
 * A boundary is a booking start/end or its 10-minute warning point — the
 * moments the panel must redraw to stay truthful. Used to shorten the
 * battery deep-sleep so the sign flips on time, not up to a poll late.
 * Returns fallback_minutes when unpaired, clock unsynced, or nothing ahead.
 */
int makeplans_minutes_to_next_boundary(int fallback_minutes);

/**
 * @brief True once the device's passive NFC tag serves the booking URL.
 *
 * The tag is (re)written after pairing / fetch whenever the target URL
 * changes: NVS makeplans/nfc_url if set, else
 * https://{account}.makeplans.com/. Tap works with zero battery cost —
 * the phone's field powers the tag, even in deep sleep.
 */
bool makeplans_nfc_active();

/**
 * @brief Run the pairing flow against {account}.makeplans.com.
 *
 * Blocking, but safe to call from a small-stack task (the TLS work runs in
 * a dedicated 16KB-stack worker internally). On success persists NVS config,
 * registers the data source and triggers the first fetch.
 */
MakePlansPairResult makeplans_pair(const std::string& account,
                                   const std::string& resource_id,
                                   const std::string& code);

/** Forget cookie + config (NVS). The data source stays registered but
 *  fetches become no-ops until re-paired. */
void makeplans_unpair();

#endif  // MAKEPLANS_API_H

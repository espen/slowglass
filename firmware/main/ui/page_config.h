/**
 * @file page_config.h
 * @brief NVS-backed page configuration: which page boots, which are enabled
 *
 * Turns "which app is this device" into runtime configuration instead of a
 * build. Stored in NVS namespace "pages":
 *   "home"    — page name string (default "weather")
 *   "enabled" — comma-separated page names for the quick-switch menu
 *               (default "weather,gallery,doorsign,settings"); the home page is
 *               always treated as enabled.
 *
 * Recognized names: the core pages (gallery, settings, chat, log) plus every
 * page a compiled-in widget registers with a config name (weather, doorsign,
 * news, calendar, almanac, lifebar, yearprogress, ebook, ...).
 */

#ifndef UI_PAGE_CONFIG_H
#define UI_PAGE_CONFIG_H

#include <string>
#include <vector>

#include "ui/page_id.h"

namespace ui {
namespace pageconfig {

/** Page the device boots into (and returns to from transient modes). */
RawDrawPageId HomePage();

/** Pages offered in the quick-switch menu, home first, duplicates removed. */
std::vector<RawDrawPageId> EnabledPages();

/** Name for a page id ("weather"), or nullptr if it has no config name. */
const char* PageName(RawDrawPageId page);

/** Parse a page name; returns false if unknown. */
bool PageFromName(const std::string& name, RawDrawPageId* out);

/** Persist a new home page / enabled list (names validated; unknown names
 *  are dropped, an empty result is rejected). Returns false on bad input. */
bool SetHomePage(const std::string& name);
bool SetEnabledPages(const std::vector<std::string>& names);

/** Current config serialized for the LAN API: {"home":..,"enabled":[..]} */
std::string ToJson();

}  // namespace pageconfig
}  // namespace ui

#endif  // UI_PAGE_CONFIG_H

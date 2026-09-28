/**
 * @file page_config.cc
 * @brief NVS-backed page configuration
 */

#include "page_config.h"

#include <esp_log.h>

#include "settings.h"

namespace ui {
namespace pageconfig {

namespace {

const char* kTag = "PageConfig";
const char* kNamespace = "pages";
const char* kHomeKey = "home";
const char* kEnabledKey = "enabled";
const char* kDefaultHome = "weather";
const char* kDefaultEnabled = "weather,settings";

struct NamedPage {
    const char* name;
    RawDrawPageId id;
};

constexpr NamedPage kNamedPages[] = {
    {"weather", RawDrawPageId::Weather},
    {"gallery", RawDrawPageId::Gallery},
    {"settings", RawDrawPageId::Settings},
    {"chat", RawDrawPageId::Chat},
    {"ebook", RawDrawPageId::Ebook},
    {"news", RawDrawPageId::News},
    {"calendar", RawDrawPageId::Calendar},
    {"almanac", RawDrawPageId::Almanac},
    {"lifebar", RawDrawPageId::LifeBar},
    {"yearprogress", RawDrawPageId::YearProgress},
    {"log", RawDrawPageId::Log},
};

std::vector<std::string> SplitCsv(const std::string& csv) {
    std::vector<std::string> out;
    size_t start = 0;
    while (start <= csv.size()) {
        size_t comma = csv.find(',', start);
        if (comma == std::string::npos) comma = csv.size();
        std::string item = csv.substr(start, comma - start);
        // trim spaces
        while (!item.empty() && item.front() == ' ') item.erase(item.begin());
        while (!item.empty() && item.back() == ' ') item.pop_back();
        if (!item.empty()) out.push_back(item);
        start = comma + 1;
    }
    return out;
}

}  // namespace

const char* PageName(RawDrawPageId page) {
    for (const auto& entry : kNamedPages) {
        if (entry.id == page) return entry.name;
    }
    return nullptr;
}

bool PageFromName(const std::string& name, RawDrawPageId* out) {
    for (const auto& entry : kNamedPages) {
        if (name == entry.name) {
            if (out) *out = entry.id;
            return true;
        }
    }
    return false;
}

RawDrawPageId HomePage() {
    Settings nvs(kNamespace, false);
    const std::string name = nvs.GetString(kHomeKey, kDefaultHome);
    RawDrawPageId page;
    if (PageFromName(name, &page)) return page;
    ESP_LOGW(kTag, "Unknown home page '%s'; falling back to weather", name.c_str());
    return RawDrawPageId::Weather;
}

std::vector<RawDrawPageId> EnabledPages() {
    Settings nvs(kNamespace, false);
    const std::string csv = nvs.GetString(kEnabledKey, kDefaultEnabled);

    std::vector<RawDrawPageId> pages;
    auto add = [&pages](RawDrawPageId id) {
        for (auto existing : pages) {
            if (existing == id) return;
        }
        pages.push_back(id);
    };

    add(HomePage());  // home is always enabled and listed first
    for (const auto& name : SplitCsv(csv)) {
        RawDrawPageId page;
        if (PageFromName(name, &page)) {
            add(page);
        } else {
            ESP_LOGW(kTag, "Ignoring unknown page name '%s'", name.c_str());
        }
    }
    return pages;
}

bool SetHomePage(const std::string& name) {
    RawDrawPageId page;
    if (!PageFromName(name, &page)) return false;
    Settings nvs(kNamespace, true);
    nvs.SetString(kHomeKey, name);
    ESP_LOGI(kTag, "Home page set to '%s'", name.c_str());
    return true;
}

bool SetEnabledPages(const std::vector<std::string>& names) {
    std::string csv;
    for (const auto& name : names) {
        RawDrawPageId page;
        if (!PageFromName(name, &page)) {
            ESP_LOGW(kTag, "Rejecting enabled list: unknown page '%s'", name.c_str());
            return false;
        }
        if (!csv.empty()) csv += ",";
        csv += name;
    }
    if (csv.empty()) return false;
    Settings nvs(kNamespace, true);
    nvs.SetString(kEnabledKey, csv);
    ESP_LOGI(kTag, "Enabled pages set to '%s'", csv.c_str());
    return true;
}

std::string ToJson() {
    std::string json = "{\"home\":\"";
    const char* home = PageName(HomePage());
    json += home ? home : "weather";
    json += "\",\"enabled\":[";
    bool first = true;
    for (auto page : EnabledPages()) {
        const char* name = PageName(page);
        if (!name) continue;
        if (!first) json += ",";
        json += "\"";
        json += name;
        json += "\"";
        first = false;
    }
    json += "]}";
    return json;
}

}  // namespace pageconfig
}  // namespace ui

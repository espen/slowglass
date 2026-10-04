/**
 * @file page_id.h
 * @brief Page identifiers for the rawdraw UI
 *
 * Lives in its own header so widgets and config code can name pages without
 * pulling in the UI manager (which drags board/LVGL headers).
 *
 * An enum entry for a widget that is not compiled into the build is harmless:
 * the widget registry simply has no entry for it and page_config drops the
 * name, so ids stay stable across build configurations.
 */

#ifndef UI_PAGE_ID_H
#define UI_PAGE_ID_H

namespace ui {

enum class RawDrawPageId {
    Chat = 0,
    Ebook = 2,
    Wifi = 3,
    Settings = 4,
    Gallery = 5,
    Weather = 6,
    News = 7,
    WeatherDetail = 8,
    PhotoDetail = 9,
    LifeBar = 10,
    Almanac = 11,
    Log = 12,
    YearProgress = 13,
    Calendar = 14,
    FontDebug = 15,
    FontMetrics = 16,
    APTransfer = 17,
    MakePlans = 18,
    Count,
};

}  // namespace ui

#endif  // UI_PAGE_ID_H

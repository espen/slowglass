/**
 * @file almanac_renderer.cc
 * @brief Almanac page renderer - displays lunar date, solar terms, and almanac info
 *
 * Uses Calendar::ToLunarDate() for lunar calendar conversion (2000-2050).
 * Shows: today's Gregorian date, lunar date, solar term, weekday, and
 * traditional almanac info (yiji - auspicious/inauspicious activities).
 */

#include "almanac_renderer.h"
#include "rawdraw/rawdraw.h"
#include "rawdraw/style.h"
#include "rawdraw/layout_utils.h"  // FIX: 使用 InkCenteredTextTopYInBox 替代 line_height 居中
#include "rawdraw/components/calendar.h"
#include "rawdraw/theme.h"
#include <cstring>
#include <ctime>
#include <cstdio>

// External font references
extern const lv_font_t SourceHanSansSC_Regular_slim;
extern const lv_font_t SourceHanSansSC_Medium_slim;
extern const lv_font_t weather_icons_48;

// Weekday characters (matches calendar.cc)
static const char* kWeekdayFull[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

// Lunar month/day names (same as calendar.cc)
static const char* kLunarMonths[] = {
    "Month 1", "Month 2", "Month 3", "Month 4", "Month 5", "Month 6",
    "Month 7", "Month 8", "Month 9", "Month 10", "Month 11", "Month 12"
};
static const char* kLunarDays[] = {
    "D1", "D2", "D3", "D4", "D5", "D6", "D7", "D8", "D9", "D10",
    "D11", "D12", "D13", "D14", "D15", "D16", "D17", "D18", "D19", "D20",
    "D21", "D22", "D23", "D24", "D25", "D26", "D27", "D28", "D29", "D30"
};

// Tian Gan / Di Zhi (used via Calendar::GetLunarYearName)
// static const char* kTianGan[] = ...;  // Not used directly, Calendar handles it
// static const char* kDiZhi[] = ...;

// Solar terms (same as calendar.cc)
struct SolarTermEntry {
    int month;
    int day;
    const char* name;
};
static const SolarTermEntry kSolarTerms[] = {
    { 1,  5, "Xiaohan" }, { 1, 20, "Dahan" },
    { 2,  4, "Lichun" }, { 2, 19, "Yushui" },
    { 3,  5, "Jingzhe" }, { 3, 20, "Chunfen" },
    { 4,  4, "Qingming" }, { 4, 20, "Guyu" },
    { 5,  5, "Lixia" }, { 5, 21, "Xiaoman" },
    { 6,  5, "Mangzhong" }, { 6, 21, "Xiazhi" },
    { 7,  7, "Xiaoshu" }, { 7, 23, "Dashu" },
    { 8,  7, "Liqiu" }, { 8, 23, "Chushu" },
    { 9,  7, "Bailu" }, { 9, 23, "Qiufen" },
    {10,  8, "Hanlu" }, {10, 23, "Shuangjiang" },
    {11,  7, "Lidong" }, {11, 22, "Xiaoxue" },
    {12,  7, "Daxue" }, {12, 22, "Dongzhi" },
};

static const char* GetSolarTerm(int month, int day) {
    for (size_t i = 0; i < sizeof(kSolarTerms) / sizeof(kSolarTerms[0]); i++) {
        if (kSolarTerms[i].month == month && kSolarTerms[i].day == day) {
            return kSolarTerms[i].name;
        }
    }
    return nullptr;
}

// Simplified yiji (DoAvoid) based on lunar day patterns
// This is a traditional approximation, not a full almanac calculation
static const char* kYiTable[][4] = {
    {"Rituals", "Blessings", "Travel", "Groundwork"},
    {"Weddings", "Proposals", "Engagement", "Travel"},
    {"Opening", "Trading", "Contracts", "Wealth"},
    {"Digging", "Exhumation", "Burial", "Tomb work"},
    {"Building", "Groundwork", "Foundation", "Cornerstone"},
    {"Bed setup", "Opening", "Trading", "Contracts"},
    {"Rituals", "Bathing", "Cleaning", "Building"},
    {"Blessings", "Fertility", "Travel", "Cleansing"},
    {"Weddings", "Rituals", "Blessings", "Travel"},
    {"Opening", "Contracts", "Trading", "Wealth"},
};
static const char* kJiTable[][3] = {
    {"Digging", "Burial", "Exhumation"},
    {"Storehouse", "Shipping", "Harvest"},
    {"Lawsuits", "Disputes", "Gossip"},
    {"Weddings", "Travel", "Blessings"},
    {"Bed setup", "Moving", "Housewarming"},
    {"Rituals", "Building", "Groundwork"},
    {"Opening", "Wealth", "Trading"},
    {"Travel", "Cleansing", "Demolition"},
    {"Digging", "Exhumation", "Burial"},
    {"Proposals", "Engagement", "Weddings"},
};

namespace rawdraw {

AlmanacRenderer::AlmanacRenderer()
    : font_(&SourceHanSansSC_Regular_slim)
    , title_font_(&SourceHanSansSC_Medium_slim)
    , icon_font_(&weather_icons_48) {
}

AlmanacRenderer::~AlmanacRenderer() = default;

void AlmanacRenderer::Init(int width, int height) {
    width_ = width;
    height_ = height;
    needs_full_refresh_ = true;
    RefreshData();
}

void AlmanacRenderer::RefreshData() {
    time_t now = time(nullptr);
    localtime_r(&now, &tm_);

    year_ = tm_.tm_year + 1900;
    month_ = tm_.tm_mon + 1;
    day_ = tm_.tm_mday;
    weekday_ = tm_.tm_wday;  // 0=Sun

    // Lunar date via Calendar algorithm
    lunar_ = Calendar::ToLunarDate(year_, month_, day_);
    lunar_year_name_ = Calendar::GetLunarYearName(year_);

    // Solar term
    solar_term_ = GetSolarTerm(month_, day_);

    // Yiji (DoAvoid) - simplified based on lunar day
    int yi_idx = (lunar_.lunar_day - 1) % 10;
    int ji_idx = (lunar_.lunar_day) % 10;
    yi_ = kYiTable[yi_idx];
    ji_ = kJiTable[ji_idx];
}

void AlmanacRenderer::Render(uint8_t* fb, int width, int height) {
    if (!fb) return;

    const int content_top = Style::kStatusBarHeight + kTitleBarH + Style::kSpacingXS;
    int y = content_top + Style::kSpacingMD;
    const auto& theme = ThemeManager::Get();
    const Color text = theme.ColorFor(ThemeToken::TextPrimary);
    const Color secondary = theme.ColorFor(ThemeToken::TextSecondary);
    const Color accent = theme.ColorFor(ThemeToken::Accent);
    const Color danger = theme.ColorFor(ThemeToken::Danger);
    const Color border = theme.ColorFor(ThemeToken::Border);

    // === Title bar ===
    DrawTitleBar(fb, width);

    // === Large lunar year name + date ===
    // e.g. "丙午年 Month 3D8"
    char lunar_full[32];
    if (lunar_.lunar_month > 0 && lunar_.lunar_day > 0) {
        snprintf(lunar_full, sizeof(lunar_full), "%s · %s %s",
                 lunar_year_name_, GetLunarMonthName(lunar_.lunar_month),
                 GetLunarDayName(lunar_.lunar_day));
    } else {
        snprintf(lunar_full, sizeof(lunar_full), "%s", lunar_year_name_);
    }

    // Draw centered
    int lunar_w = MeasureTextWidth(lunar_full, title_font_);
    int lunar_x = (width - lunar_w) / 2;
    DrawText(fb, width, lunar_x, y, lunar_full, title_font_, accent);
    y += title_font_->line_height + Style::kSpacingMD;

    // === Gregorian date ===
    char greg_buf[64];
    snprintf(greg_buf, sizeof(greg_buf), "Solar %d/%d/%d %s",
             year_, month_, day_, kWeekdayFull[weekday_]);
    int greg_w = MeasureTextWidth(greg_buf, font_);
    int greg_x = (width - greg_w) / 2;
    DrawText(fb, width, greg_x, y, greg_buf, font_, secondary);
    y += font_->line_height + Style::kSpacingMD;

    // === Solar term (if today) ===
    if (solar_term_) {
        char st_buf[32];
        snprintf(st_buf, sizeof(st_buf), "【%s】", solar_term_);
        int st_w = MeasureTextWidth(st_buf, title_font_);
        int st_x = (width - st_w) / 2;
        DrawText(fb, width, st_x, y, st_buf, title_font_, accent);
        y += title_font_->line_height + Style::kSpacingMD;
    }

    // === Divider ===
    DrawHLine(fb, width, y, Style::kSpacingLG, width - Style::kSpacingLG, border);
    y += Style::kSpacingSM;

    // === Do (auspicious) section ===
    DrawText(fb, width, Style::kSpacingLG, y, "Do", title_font_, accent);
    int yi_label_w = MeasureTextWidth("Do", title_font_);
    int yi_start = Style::kSpacingLG + yi_label_w + Style::kSpacingSM;
    int yi_y = y;
    for (int i = 0; i < 4; i++) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%s", yi_[i]);
        DrawText(fb, width, yi_start + i * 60, yi_y, buf, font_, text);
    }
    y += font_->line_height + Style::kSpacingMD;

    // === Avoid (inauspicious) section ===
    DrawText(fb, width, Style::kSpacingLG, y, "Avoid", title_font_, danger);
    int ji_label_w = MeasureTextWidth("Avoid", title_font_);
    int ji_start = Style::kSpacingLG + ji_label_w + Style::kSpacingSM;
    int ji_y = y;
    for (int i = 0; i < 3; i++) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%s", ji_[i]);
        DrawText(fb, width, ji_start + i * 60, ji_y, buf, font_, text);
    }

    needs_full_refresh_ = false;
}

void AlmanacRenderer::DrawTitleBar(uint8_t* fb, int width) {
    const auto& theme = ThemeManager::Get();
    const PaintStyle bar_style = theme.Style(ThemeToken::BackgroundSecondary);
    const Color text = theme.ColorFor(ThemeToken::TextPrimary);
    const Color border = theme.ColorFor(ThemeToken::Border);
    const int title_y_start = Style::kStatusBarHeight;
    const int title_bar_h = kTitleBarH;

    // Background
    DrawStyledRect(fb, width, {0, title_y_start, width, title_bar_h}, bar_style);

    // Top divider (2px)
    DrawHLine(fb, width, title_y_start, 0, width, border);
    DrawHLine(fb, width, title_y_start + 1, 0, width, border);

    // Bottom divider (2px)
    const int line_y = title_y_start + title_bar_h - 2;
    DrawHLine(fb, width, line_y, 0, width, border);
    DrawHLine(fb, width, line_y + 1, 0, width, border);

    // FIX: 改用 InkCenteredTextTopYInBox，避免 line_height 居中导致中文偏上
    // 参见 wiki/projects/notellm-baseline-alignment.md
    int title_text_y = InkCenteredTextTopYInBox(font_, "Almanac", title_y_start, title_bar_h, 1);
    DrawText(fb, width, Style::kSpacingLG, title_text_y, "Almanac", font_, text);
}

const char* AlmanacRenderer::GetLunarMonthName(int month) {
    if (month < 1 || month > 12) return "";
    return kLunarMonths[month - 1];
}

const char* AlmanacRenderer::GetLunarDayName(int day) {
    if (day < 1 || day > 30) return "";
    return kLunarDays[day - 1];
}

bool AlmanacRenderer::HandleInput(const ButtonEvent& event) {
    switch (event.type) {
        case ButtonEvent::kUpClick:
        case ButtonEvent::kDownClick:
            // Navigate months (UP=prev, DOWN=next)
            if (event.type == ButtonEvent::kUpClick) {
                month_--;
                if (month_ < 1) { month_ = 12; year_--; }
            } else {
                month_++;
                if (month_ > 12) { month_ = 1; year_++; }
            }
            lunar_ = Calendar::ToLunarDate(year_, month_, day_);
            solar_term_ = GetSolarTerm(month_, day_);
            needs_full_refresh_ = true;
            return true;

        case ButtonEvent::kBootLongPress:
            // Jump to today
            RefreshData();
            needs_full_refresh_ = true;
            return true;

        default:
            break;
    }
    return false;
}

}  // namespace rawdraw

/**
 * @file weather_renderer.cc
 * @brief Rawdraw weather dashboard for 400x300 BWRY EPD
 *
 * Layout (see design/mockup.html in the project root):
 *   header  — city left, date right, heavy rule
 *   hero    — 7-segment temperature, condition, H/L range, feels-like;
 *             big drawn icon right
 *   today   — 4 rolling hourly slots (+2/+4/+6/+8 h): time, mini icon, temp
 *   strip   — inverted "TOMORROW" band: tag, small icon, high/low, condition
 *
 * Color semantics: YELLOW = sun & low temps, RED = precipitation & freezing,
 * BLACK/WHITE = structure. Red only appears when the weather warrants it.
 *
 * The hero temperature is drawn as 7-segment digits with rects because the
 * largest embedded text font is 24 px.
 */

#include "weather_renderer.h"

#include "widgets/weather/weather_api.h"
#include "rawdraw/layout_utils.h"
#include "rawdraw/rawdraw.h"
#include "rawdraw/style.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

// External font references
// Proper Western fonts (Source Sans 3, full Latin-1) — city names like
// Tromsø need real æøå glyphs with Western metrics.
extern const lv_font_t latin_ui_16;
extern const lv_font_t latin_ui_24;
extern const lv_font_t weather_icons_16;

namespace rawdraw {

namespace {

// ============================================================
// 7-segment digits for the hero temperature
// ============================================================

// Segment bit order: A top, B top-right, C bottom-right, D bottom,
// E bottom-left, F top-left, G middle
constexpr uint8_t kSegments[10] = {
    0b0111111,  // 0: ABCDEF
    0b0000110,  // 1: BC
    0b1011011,  // 2: ABDEG
    0b1001111,  // 3: ABCDG
    0b1100110,  // 4: BCFG
    0b1101101,  // 5: ACDFG
    0b1111101,  // 6: ACDEFG
    0b0000111,  // 7: ABC
    0b1111111,  // 8: all
    0b1101111,  // 9: ABCDFG
};

struct DigitMetrics {
    int w = 42;       // digit width
    int h = 76;       // digit height
    int t = 10;       // segment thickness
    int gap = 10;     // spacing between digits
};

void DrawSegDigit(uint8_t* fb, int width, int x, int y, int digit, const DigitMetrics& m, Color c) {
    if (digit < 0 || digit > 9) return;
    const uint8_t s = kSegments[digit];
    const int half = m.h / 2;
    if (s & 0b0000001) DrawRect(fb, width, {x, y, m.w, m.t}, c);                              // A
    if (s & 0b0000010) DrawRect(fb, width, {x + m.w - m.t, y, m.t, half}, c);                 // B
    if (s & 0b0000100) DrawRect(fb, width, {x + m.w - m.t, y + half, m.t, m.h - half}, c);    // C
    if (s & 0b0001000) DrawRect(fb, width, {x, y + m.h - m.t, m.w, m.t}, c);                  // D
    if (s & 0b0010000) DrawRect(fb, width, {x, y + half, m.t, m.h - half}, c);                // E
    if (s & 0b0100000) DrawRect(fb, width, {x, y, m.t, half}, c);                             // F
    if (s & 0b1000000) DrawRect(fb, width, {x, y + half - m.t / 2, m.w, m.t}, c);             // G
}

// Draws e.g. "-7°"; returns total width used
int DrawSegTemperature(uint8_t* fb, int width, int x, int y, int temp, Color c,
                       const DigitMetrics& m = DigitMetrics{}) {
    int cx = x;
    if (temp < 0) {
        DrawRect(fb, width, {cx, y + m.h / 2 - m.t / 2, m.w - 12, m.t}, c);  // minus
        cx += m.w - 12 + m.gap;
        temp = -temp;
    }
    char digits[16];
    snprintf(digits, sizeof(digits), "%d", temp);
    for (const char* p = digits; *p; ++p) {
        DrawSegDigit(fb, width, cx, y, *p - '0', m, c);
        cx += m.w + m.gap;
    }
    // degree mark, scaled to the digit height
    const int deg_r = m.h / 9;
    DrawCircleBorder(fb, width, {cx + deg_r, y + deg_r + 1}, deg_r, m.t / 3 + 1, c);
    cx += deg_r * 2 + 4;
    return cx - x;
}

// ============================================================
// Large hero icons drawn with primitives
// ============================================================

void DrawThickLine(uint8_t* fb, int width, Point a, Point b, int thickness, Color c) {
    for (int i = 0; i < thickness; ++i) {
        DrawLine(fb, width, {a.x + i, a.y}, {b.x + i, b.y}, c);
        DrawLine(fb, width, {a.x, a.y + i}, {b.x, b.y + i}, c);
    }
}

void DrawSun(uint8_t* fb, int width, int cx, int cy, int r) {
    DrawCircle(fb, width, {cx, cy}, r, YELLOW);
    DrawCircleBorder(fb, width, {cx, cy}, r, 3, BLACK);
    const int inner = r + 6;
    const int outer = r + 16;
    // cardinal rays as rects
    DrawRect(fb, width, {cx - 2, cy - outer, 5, outer - inner}, BLACK);
    DrawRect(fb, width, {cx - 2, cy + inner, 5, outer - inner}, BLACK);
    DrawRect(fb, width, {cx - outer, cy - 2, outer - inner, 5}, BLACK);
    DrawRect(fb, width, {cx + inner, cy - 2, outer - inner, 5}, BLACK);
    // diagonal rays
    const int di = (inner * 7) / 10;
    const int do_ = (outer * 7) / 10;
    DrawThickLine(fb, width, {cx - do_, cy - do_}, {cx - di, cy - di}, 3, BLACK);
    DrawThickLine(fb, width, {cx + di, cy + di}, {cx + do_, cy + do_}, 3, BLACK);
    DrawThickLine(fb, width, {cx + di, cy - do_ + (do_ - di)}, {cx + do_, cy - do_}, 3, BLACK);
    DrawThickLine(fb, width, {cx - do_, cy + do_}, {cx - di, cy + di}, 3, BLACK);
}

void DrawCloudShape(uint8_t* fb, int width, int cx, int cy, int scale, Color fill, Color outline) {
    // Two lobes + a flat base; scale ~ radius of the large lobe
    const int r1 = scale;
    const int r2 = (scale * 3) / 4;
    const Point big{cx + scale / 3, cy};
    const Point small{cx - scale / 2, cy + scale / 5};
    const Rect base{cx - scale, cy + scale / 5, scale * 2, (scale * 4) / 5};
    DrawCircle(fb, width, big, r1, fill);
    DrawCircle(fb, width, small, r2, fill);
    DrawRect(fb, width, base, fill);
    if (outline != fill) {
        DrawCircleBorder(fb, width, big, r1, 3, outline);
        DrawCircleBorder(fb, width, small, r2, 3, outline);
        // repaint interior over the borders that crossed into the body
        DrawCircle(fb, width, big, r1 - 3, fill);
        DrawCircle(fb, width, small, r2 - 3, fill);
        DrawRect(fb, width, {base.x + 3, base.y, base.w - 6, base.h - 3}, fill);
        DrawHLine(fb, width, base.y + base.h - 1, base.x + 2, base.x + base.w - 2, outline);
        DrawHLine(fb, width, base.y + base.h - 2, base.x + 2, base.x + base.w - 2, outline);
        DrawHLine(fb, width, base.y + base.h - 3, base.x + 2, base.x + base.w - 2, outline);
        DrawVLine(fb, width, base.x, base.y, base.y + base.h - 1, outline);
        DrawVLine(fb, width, base.x + 1, base.y, base.y + base.h - 1, outline);
        DrawVLine(fb, width, base.x + 2, base.y, base.y + base.h - 1, outline);
        DrawVLine(fb, width, base.x + base.w - 1, base.y, base.y + base.h - 1, outline);
        DrawVLine(fb, width, base.x + base.w - 2, base.y, base.y + base.h - 1, outline);
        DrawVLine(fb, width, base.x + base.w - 3, base.y, base.y + base.h - 1, outline);
    }
}

void DrawRainDrops(uint8_t* fb, int width, int cx, int drop_y, Color c) {
    for (int i = -1; i <= 1; ++i) {
        const int x = cx + i * 24;
        DrawThickLine(fb, width, {x, drop_y}, {x - 6, drop_y + 16}, 4, c);
    }
}

void DrawMoon(uint8_t* fb, int width, int cx, int cy, int r) {
    // Full moon with craters: no rays (vs. sun), reads cleanly at small sizes.
    DrawCircle(fb, width, {cx, cy}, r, YELLOW);
    DrawCircleBorder(fb, width, {cx, cy}, r, 3, BLACK);
    DrawCircleBorder(fb, width, {cx - r / 3, cy - r / 4}, r / 5, 2, BLACK);
    DrawCircleBorder(fb, width, {cx + r / 4, cy + r / 8}, r / 6, 2, BLACK);
    DrawCircleBorder(fb, width, {cx - r / 8, cy + r / 2 - 3}, r / 7, 2, BLACK);
}

void DrawHeroIcon(uint8_t* fb, int width, WeatherIcon icon, int cx, int cy) {
    switch (icon) {
        case WeatherIcon::Sunny:
            DrawSun(fb, width, cx, cy, 26);
            break;
        case WeatherIcon::ClearNight:
            DrawMoon(fb, width, cx, cy, 32);
            break;
        case WeatherIcon::PartlyCloudy:
            DrawSun(fb, width, cx - 14, cy - 14, 18);
            DrawCloudShape(fb, width, cx + 8, cy + 10, 22, WHITE, BLACK);
            break;
        case WeatherIcon::PartlyCloudyNight:
            DrawMoon(fb, width, cx - 14, cy - 14, 22);
            DrawCloudShape(fb, width, cx + 8, cy + 10, 22, WHITE, BLACK);
            break;
        case WeatherIcon::Cloudy:
        case WeatherIcon::Overcast:
            DrawCloudShape(fb, width, cx, cy - 4, 26, WHITE, BLACK);
            break;
        case WeatherIcon::Rain:
            DrawCloudShape(fb, width, cx, cy - 14, 22, BLACK, BLACK);
            DrawRainDrops(fb, width, cx, cy + 22, RED);
            break;
        case WeatherIcon::Snow:
            DrawCloudShape(fb, width, cx, cy - 14, 22, BLACK, BLACK);
            for (int i = -1; i <= 1; ++i) {
                DrawCircleBorder(fb, width, {cx + i * 24, cy + 28}, 5, 3, BLACK);
            }
            break;
        case WeatherIcon::Fog:
            DrawCloudShape(fb, width, cx, cy - 14, 22, WHITE, BLACK);
            for (int i = 0; i < 3; ++i) {
                DrawRect(fb, width, {cx - 30, cy + 18 + i * 9, 60, 4}, BLACK);
            }
            break;
        case WeatherIcon::Unknown:
        default:
            DrawCircleBorder(fb, width, {cx, cy}, 24, 3, BLACK);
            break;
    }
}

// ============================================================
// Mini icons for the TODAY timeline slots (~36 px box, white bg)
// ============================================================

void DrawMiniCloud(uint8_t* fb, int width, int cx, int cy, int s, Color fill, Color outline) {
    // Simplified two-lobe cloud; 2 px outline so small sizes don't clog
    const Point big{cx + s / 3, cy};
    const Point small{cx - s / 2, cy + s / 5};
    const Rect base{cx - s, cy + s / 5, s * 2, (s * 3) / 5};
    DrawCircle(fb, width, big, s, outline);
    DrawCircle(fb, width, small, (s * 3) / 4, outline);
    DrawRect(fb, width, base, outline);
    if (outline != fill) {
        DrawCircle(fb, width, big, s - 2, fill);
        DrawCircle(fb, width, small, (s * 3) / 4 - 2, fill);
        DrawRect(fb, width, {base.x + 2, base.y, base.w - 4, base.h - 2}, fill);
    }
}

void DrawMiniSun(uint8_t* fb, int width, int cx, int cy, int r) {
    DrawCircle(fb, width, {cx, cy}, r, YELLOW);
    DrawCircleBorder(fb, width, {cx, cy}, r, 2, BLACK);
    const int in = r + 3, out = r + 8;
    DrawRect(fb, width, {cx - 1, cy - out, 3, out - in}, BLACK);
    DrawRect(fb, width, {cx - 1, cy + in, 3, out - in}, BLACK);
    DrawRect(fb, width, {cx - out, cy - 1, out - in, 3}, BLACK);
    DrawRect(fb, width, {cx + in, cy - 1, out - in, 3}, BLACK);
}

void DrawMiniMoon(uint8_t* fb, int width, int cx, int cy, int r) {
    DrawCircle(fb, width, {cx, cy}, r, YELLOW);
    DrawCircleBorder(fb, width, {cx, cy}, r, 2, BLACK);
    DrawCircleBorder(fb, width, {cx - r / 3, cy - r / 4}, r / 4, 1, BLACK);
    DrawCircleBorder(fb, width, {cx + r / 4, cy + r / 5}, r / 5, 1, BLACK);
}

void DrawMiniIcon(uint8_t* fb, int width, WeatherIcon icon, int cx, int cy) {
    switch (icon) {
        case WeatherIcon::Sunny:
            DrawMiniSun(fb, width, cx, cy, 8);
            break;
        case WeatherIcon::ClearNight:
            DrawMiniMoon(fb, width, cx, cy, 10);
            break;
        case WeatherIcon::PartlyCloudy:
            DrawMiniSun(fb, width, cx - 5, cy - 6, 6);
            DrawMiniCloud(fb, width, cx + 3, cy + 4, 8, WHITE, BLACK);
            break;
        case WeatherIcon::PartlyCloudyNight:
            DrawMiniMoon(fb, width, cx - 5, cy - 6, 7);
            DrawMiniCloud(fb, width, cx + 3, cy + 4, 8, WHITE, BLACK);
            break;
        case WeatherIcon::Cloudy:
        case WeatherIcon::Overcast:
            DrawMiniCloud(fb, width, cx, cy - 2, 10, WHITE, BLACK);
            break;
        case WeatherIcon::Rain:
            DrawMiniCloud(fb, width, cx, cy - 6, 8, BLACK, BLACK);
            for (int i = -1; i <= 1; ++i) {
                const int x = cx + i * 9;
                DrawLine(fb, width, {x, cy + 8}, {x - 3, cy + 15}, RED);
                DrawLine(fb, width, {x + 1, cy + 8}, {x - 2, cy + 15}, RED);
            }
            break;
        case WeatherIcon::Snow:
            DrawMiniCloud(fb, width, cx, cy - 6, 8, BLACK, BLACK);
            for (int i = -1; i <= 1; ++i) {
                DrawCircleBorder(fb, width, {cx + i * 9, cy + 12}, 3, 2, BLACK);
            }
            break;
        case WeatherIcon::Fog:
            DrawMiniCloud(fb, width, cx, cy - 6, 8, WHITE, BLACK);
            DrawRect(fb, width, {cx - 12, cy + 8, 24, 2}, BLACK);
            DrawRect(fb, width, {cx - 12, cy + 13, 24, 2}, BLACK);
            break;
        case WeatherIcon::Unknown:
        default:
            DrawCircleBorder(fb, width, {cx, cy}, 9, 2, BLACK);
            break;
    }
}

// Small glyphs (weather_icons_16 font) for the tomorrow strip
const char* SmallGlyphFor(WeatherIcon icon) {
    switch (icon) {
        case WeatherIcon::Sunny:
        case WeatherIcon::ClearNight:   return "\xef\x83\x9e";  // sun
        case WeatherIcon::PartlyCloudyNight:
        case WeatherIcon::PartlyCloudy:
        case WeatherIcon::Cloudy:
        case WeatherIcon::Overcast:     return "\xef\x83\x82";  // cloud
        case WeatherIcon::Rain:         return "\xef\x83\xa9";  // rain
        case WeatherIcon::Snow:         return "\xef\x8b\x9c";  // snow
        case WeatherIcon::Fog:          return "\xef\x9d\x9f";  // fog
        default:                        return "\xef\x83\x9e";
    }
}

std::string UpperAscii(const std::string& in) {
    std::string out = in;
    for (auto& ch : out) ch = static_cast<char>(toupper(static_cast<unsigned char>(ch)));
    return out;
}

}  // namespace

WeatherRenderer::WeatherRenderer()
    : has_data_(false)
    , page_index_(0)
    , font_(&latin_ui_16)
    , title_font_(&latin_ui_24) {
}

WeatherRenderer::~WeatherRenderer() {}

void WeatherRenderer::Init(int width, int height) {
    width_ = width;
    height_ = height;
    has_data_ = false;
    needs_full_refresh_ = true;
    page_index_ = 0;
    firmware_version_.clear();
}

void WeatherRenderer::Render(uint8_t* fb, int width, int height) {
    if (!fb) return;

    // Chrome-free page: the manager draws no status bar here, we own all 300px.
    const int content_top = 0;
    DrawRect(fb, width, {0, 0, width, height}, WHITE);

    if (!has_data_) {
        const char* empty_text = "Waiting for weather data...";
        const char* hint = "Long-press any button to retry";
        int text_w = MeasureTextWidth(empty_text, font_);
        int hint_w = MeasureTextWidth(hint, font_);
        int center_y = content_top + (height - content_top) / 2;
        DrawText(fb, width, (width - text_w) / 2,
                 InkCenteredTextTopY(font_, empty_text, center_y - 16, 0), empty_text, font_, BLACK);
        DrawText(fb, width, (width - hint_w) / 2,
                 InkCenteredTextTopY(font_, hint, center_y + 16, 0), hint, font_, BLACK);
        needs_full_refresh_ = false;
        return;
    }

    // ---- Header: city left, date right, heavy rule ----
    const int header_y = content_top + 10;
    std::string place = UpperAscii(city_name_.empty() ? current_data_.city : city_name_);
    if (place.empty()) place = "WEATHER";
    DrawText(fb, width, 14, InkCenteredTextTopY(title_font_, place.c_str(), header_y + 13, 0),
             place.c_str(), title_font_, BLACK);
    const std::string& date = current_data_.date_string;
    if (!date.empty()) {
        int date_w = MeasureTextWidth(date.c_str(), font_);
        DrawText(fb, width, width - 14 - date_w,
                 InkCenteredTextTopY(font_, date.c_str(), header_y + 13, 0),
                 date.c_str(), font_, BLACK);
    }
    const int rule_y = header_y + 28;
    DrawRect(fb, width, {0, rule_y, width, 3}, BLACK);

    // ---- Hero: temperature + condition + H/L + feels-like left, icon right ----
    // Digits shrunk vs v1 (76 -> 56 px) to make room for the TODAY band.
    const int hero_y = rule_y + 12;
    const DigitMetrics hero_digits{32, 56, 8, 8};
    const Color temp_color = (current_data_.temp_int < 0) ? RED : BLACK;
    DrawSegTemperature(fb, width, 22, hero_y, current_data_.temp_int, temp_color, hero_digits);

    const int text_x = 24;
    const std::string& cond = current_data_.weather_text;
    if (!cond.empty()) {
        DrawText(fb, width, text_x,
                 InkCenteredTextTopY(font_, cond.c_str(), hero_y + 56 + 12, 0),
                 cond.c_str(), font_, BLACK);
    }

    // Today's high/low (forecast[0] when it is "Today")
    const WeatherForecastDay* today_fc = nullptr;
    const WeatherForecastDay* tomorrow = nullptr;
    for (const auto& day : current_data_.forecast) {
        if (day.label == "Today") today_fc = &day;
        if (day.label == "Tomorrow") tomorrow = &day;
    }
    if (today_fc) {
        char range[40];
        snprintf(range, sizeof(range), "H %d\xC2\xB0 / L %d\xC2\xB0",
                 (int)today_fc->temp_max, (int)today_fc->temp_min);
        DrawText(fb, width, text_x, InkCenteredTextTopY(title_font_, range, hero_y + 89, 0),
                 range, title_font_, BLACK);
    }

    // Feels-like: non-empty only when it differs >= 2° from the actual temp
    if (!current_data_.feels_like.empty()) {
        const char* label = "Feels like ";
        const int feels_cy = hero_y + 109;
        DrawText(fb, width, text_x, InkCenteredTextTopY(font_, label, feels_cy, 0),
                 label, font_, BLACK);
        char feels_buf[16];
        snprintf(feels_buf, sizeof(feels_buf), "%s\xC2\xB0", current_data_.feels_like.c_str());
        const Color feels_color = (atoi(current_data_.feels_like.c_str()) < 0) ? RED : BLACK;
        DrawText(fb, width, text_x + MeasureTextWidth(label, font_),
                 InkCenteredTextTopY(title_font_, feels_buf, feels_cy, 0),
                 feels_buf, title_font_, feels_color);
    }

    WeatherIcon now_icon = ParseWeatherIcon(current_data_.weather_icon.c_str());
    if (now_icon == WeatherIcon::Unknown) {
        now_icon = ParseWeatherIcon(current_data_.weather_text.c_str());
    }
    DrawHeroIcon(fb, width, now_icon, width - 84, hero_y + 52);

    // ---- TODAY band: 4 rolling slots (+2/+4/+6/+8 h; hourly[0] = +1 h) ----
    const int band_top = 170;
    const int strip_h = 40;
    const Rect strip{0, height - strip_h, width, strip_h};
    const int band_bottom = strip.y;

    const auto& hourly = current_data_.hourly;
    int picks[4] = {-1, -1, -1, -1};
    const int n = (int)hourly.size();
    if (n >= 8) {
        picks[0] = 1; picks[1] = 3; picks[2] = 5; picks[3] = 7;
    } else if (n >= 4) {
        // degraded data: spread what we have across the four slots
        picks[0] = 0; picks[1] = n / 3; picks[2] = (2 * n) / 3; picks[3] = n - 1;
    }

    if (picks[0] >= 0) {
        DrawRect(fb, width, {0, band_top, width, 2}, BLACK);
        const int slot_w = width / 4;
        for (int i = 1; i < 4; ++i) {
            DrawVLine(fb, width, i * slot_w, band_top + 4, band_bottom - 4, BLACK);
        }
        for (int i = 0; i < 4; ++i) {
            const WeatherHourly& h = hourly[picks[i]];
            const int cx = i * slot_w + slot_w / 2;

            char hh[8];
            snprintf(hh, sizeof(hh), "%02d", h.hour_local);
            int hh_w = MeasureTextWidth(hh, font_);
            DrawText(fb, width, cx - hh_w / 2, InkCenteredTextTopY(font_, hh, band_top + 13, 0),
                     hh, font_, BLACK);

            DrawMiniIcon(fb, width, ParseWeatherIcon(h.icon_code.c_str()), cx, band_top + 44);

            char st[16];
            snprintf(st, sizeof(st), "%d\xC2\xB0", (int)h.temp);
            int st_w = MeasureTextWidth(st, title_font_);
            DrawText(fb, width, cx - st_w / 2,
                     InkCenteredTextTopY(title_font_, st, band_top + 76, 0),
                     st, title_font_, (h.temp < 0) ? RED : BLACK);
        }
    }

    // ---- Tomorrow strip: inverted band, flush to the bottom edge ----
    DrawRect(fb, width, strip, BLACK);

    if (tomorrow) {
        // yellow tag
        const char* tag = "TOMORROW";
        int tag_w = MeasureTextWidth(tag, font_);
        const Rect tag_box{12, strip.y + (strip_h - 24) / 2, tag_w + 12, 24};
        DrawRect(fb, width, tag_box, YELLOW);
        DrawText(fb, width, tag_box.x + 6,
                 InkCenteredTextTopY(font_, tag, tag_box.y + tag_box.h / 2, 0), tag, font_, BLACK);

        int x = tag_box.x + tag_box.w + 14;
        WeatherIcon icon = ParseWeatherIcon(tomorrow->icon_code.c_str());
        const char* glyph = SmallGlyphFor(icon);
        const Color glyph_color = (icon == WeatherIcon::Rain) ? RED
                                : (icon == WeatherIcon::Sunny) ? YELLOW : WHITE;
        DrawIcon(fb, width, x, InkCenteredTextTopY(&weather_icons_16, glyph, strip.y + strip_h / 2, 0),
                 glyph, &weather_icons_16, glyph_color);
        x += MeasureTextWidth(glyph, &weather_icons_16) + 14;

        char range[32];
        snprintf(range, sizeof(range), "%d\xC2\xB0 / %d\xC2\xB0",
                 (int)tomorrow->temp_max, (int)tomorrow->temp_min);
        // draw max in white, then min part in yellow
        char max_part[16];
        snprintf(max_part, sizeof(max_part), "%d\xC2\xB0 ", (int)tomorrow->temp_max);
        DrawText(fb, width, x, InkCenteredTextTopY(title_font_, range, strip.y + strip_h / 2, 0),
                 max_part, title_font_, WHITE);
        int max_w = MeasureTextWidth(max_part, title_font_);
        char min_part[16];
        snprintf(min_part, sizeof(min_part), "/ %d\xC2\xB0", (int)tomorrow->temp_min);
        DrawText(fb, width, x + max_w,
                 InkCenteredTextTopY(title_font_, range, strip.y + strip_h / 2, 0),
                 min_part, title_font_, YELLOW);

        const std::string& t_cond = tomorrow->weather_text;
        if (!t_cond.empty()) {
            int cond_w = MeasureTextWidth(t_cond.c_str(), font_);
            if (x + max_w + MeasureTextWidth(min_part, title_font_) + 20 + cond_w < width - 12) {
                DrawText(fb, width, width - 12 - cond_w,
                         InkCenteredTextTopY(font_, t_cond.c_str(), strip.y + strip_h / 2, 0),
                         t_cond.c_str(), font_, WHITE);
            }
        }
    } else {
        const char* na = "No forecast for tomorrow";
        DrawText(fb, width, 14, InkCenteredTextTopY(font_, na, strip.y + strip_h / 2, 0),
                 na, font_, WHITE);
    }

    needs_full_refresh_ = false;
}

bool WeatherRenderer::HandleInput(const ButtonEvent& event) {
    switch (event.type) {
        case ButtonEvent::kUpLongPress:
        case ButtonEvent::kDownLongPress:
        case ButtonEvent::kBootLongPress:
            weather_api_fetch_now();
            needs_full_refresh_ = true;
            return true;
        default:
            return false;
    }
}

void WeatherRenderer::Update(const WeatherData& data) {
    current_data_ = data;
    has_data_ = true;
    needs_full_refresh_ = true;
}

void WeatherRenderer::SetCityName(const char* name) {
    city_name_ = name ? name : "";
    needs_full_refresh_ = true;
}

void WeatherRenderer::SetFirmwareVersion(const char* version) {
    firmware_version_ = version ? version : "";
    needs_full_refresh_ = true;
}

}  // namespace rawdraw

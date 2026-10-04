/**
 * @file makeplans_renderer.cc
 * @brief MakePlans meeting-room door sign for 400x300 BWRY EPD
 *
 * Layout (see design/doorsign-mockup.html in the project root):
 *   header — room name left, date right, heavy rule
 *   hero   — OCCUPIED: "BUSY UNTIL" + end time in 7-segment digits on a red
 *            panel; FREE: giant block-letter "FREE" on white
 *   chip   — yellow tag when a transition is <= 10 min away (absolute times
 *            only: the panel refreshes on poll, countdowns would go stale)
 *   strip  — inverted "NEXT" band: tag, start time, title, +N today
 *   footer — MakePlans + data timestamp
 *
 * Color semantics: RED = occupied, WHITE = free, YELLOW = imminent change.
 */

#include "makeplans_renderer.h"

#include "rawdraw/layout_utils.h"
#include "rawdraw/rawdraw.h"
#include "rawdraw/style.h"

#include <cstdio>
#include <cstring>
#include <ctime>
#include <string>

// Proper Western fonts (Source Sans 3, full Latin-1) — the CJK fonts'
// half-width Latin is unsuitable for English/Norwegian room data.
extern const lv_font_t latin_ui_16;
extern const lv_font_t latin_ui_24;

namespace rawdraw {

namespace {

constexpr int64_t kMinValidEpoch = 1600000000;   // clock not synced below this
constexpr int64_t kSoonWindowSec = 10 * 60;      // yellow-chip threshold
constexpr int64_t kBackToBackSlopSec = 5 * 60;   // no "FREE AT" if next starts here

// ============================================================
// 7-segment time (HH:MM) — same technique as the weather hero temp
// ============================================================

constexpr uint8_t kSegments[10] = {
    0b0111111, 0b0000110, 0b1011011, 0b1001111, 0b1100110,
    0b1101101, 0b1111101, 0b0000111, 0b1111111, 0b1101111,
};

struct DigitMetrics {
    int w = 36;
    int h = 68;
    int t = 9;
    int gap = 8;
};

void DrawSegDigit(uint8_t* fb, int width, int x, int y, int digit,
                  const DigitMetrics& m, Color c) {
    if (digit < 0 || digit > 9) return;
    const uint8_t s = kSegments[digit];
    const int half = m.h / 2;
    if (s & 0b0000001) DrawRect(fb, width, {x, y, m.w, m.t}, c);
    if (s & 0b0000010) DrawRect(fb, width, {x + m.w - m.t, y, m.t, half}, c);
    if (s & 0b0000100) DrawRect(fb, width, {x + m.w - m.t, y + half, m.t, m.h - half}, c);
    if (s & 0b0001000) DrawRect(fb, width, {x, y + m.h - m.t, m.w, m.t}, c);
    if (s & 0b0010000) DrawRect(fb, width, {x, y + half, m.t, m.h - half}, c);
    if (s & 0b0100000) DrawRect(fb, width, {x, y, m.t, half}, c);
    if (s & 0b1000000) DrawRect(fb, width, {x, y + half - m.t / 2, m.w, m.t}, c);
}

// Draws "11:00"; returns total width used
int DrawSegTime(uint8_t* fb, int width, int x, int y, const char* hhmm, Color c) {
    DigitMetrics m;
    int cx = x;
    for (const char* p = hhmm; *p; ++p) {
        if (*p == ':') {
            const int dot = m.t;
            DrawRect(fb, width, {cx + 2, y + m.h / 3 - dot / 2, dot, dot}, c);
            DrawRect(fb, width, {cx + 2, y + (m.h * 2) / 3 - dot / 2, dot, dot}, c);
            cx += dot + 10;
        } else if (*p >= '0' && *p <= '9') {
            DrawSegDigit(fb, width, cx, y, *p - '0', m, c);
            cx += m.w + m.gap;
        }
    }
    return cx - x;
}

// ============================================================
// Giant block letters for "FREE" (fonts max out at 24 px)
// ============================================================

struct LetterMetrics {
    int w = 42;
    int h = 68;
    int t = 11;
    int gap = 10;
};

void DrawBlockLetter(uint8_t* fb, int width, int x, int y, char ch,
                     const LetterMetrics& m, Color c) {
    const int half = m.h / 2;
    switch (ch) {
        case 'F':
            DrawRect(fb, width, {x, y, m.t, m.h}, c);                    // stem
            DrawRect(fb, width, {x, y, m.w, m.t}, c);                    // top
            DrawRect(fb, width, {x, y + half - m.t / 2, m.w - 8, m.t}, c);  // mid
            break;
        case 'E':
            DrawRect(fb, width, {x, y, m.t, m.h}, c);
            DrawRect(fb, width, {x, y, m.w, m.t}, c);
            DrawRect(fb, width, {x, y + half - m.t / 2, m.w - 8, m.t}, c);
            DrawRect(fb, width, {x, y + m.h - m.t, m.w, m.t}, c);        // bottom
            break;
        case 'R':
            DrawRect(fb, width, {x, y, m.t, m.h}, c);                    // stem
            DrawRect(fb, width, {x, y, m.w, m.t}, c);                    // top
            DrawRect(fb, width, {x + m.w - m.t, y, m.t, half}, c);       // bowl right
            DrawRect(fb, width, {x, y + half - m.t / 2, m.w, m.t}, c);   // mid
            // leg: stepped diagonal from mid to bottom-right
            for (int i = 0; i < 4; ++i) {
                const int seg_h = (m.h - half) / 4 + 1;
                const int yy = y + half + i * ((m.h - half) / 4);
                const int xx = x + m.t + ((m.w - 2 * m.t) * i) / 4;
                DrawRect(fb, width, {xx, yy, m.t, seg_h}, c);
            }
            break;
        default:
            DrawRect(fb, width, {x, y, m.w, m.h}, c);
            break;
    }
}

int DrawBlockWord(uint8_t* fb, int width, int x, int y, const char* word, Color c) {
    LetterMetrics m;
    int cx = x;
    for (const char* p = word; *p; ++p) {
        DrawBlockLetter(fb, width, cx, y, *p, m, c);
        cx += m.w + m.gap;
    }
    return cx - x;
}

// ============================================================
// Text helpers
// ============================================================

// ASCII + Latin-1 uppercase (UTF-8 aware): "Møterom" -> "MØTEROM".
// Latin-1 lowercase U+00E0..U+00FE maps to uppercase by -0x20 on the
// continuation byte, except ÷ (U+00F7); ÿ has no Latin-1 uppercase.
std::string UpperAscii(const std::string& in) {
    std::string out = in;
    for (size_t i = 0; i < out.size(); ++i) {
        const unsigned char c = (unsigned char)out[i];
        if (c < 0x80) {
            out[i] = static_cast<char>(toupper(c));
        } else if (c == 0xC3 && i + 1 < out.size()) {
            const unsigned char n = (unsigned char)out[i + 1];
            if (n >= 0xA0 && n <= 0xBE && n != 0xB7) out[i + 1] = (char)(n - 0x20);
            ++i;
        }
    }
    return out;
}

// Truncate at a UTF-8 boundary so the string fits max_w; appends ".." if cut.
std::string TruncateToWidth(const std::string& in, const lv_font_t* font, int max_w) {
    if (MeasureTextWidth(in.c_str(), font) <= max_w) return in;
    std::string out = in;
    while (!out.empty()) {
        out.pop_back();
        while (!out.empty() && (out.back() & 0xC0) == 0x80) out.pop_back();
        if (MeasureTextWidth((out + "..").c_str(), font) <= max_w) break;
    }
    return out + "..";
}

void DrawYellowChip(uint8_t* fb, int width, int x, int y, const char* text,
                    const lv_font_t* font) {
    const int text_w = MeasureTextWidth(text, font);
    const Rect box{x, y, text_w + 14, 28};
    DrawRect(fb, width, box, YELLOW);
    DrawText(fb, width, box.x + 7,
             InkCenteredTextTopY(font, text, box.y + box.h / 2, 0), text, font, BLACK);
}

}  // namespace

MakePlansRenderer::MakePlansRenderer()
    : font_(&latin_ui_16)
    , title_font_(&latin_ui_24) {
}

MakePlansRenderer::~MakePlansRenderer() {}

void MakePlansRenderer::Init(int width, int height) {
    width_ = width;
    height_ = height;
    needs_full_refresh_ = true;
}

void MakePlansRenderer::RenderMessage(uint8_t* fb, int width, int height,
                                      const char* line1, const char* line2) {
    DrawRect(fb, width, {0, 0, width, height}, WHITE);
    const int center_y = height / 2;
    int w1 = MeasureTextWidth(line1, font_);
    DrawText(fb, width, (width - w1) / 2,
             InkCenteredTextTopY(font_, line1, center_y - 16, 0), line1, font_, BLACK);
    if (line2) {
        int w2 = MeasureTextWidth(line2, font_);
        DrawText(fb, width, (width - w2) / 2,
                 InkCenteredTextTopY(font_, line2, center_y + 16, 0), line2, font_, BLACK);
    }
}

void MakePlansRenderer::RenderSetupScreen(uint8_t* fb, int width, int height,
                                          bool expired) {
    DrawRect(fb, width, {0, 0, width, height}, WHITE);

    // Header: same bones as the live door sign
    const char* title = "DOOR SIGN";
    DrawText(fb, width, 14, InkCenteredTextTopY(title_font_, title, 23, 0),
             title, title_font_, BLACK);
    const char* corner = "SETUP";
    int cw = MeasureTextWidth(corner, font_);
    DrawText(fb, width, width - 14 - cw, InkCenteredTextTopY(font_, corner, 23, 0),
             corner, font_, BLACK);
    DrawRect(fb, width, {0, 38, width, 3}, BLACK);

    // State headline, with a yellow chip so the panel reads at a glance
    const char* state = expired ? "PAIRING EXPIRED" : "NOT PAIRED";
    DrawText(fb, width, 16, InkCenteredTextTopY(title_font_, state, 66, 0),
             state, title_font_, expired ? RED : BLACK);

    const char* steps[3] = {
        expired ? "The sign lost access to the schedule."
                : "1. MakePlans admin: resource > Room display",
        expired ? "Generate a new pairing code, then:"
                : "2. Generate a pairing code (valid 10 min)",
        expired ? "open the address below and pair again."
                : "3. Open the address below and pair",
    };
    int y = 104;
    for (const char* line : steps) {
        DrawText(fb, width, 16, InkCenteredTextTopY(font_, line, y, 0), line, font_, BLACK);
        y += 30;
    }

    // Address strip: inverted band, URL on its own line so it never clips
    const int strip_h = 52;
    const int footer_h = 26;
    const Rect strip{0, height - footer_h - strip_h, width, strip_h};
    DrawRect(fb, width, strip, BLACK);
    if (!lan_url_.empty()) {
        const char* tag = "OPEN";
        int tag_w = MeasureTextWidth(tag, font_);
        const Rect tag_box{12, strip.y + (strip_h - 28) / 2, tag_w + 12, 28};
        DrawRect(fb, width, tag_box, YELLOW);
        DrawText(fb, width, tag_box.x + 6,
                 InkCenteredTextTopY(font_, tag, tag_box.y + tag_box.h / 2, 0),
                 tag, font_, BLACK);
        const int x = tag_box.x + tag_box.w + 14;
        std::string url = TruncateToWidth(lan_url_, font_, width - x - 12);
        DrawText(fb, width, x,
                 InkCenteredTextTopY(font_, url.c_str(), strip.y + strip_h / 2, 0),
                 url.c_str(), font_, WHITE);
    } else {
        const char* na = "Power over USB to enable setup";
        DrawText(fb, width, 14, InkCenteredTextTopY(font_, na, strip.y + strip_h / 2, 0),
                 na, font_, WHITE);
    }

    const int footer_center = height - footer_h / 2;
    DrawText(fb, width, 14, InkCenteredTextTopY(font_, "MakePlans", footer_center, 0),
             "MakePlans", font_, BLACK);
    needs_full_refresh_ = false;
}

void MakePlansRenderer::Render(uint8_t* fb, int width, int height) {
    if (!fb) return;

    if (has_data_ && schedule_.unpaired) {
        RenderSetupScreen(fb, width, height, true);
        return;
    }
    if (!has_data_) {
        if (makeplans_is_configured()) {
            RenderMessage(fb, width, height, "Waiting for schedule...",
                          "Long-press any button to retry");
        } else {
            RenderSetupScreen(fb, width, height, false);
        }
        needs_full_refresh_ = false;
        return;
    }

    const int64_t now = (int64_t)time(nullptr);
    if (now < kMinValidEpoch) {
        RenderMessage(fb, width, height, "Syncing clock...", nullptr);
        needs_full_refresh_ = false;
        return;
    }

    // ---- Occupancy state from the schedule ----
    const MakePlansEntry* current = nullptr;
    const MakePlansEntry* next = nullptr;
    int remaining_after_next = 0;
    for (const auto& e : schedule_.entries) {
        if (e.start_epoch <= now && now < e.end_epoch && !current) current = &e;
        if (e.start_epoch > now) {
            if (!next) next = &e;
            else remaining_after_next++;
        }
    }
    const bool occupied = current != nullptr;

    const Color bg = occupied ? RED : WHITE;
    const Color fg = occupied ? WHITE : BLACK;
    DrawRect(fb, width, {0, 0, width, height}, bg);

    // ---- Header: room name left, date right ----
    std::string room = UpperAscii(schedule_.room_title.empty() ? "DOOR SIGN"
                                                               : schedule_.room_title);
    room = TruncateToWidth(room, title_font_, width - 140);
    DrawText(fb, width, 14, InkCenteredTextTopY(title_font_, room.c_str(), 23, 0),
             room.c_str(), title_font_, fg);
    if (!schedule_.fetched_date.empty()) {
        int dw = MeasureTextWidth(schedule_.fetched_date.c_str(), font_);
        DrawText(fb, width, width - 14 - dw,
                 InkCenteredTextTopY(font_, schedule_.fetched_date.c_str(), 23, 0),
                 schedule_.fetched_date.c_str(), font_, fg);
    }
    DrawRect(fb, width, {0, 38, width, 3}, fg);

    // ---- Hero ----
    const int label_y = 56;
    const int big_y = 74;
    const int meta_y = big_y + 68 + 18;
    std::string chip_text;

    if (occupied) {
        DrawText(fb, width, 16, InkCenteredTextTopY(font_, "BUSY UNTIL", label_y, 0),
                 "BUSY UNTIL", font_, fg);
        DrawSegTime(fb, width, 16, big_y, current->end_hhmm.c_str(), fg);

        std::string meta = current->title;
        if (!current->person.empty()) meta += " - " + current->person;
        if (current->tentative) meta += " (tentative)";
        meta = TruncateToWidth(meta, font_, width - 32);
        DrawText(fb, width, 16, InkCenteredTextTopY(font_, meta.c_str(), meta_y, 0),
                 meta.c_str(), font_, fg);

        const bool back_to_back =
            next && next->start_epoch <= current->end_epoch + kBackToBackSlopSec;
        if (current->end_epoch - now <= kSoonWindowSec && !back_to_back) {
            chip_text = "FREE AT " + current->end_hhmm;
        }
    } else {
        DrawBlockWord(fb, width, 16, big_y, "FREE", BLACK);

        std::string meta = next ? ("until " + next->start_hhmm) : "rest of the day";
        DrawText(fb, width, 16, InkCenteredTextTopY(font_, meta.c_str(), meta_y, 0),
                 meta.c_str(), font_, BLACK);

        if (next && next->start_epoch - now <= kSoonWindowSec) {
            std::string what = UpperAscii(next->title);
            what = TruncateToWidth(what, font_, width - 160);
            chip_text = what + " AT " + next->start_hhmm;
        }
    }
    if (!chip_text.empty()) {
        DrawYellowChip(fb, width, 16, meta_y + 16, chip_text.c_str(), font_);
    }

    // ---- NEXT strip: inverted band above the footer ----
    const int strip_h = 52;
    const int footer_h = 26;
    const Rect strip{0, height - footer_h - strip_h, width, strip_h};
    DrawRect(fb, width, strip, BLACK);

    if (next) {
        const char* tag = "NEXT";
        int tag_w = MeasureTextWidth(tag, font_);
        const Rect tag_box{12, strip.y + (strip_h - 28) / 2, tag_w + 12, 28};
        DrawRect(fb, width, tag_box, YELLOW);
        DrawText(fb, width, tag_box.x + 6,
                 InkCenteredTextTopY(font_, tag, tag_box.y + tag_box.h / 2, 0),
                 tag, font_, BLACK);

        int x = tag_box.x + tag_box.w + 14;
        DrawText(fb, width, x,
                 InkCenteredTextTopY(title_font_, next->start_hhmm.c_str(),
                                     strip.y + strip_h / 2, 0),
                 next->start_hhmm.c_str(), title_font_, YELLOW);
        x += MeasureTextWidth(next->start_hhmm.c_str(), title_font_) + 12;

        char more[20] = "";
        int more_w = 0;
        if (remaining_after_next > 0) {
            snprintf(more, sizeof(more), "+%d today", remaining_after_next);
            more_w = MeasureTextWidth(more, font_) + 12;
        }
        std::string what = TruncateToWidth(next->title, font_, width - x - more_w - 12);
        DrawText(fb, width, x,
                 InkCenteredTextTopY(font_, what.c_str(), strip.y + strip_h / 2, 0),
                 what.c_str(), font_, WHITE);
        if (more[0]) {
            DrawText(fb, width, width - 12 - MeasureTextWidth(more, font_),
                     InkCenteredTextTopY(font_, more, strip.y + strip_h / 2, 0),
                     more, font_, WHITE);
        }
    } else {
        const char* na = "No more bookings today";
        DrawText(fb, width, 14, InkCenteredTextTopY(font_, na, strip.y + strip_h / 2, 0),
                 na, font_, WHITE);
    }

    // ---- Footer ----
    // Static text only: any change here would cost a 10s full e-paper
    // refresh. NFC state is visible via GET /api/makeplans (nfc_active).
    const int footer_center = height - footer_h / 2;
    DrawText(fb, width, 14, InkCenteredTextTopY(font_, "MakePlans", footer_center, 0),
             "MakePlans", font_, fg);
    if (!schedule_.fetched_hhmm.empty()) {
        char updated[24];
        snprintf(updated, sizeof(updated), "Updated %s", schedule_.fetched_hhmm.c_str());
        int w = MeasureTextWidth(updated, font_);
        DrawText(fb, width, width - 14 - w,
                 InkCenteredTextTopY(font_, updated, footer_center, 0), updated, font_, fg);
    }

    needs_full_refresh_ = false;
}

bool MakePlansRenderer::HandleInput(const ButtonEvent& event) {
    switch (event.type) {
        case ButtonEvent::kUpLongPress:
        case ButtonEvent::kDownLongPress:
        case ButtonEvent::kBootLongPress:
            makeplans_api_fetch_now();
            needs_full_refresh_ = true;
            return true;
        default:
            return false;
    }
}

void MakePlansRenderer::Update(const MakePlansSchedule& schedule) {
    schedule_ = schedule;
    has_data_ = true;
    needs_full_refresh_ = true;
}

void MakePlansRenderer::SetLanUrl(const std::string& url) {
    lan_url_ = url;
}

}  // namespace rawdraw

/**
 * @file makeplans_renderer.h
 * @brief MakePlans meeting-room door sign renderer (400x300 BWRY)
 */

#ifndef RAWDRAW_MAKEPLANS_RENDERER_H
#define RAWDRAW_MAKEPLANS_RENDERER_H

#include "widgets/makeplans/makeplans_api.h"
#include "ui/renderers/rawdraw/page_renderer.h"
#include "rawdraw/style.h"
#include <string>

namespace rawdraw {

/**
 * @brief Door sign for one MakePlans resource.
 *
 * Full-bleed (chrome-free) page. Occupancy state is computed at render time
 * from the latest schedule + the device clock:
 *   OCCUPIED (red panel) / FREE (white panel), with yellow absolute-time
 *   chips when a transition is <= 10 minutes away. No clocks or countdowns —
 *   the panel only redraws on poll, so everything shown must stay true
 *   between refreshes.
 */
class MakePlansRenderer : public PageRenderer {
public:
    MakePlansRenderer();
    ~MakePlansRenderer() override;

    void Init(int width, int height) override;
    void Render(uint8_t* fb, int width, int height) override;
    bool HandleInput(const ButtonEvent& event) override;
    // Full-bleed dashboard with its own header — no status bar.
    bool WantsFullBleed() const override { return true; }

    /** New schedule from the data source. */
    void Update(const MakePlansSchedule& schedule);

    /** LAN config URL shown on the setup screen (set when the server starts). */
    void SetLanUrl(const std::string& url);

private:
    void RenderSetupScreen(uint8_t* fb, int width, int height, bool expired);
    void RenderMessage(uint8_t* fb, int width, int height,
                       const char* line1, const char* line2);

    MakePlansSchedule schedule_;
    bool has_data_ = false;
    std::string lan_url_;

    const lv_font_t* font_ = nullptr;
    const lv_font_t* title_font_ = nullptr;
};

}  // namespace rawdraw

#endif  // RAWDRAW_MAKEPLANS_RENDERER_H

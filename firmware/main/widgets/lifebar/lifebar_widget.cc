/**
 * @file lifebar_widget.cc
 * @brief Life-progress bar widget (renderer-only page)
 */

#include "widgets/widget.h"

#include "widgets/lifebar/lifebar_renderer.h"

namespace widgets {

void RegisterLifeBarWidget() {
    WidgetDef def;
    def.name = "lifebar";
    def.pages = {
        {ui::RawDrawPageId::LifeBar, "lifebar", "Life Progress", nullptr,
         []() -> rawdraw::PageRenderer* { return new rawdraw::LifeBarRenderer(); }},
    };
    Register(std::move(def));
}

}  // namespace widgets

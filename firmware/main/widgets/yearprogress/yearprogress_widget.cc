/**
 * @file yearprogress_widget.cc
 * @brief Year-progress widget (renderer-only page)
 */

#include "widgets/widget.h"

#include "ui/renderers/rawdraw/yearprogress_renderer.h"

namespace widgets {

void RegisterYearProgressWidget() {
    WidgetDef def;
    def.name = "yearprogress";
    def.pages = {
        {ui::RawDrawPageId::YearProgress, "yearprogress", "Year Progress", nullptr,
         []() -> rawdraw::PageRenderer* { return new rawdraw::YearProgressRenderer(); }},
    };
    Register(std::move(def));
}

}  // namespace widgets

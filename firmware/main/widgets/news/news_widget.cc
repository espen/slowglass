/**
 * @file news_widget.cc
 * @brief News widget (renderer-only page)
 */

#include "widgets/widget.h"

#include "ui/renderers/rawdraw/news_renderer.h"

namespace widgets {

void RegisterNewsWidget() {
    WidgetDef def;
    def.name = "news";
    def.pages = {
        {ui::RawDrawPageId::News, "news", "News", nullptr,
         []() -> rawdraw::PageRenderer* { return new rawdraw::NewsRenderer(); }},
    };
    Register(std::move(def));
}

}  // namespace widgets

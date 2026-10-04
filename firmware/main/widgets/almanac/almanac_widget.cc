/**
 * @file almanac_widget.cc
 * @brief Almanac widget (renderer-only page)
 */

#include "widgets/widget.h"

#include "widgets/almanac/almanac_renderer.h"

namespace widgets {

void RegisterAlmanacWidget() {
    WidgetDef def;
    def.name = "almanac";
    def.pages = {
        {ui::RawDrawPageId::Almanac, "almanac", "Almanac", nullptr,
         []() -> rawdraw::PageRenderer* { return new rawdraw::AlmanacRenderer(); }},
    };
    Register(std::move(def));
}

}  // namespace widgets

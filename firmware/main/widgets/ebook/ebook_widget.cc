/**
 * @file ebook_widget.cc
 * @brief Ebook reader widget (renderer-only page)
 */

#include "widgets/widget.h"

#include "ui/renderers/rawdraw/ebook_renderer.h"

namespace widgets {

void RegisterEbookWidget() {
    WidgetDef def;
    def.name = "ebook";
    def.pages = {
        {ui::RawDrawPageId::Ebook, "ebook", "Ebook", nullptr,
         []() -> rawdraw::PageRenderer* { return new rawdraw::EbookRenderer(); }},
    };
    Register(std::move(def));
}

}  // namespace widgets

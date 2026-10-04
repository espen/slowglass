/**
 * @file calendar_widget.cc
 * @brief Monthly calendar widget (renderer-only page)
 */

#include "widgets/widget.h"

#include "widgets/calendar/calendar_renderer.h"

namespace widgets {

void RegisterCalendarWidget() {
    WidgetDef def;
    def.name = "calendar";
    def.pages = {
        {ui::RawDrawPageId::Calendar, "calendar", "Calendar", nullptr,
         []() -> rawdraw::PageRenderer* { return new rawdraw::CalendarRenderer(); }},
    };
    Register(std::move(def));
}

}  // namespace widgets

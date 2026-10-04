/**
 * @file widgets_init.cc
 * @brief The one place that lists which widgets this firmware contains
 *
 * Explicit calls instead of static-initializer self-registration: with
 * static libraries the linker drops translation units nothing references,
 * so a self-registering widget could silently vanish from the build. One
 * greppable line per widget is honest and doubles as the compile-time
 * selection point.
 *
 * Each CONFIG_WIDGET_* option (menuconfig -> Xiaozhi Assistant -> Widgets)
 * must gate BOTH the call here and the widget's sources in CMakeLists.txt.
 */

#include "widgets/widget_registry.h"

#include <sdkconfig.h>

namespace widgets {

// Each widget's <name>_widget.cc defines its Register*Widget() function.
void RegisterWeatherWidget();
void RegisterMakePlansWidget();
void RegisterNewsWidget();
void RegisterCalendarWidget();
void RegisterAlmanacWidget();
void RegisterLifeBarWidget();
void RegisterYearProgressWidget();
void RegisterEbookWidget();

void RegisterAll() {
#if CONFIG_WIDGET_WEATHER
    RegisterWeatherWidget();
#endif
#if CONFIG_WIDGET_MAKEPLANS
    RegisterMakePlansWidget();
#endif
#if CONFIG_WIDGET_NEWS
    RegisterNewsWidget();
#endif
#if CONFIG_WIDGET_CALENDAR
    RegisterCalendarWidget();
#endif
#if CONFIG_WIDGET_ALMANAC
    RegisterAlmanacWidget();
#endif
#if CONFIG_WIDGET_LIFEBAR
    RegisterLifeBarWidget();
#endif
#if CONFIG_WIDGET_YEARPROGRESS
    RegisterYearProgressWidget();
#endif
#if CONFIG_WIDGET_EBOOK
    RegisterEbookWidget();
#endif
}

}  // namespace widgets

/**
 * @file widgets_init.cc
 * @brief The one place that lists which widgets this firmware contains
 *
 * Explicit calls instead of static-initializer self-registration: with
 * static libraries the linker drops translation units nothing references,
 * so a self-registering widget could silently vanish from the build. One
 * greppable line per widget is honest and doubles as the compile-time
 * selection point (each line gets a CONFIG_WIDGET_* guard).
 */

#include "widgets/widget_registry.h"

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
    RegisterWeatherWidget();
    RegisterMakePlansWidget();
    RegisterNewsWidget();
    RegisterCalendarWidget();
    RegisterAlmanacWidget();
    RegisterLifeBarWidget();
    RegisterYearProgressWidget();
    RegisterEbookWidget();
}

}  // namespace widgets

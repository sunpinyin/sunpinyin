#ifndef IBUS_COMPABILITY_H
#define  IBUS_COMPABILITY_H

#if defined(WITH_IBUS_1_1_0)

#include <ibus.h>

void ibus_property_set_icon (IBusProperty *prop,
                             const gchar  *icon);
void ibus_property_set_state (IBusProperty  *prop,
                              IBusPropState state);
#endif // WITH_IBUS_1_1_0

#endif // IBUS_COMPABILITY_H

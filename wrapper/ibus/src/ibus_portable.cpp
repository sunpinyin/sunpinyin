#include "ibus_portable.h"

#if defined(WITH_IBUS_1_1_0)

void
ibus_property_set_icon (IBusProperty *prop,
                        const gchar  *icon)
{
    g_assert (IBUS_IS_PROPERTY (prop));

    g_free (prop->icon);
    prop->icon = NULL;
    prop->icon = g_strdup (icon != NULL ? icon : "");
}

void
ibus_property_set_state (IBusProperty  *prop,
                         IBusPropState  state)
{
    g_assert (IBUS_IS_PROPERTY (prop));
    g_return_if_fail (state == PROP_STATE_UNCHECKED ||
                      state == PROP_STATE_CHECKED ||
                      state == PROP_STATE_INCONSISTENT);

    prop->state = state;
}

#endif // WITH_IBUS_1_1_0

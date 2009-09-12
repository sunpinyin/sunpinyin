#include <ibus.h>
#include "engine.h"

static IBusBus *bus = NULL;
static IBusFactory *factory = NULL;

static void
ibus_disconnected_cb (IBusBus  *bus,
                      gpointer  user_data)
{
    ibus_quit ();
}

static void
init ()
{
    IBusComponent *component;

    ibus_init ();

    bus = ibus_bus_new ();
    g_signal_connect (bus, "disconnected", G_CALLBACK (ibus_disconnected_cb), NULL);
	
    ibus_sunpinyin_init (bus);
    
    factory = ibus_factory_new (ibus_bus_get_connection (bus));
    ibus_factory_add_engine (factory, "sunpinyin", IBUS_TYPE_SUNPINYIN_ENGINE);

    ibus_bus_request_name (bus, "org.freedesktop.IBus.SunPinyin", 0);

    component = ibus_component_new ("org.freedesktop.IBus.SunPinyin",
                                    "SunPinyin2",
                                    "0.1.0",
                                    "LGPL/CDDL",
                                    "Kov Chai <tchaikov@gmail.com>",
                                    "http://opensolaris.org/os/project/input-method/",
                                    "",
                                    "ibus-sunpinyin");
    ibus_component_add_engine (component,
                               ibus_engine_desc_new ("sunpinyin",
                                                     "SunPinyin",
                                                     "Simplified Chinese Input Method developed by SUN",
                                                     "zh_CN",
                                                     "LGPL/CDDL",
                                                     "Kov Chai <tchaikov@gmail.com>",
                                                     SUNPINYIN_ICON_DIR"/sunpinyin_logo.xpm",
                                                     "en"));
    ibus_bus_register_component (bus, component);
    g_object_unref (component);
}

int main()
{
    init ();
    ibus_main ();
    ibus_sunpinyin_exit ();
}

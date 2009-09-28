/*
 * Copyright (c) 2009 Kov Chai <tchaikov@gmail.com>
 *
 * The contents of this file are subject to the terms of either the GNU Lesser
 * General Public License Version 2.1 only ("LGPL") or the Common Development and
 * Distribution License ("CDDL")(collectively, the "License"). You may not use this
 * file except in compliance with the License. You can obtain a copy of the CDDL at
 * http://www.opensource.org/licenses/cddl1.php and a copy of the LGPLv2.1 at
 * http://www.opensource.org/licenses/lgpl-license.php. See the License for the 
 * specific language governing permissions and limitations under the License. When
 * distributing the software, include this License Header Notice in each file and
 * include the full text of the License in the License file as well as the
 * following notice:
 * 
 * NOTICE PURSUANT TO SECTION 9 OF THE COMMON DEVELOPMENT AND DISTRIBUTION LICENSE
 * (CDDL)
 * For Covered Software in this distribution, this License shall be governed by the
 * laws of the State of California (excluding conflict-of-law provisions).
 * Any litigation relating to this License shall be subject to the jurisdiction of
 * the Federal Courts of the Northern District of California and the state courts
 * of the State of California, with venue lying in Santa Clara County, California.
 * 
 * Contributor(s):
 * 
 * If you wish your version of this file to be governed by only the CDDL or only
 * the LGPL Version 2.1, indicate your decision by adding "[Contributor]" elects to
 * include this software in this distribution under the [CDDL or LGPL Version 2.1]
 * license." If you don't indicate a single choice of license, a recipient has the
 * option to distribute your version of this file under either the CDDL or the LGPL
 * Version 2.1, or to extend the choice of license to its licensees as provided
 * above. However, if you add LGPL Version 2.1 code and therefore, elected the LGPL
 * Version 2 license, then the option applies only if the new code is made subject
 * to such option by the copyright holder. 
 */

#include <locale.h>
#include <libintl.h>
#include <unistd.h>
#include <ibus.h>
#include "config.h"
#include "engine.h"

#define N_(String) (String)
#define _(String)  gettext(String)

static IBusBus *bus = NULL;
static IBusFactory *factory = NULL;

// options
static gboolean ibus = FALSE;
static gboolean verbose = FALSE;

static const GOptionEntry entries[] = 
{
    { "ibus",    'i', 0, G_OPTION_ARG_NONE, &ibus, "component is executed by ibus", NULL },
    { "verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose, "verbose", NULL },
    { NULL },
};


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
    
    component = ibus_component_new ("org.freedesktop.IBus.SunPinyin",
                                    "Chinese SunPinyin Component",
                                    "0.1.0",
                                    "LGPL/CDDL",
                                    "Kov Chai <tchaikov@gmail.com>",
                                    "http://opensolaris.org/os/project/input-method/",
                                    "",
                                    "ibus-sunpinyin");
    ibus_component_add_engine (component,
                               ibus_engine_desc_new ("sunpinyin",
                                                     "SunPinyin",
                                                     _("Simplified Chinese Input Method developed by SUN"),
                                                     "zh_CN",
                                                     "LGPL/CDDL",
                                                     "Kov Chai <tchaikov@gmail.com>",
                                                     SUNPINYIN_ICON_DIR"/sunpinyin_logo.xpm",
                                                     "en"));

    factory = ibus_factory_new (ibus_bus_get_connection (bus));
    ibus_factory_add_engine (factory, "sunpinyin", IBUS_TYPE_SUNPINYIN_ENGINE);
    
    if (ibus) {
        ibus_bus_request_name (bus, "org.freedesktop.IBus.SunPinyin", 0);
    } else {
        ibus_bus_register_component (bus, component);
    }
    
    g_object_unref (component);
}

int main(int argc, char *argv[])
{

    setlocale (LC_ALL, "");
    bindtextdomain (GETTEXT_PACKAGE, SUNPINYIN_LOCALEDIR);
    textdomain (GETTEXT_PACKAGE);
    
    GOptionContext* context;
    context = g_option_context_new ("- ibus sunpinyin engine component");
    g_option_context_add_main_entries (context, entries, "ibus-sunpinyin");
    GError *error = NULL;
    if (!g_option_context_parse (context, &argc, &argv, &error)) {
        g_print ("Option parsing failed: %s\n", error->message);
        return -1;
    }
    
    init ();
    ibus_main ();
    ibus_sunpinyin_exit ();
}

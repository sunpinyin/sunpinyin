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

#include "sunpinyin_engine_proxy.h"
#include "sunpinyin_engine.h"
#include "sunpinyin_config.h"
#include "engine.h"


#define IBUS_SUNPINYIN_ENGINE_CLASS(klass)     \
    (G_TYPE_CHECK_CLASS_CAST ((klass), IBUS_TYPE_SUNPINYIN_ENGINE, IBusSunPinyinEngineClass))
#define IBUS_SUNPINYIN_ENGINE_GET_CLASS(obj)   \
    (G_TYPE_INSTANCE_GET_CLASS ((obj), IBUS_TYPE_SUNPINYIN_ENGINE, IBusSunPinyinEngineClass))

struct IBusSunPinyinEngineClass {
    IBusEngineClass parent;
};

typedef SunPinyinEngine IBusSunPinyinEngine;

/* functions prototype */
static void ibus_sunpinyin_engine_class_init (IBusSunPinyinEngineClass *);

IBusEngineClass *parent_class = NULL;

GType
ibus_sunpinyin_engine_get_type (void)
{
    static GType type = 0;

    static const GTypeInfo type_info = {
        sizeof (IBusSunPinyinEngineClass),
        (GBaseInitFunc)     NULL,
        (GBaseFinalizeFunc) NULL,
        (GClassInitFunc)    ibus_sunpinyin_engine_class_init,
        NULL,
        NULL,
        sizeof (IBusSunPinyinEngine),
        0,
        (GInstanceInitFunc) ibus_sunpinyin_engine_init,
    };

    if (type == 0) {
        type = g_type_register_static (IBUS_TYPE_ENGINE,
                                       "IBusSunPinyinEngine",
                                       &type_info,
                                       (GTypeFlags) 0);
    }

    return type;
}

// initialize the meta class object
void
ibus_sunpinyin_engine_class_init (IBusSunPinyinEngineClass *klass)
{
    IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS (klass);
    IBusEngineClass *engine_class = IBUS_ENGINE_CLASS (klass);
    
    parent_class = (IBusEngineClass *) g_type_class_peek_parent (klass);
    
    ibus_object_class->destroy = (IBusObjectDestroyFunc) ibus_sunpinyin_engine_destroy;
    
    engine_class->process_key_event = ibus_sunpinyin_engine_process_key_event;
    engine_class->focus_in          = ibus_sunpinyin_engine_focus_in;
    engine_class->focus_out         = ibus_sunpinyin_engine_focus_out;
    engine_class->reset             = ibus_sunpinyin_engine_reset;
    engine_class->enable            = ibus_sunpinyin_engine_enable;
    engine_class->disable           = ibus_sunpinyin_engine_disable;
    engine_class->focus_in          = ibus_sunpinyin_engine_focus_in;
    engine_class->focus_out         = ibus_sunpinyin_engine_focus_out;
    engine_class->page_up           = ibus_sunpinyin_engine_page_up;
    engine_class->page_down         = ibus_sunpinyin_engine_page_down;
    engine_class->cursor_up         = ibus_sunpinyin_engine_cursor_up;
    engine_class->cursor_down       = ibus_sunpinyin_engine_cursor_down;
    engine_class->property_activate = ibus_sunpinyin_engine_property_activate;
    engine_class->candidate_clicked = ibus_sunpinyin_engine_candidate_clicked;
}


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

#include <libintl.h>
#include <ibus.h>
#include "ibus_portable.h"
#include "sunpinyin_property.h"

#define N_(String) (String)
#define _(String)  gettext(String)

static const char *PROP_STATUS = "status";
static const char *PROP_LETTER = "full_letter";
static const char *PROP_PUNCT  = "full_punct";

PropertyInfo::PropertyInfo()
    : label(NULL), tooltip(NULL)
{}

PropertyInfo::~PropertyInfo()
{
    if (label) {
        g_object_unref(label);
    }
    if (tooltip) {
        g_object_unref(tooltip);
    }
}

SunPinyinProperty *
SunPinyinProperty::create_status_prop(IBusEngine *engine, bool state)
{
    SunPinyinProperty *prop = new SunPinyinProperty(engine, PROP_STATUS);
    prop->m_info[0].label = ibus_text_new_from_ucs4((const gunichar*) L"EN");
    prop->m_info[0].icon  = SUNPINYIN_ICON_DIR"/eng.svg";
    prop->m_info[0].tooltip = ibus_text_new_from_static_string(_("Switch to Chinese input mode"));
    prop->m_info[1].label = ibus_text_new_from_ucs4((const gunichar*) L"CN");
    prop->m_info[1].icon  = SUNPINYIN_ICON_DIR"/han.svg";
    prop->m_info[1].tooltip = ibus_text_new_from_static_string(_("Switch to English input mode"));
    prop->init(state);
    
    return prop;
}

SunPinyinProperty *
SunPinyinProperty::create_letter_prop(IBusEngine *engine, bool state)
{
    SunPinyinProperty *prop = new SunPinyinProperty(engine, PROP_LETTER);
    prop->m_info[0].label = ibus_text_new_from_ucs4((const gunichar*) L"Aa");
    prop->m_info[0].icon  = SUNPINYIN_ICON_DIR"/halfwidth.png";
    prop->m_info[0].tooltip = ibus_text_new_from_static_string(_("Switch to full-width letter input mode"));
    prop->m_info[1].label = ibus_text_new_from_ucs4((const gunichar*) L"Ａａ");
    prop->m_info[1].icon  = SUNPINYIN_ICON_DIR"/fullwidth.png";
    prop->m_info[1].tooltip = ibus_text_new_from_static_string(_("Switch to half-width letter input mode"));
    prop->init(state);
    return prop;
}

SunPinyinProperty *
SunPinyinProperty::create_punct_prop(IBusEngine *engine, bool state)
{
    SunPinyinProperty *prop = new SunPinyinProperty(engine, PROP_PUNCT);
    prop->m_info[0].label = ibus_text_new_from_ucs4((const gunichar*) L",.");
    prop->m_info[0].icon  = SUNPINYIN_ICON_DIR"/enpunc.png";
    prop->m_info[0].tooltip = ibus_text_new_from_static_string(_("Switch to Chinese punctuation"));
    prop->m_info[1].label = ibus_text_new_from_ucs4((const gunichar*) L"，。");
    prop->m_info[1].icon  = SUNPINYIN_ICON_DIR"/cnpunc.png";
    prop->m_info[1].tooltip = ibus_text_new_from_static_string(_("Switch to English punctuation"));
    prop->init(state);
    return prop;

}

SunPinyinProperty::SunPinyinProperty(IBusEngine *engine, const std::string& name)
    : m_engine(engine),
      m_name(name),
      m_state(false)
{
    m_prop = ibus_property_new(name.c_str(),
                               PROP_TYPE_NORMAL,
                               NULL, /* label */ NULL, /* icon */
                               NULL, /* tooltip */ TRUE, /* sensitive */
                               TRUE, /* visible */ PROP_STATE_UNCHECKED, /* state */
                               NULL);
}

SunPinyinProperty::~SunPinyinProperty()
{
    if (m_prop) {
        g_object_unref(m_prop);
        m_prop = NULL;
    }
}

bool
SunPinyinProperty::toggle(const std::string& name)
{
    if (name == m_name) {
        // called by ibus, simple toggle current state
        update(!m_state);
        return true;
    }
    return false;
}

void
SunPinyinProperty::update(bool state)
{
    if (state == m_state)
        return;
    init(state);
    ibus_engine_update_property(m_engine, m_prop);
}

void
SunPinyinProperty::init(bool state)
{
    m_state = state;
    int which = m_state ? 1 : 0;
    PropertyInfo& info = m_info[which];
    ibus_property_set_label(m_prop, info.label);
    ibus_property_set_icon(m_prop, info.icon.c_str());
    ibus_property_set_tooltip(m_prop, info.tooltip);
    ibus_property_set_visible(m_prop, TRUE);
    ibus_property_set_state(m_prop, state ? PROP_STATE_CHECKED : PROP_STATE_UNCHECKED);
}

bool
SunPinyinProperty::state() const
{
    return m_state;
}

IBusProperty *
SunPinyinProperty::get()
{
    return m_prop;
}


SetupLauncher::SetupLauncher()
    : m_name("setup")
{
    
    m_prop = ibus_property_new(m_name.c_str(),
                               PROP_TYPE_NORMAL,
                               NULL, /* label */ NULL, /* icon */
                               NULL, /* tooltip */ TRUE, /* sensitive */
                               TRUE, /* visible */ PROP_STATE_UNCHECKED, /* state */
                               NULL);
    m_info.label   = ibus_text_new_from_ucs4((const gunichar*) L"Perference");
    m_info.tooltip = ibus_text_new_from_static_string(_("Configure SunPinyin"));
    m_info.icon    = SUNPINYIN_ICON_DIR"/setup.svg";
    init();
}

void
SetupLauncher::launch(const std::string& name)
{
    if (m_name != name) return;
    
    GError *error = NULL;
    gchar *argv[2] = { NULL, };
	gchar *path;
	const char* libexecdir;
    
	libexecdir = g_getenv("LIBEXECDIR");
	if (libexecdir == NULL)
	    libexecdir = LIBEXECDIR;
    
	path = g_build_filename(libexecdir, "ibus-setup-sunpinyin", NULL);
	argv[0] = path;
	argv[1] = NULL;
    gboolean success;
    success = g_spawn_async (NULL, argv, NULL,
                             G_SPAWN_SEARCH_PATH,
                             NULL, NULL, NULL, &error);
    if (!success) {
        g_message("Unabled to launch \"%s\"", path);
    }
    g_free(path);
}

IBusProperty *
SetupLauncher::get()
{
    return m_prop;
}

void
SetupLauncher::init()
{
    ibus_property_set_label(m_prop, m_info.label);
    ibus_property_set_icon (m_prop, m_info.icon.c_str());
    ibus_property_set_tooltip(m_prop, m_info.tooltip);
    ibus_property_set_visible (m_prop, TRUE);
}

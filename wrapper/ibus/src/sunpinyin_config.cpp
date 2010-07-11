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

#include <cassert>
#include <sunpinyin.h>
#include "sunpinyin_config_keys.h"
#include "sunpinyin_config.h"

using namespace std;

struct ConfigItem
{
    string section;
    string name;
    ConfigItem(const string& key)
    {
        section = "engine/SunPinyin/";
        size_t pos = key.rfind('/');
        if (pos != key.npos) {
            section += key.substr(0, pos);
            pos += 1;
        } else {
            pos = 0;
        }
        name = key.substr(pos);
    }
};

static vector<string> get_strings_from_gvalue(GValue* value);

IBusConfig* SunPinyinConfig::m_config;

SunPinyinConfig::SunPinyinConfig()
{
    m_scheme_names["QuanPin"]    = CSunpinyinSessionFactory::QUANPIN;
    m_scheme_names["ShuangPin"]  = CSunpinyinSessionFactory::SHUANGPIN;
}

SunPinyinConfig::~SunPinyinConfig()
{}

void
SunPinyinConfig::set_config(IBusConfig* config)
{
    assert(config);
    m_config = config;
    listen_on_changed();
}

bool
SunPinyinConfig::get(const char* key, bool val)
{
    assert(m_config != NULL);
    
    GValue v = {0};
    gboolean got;
    ConfigItem item(key);
    got = ibus_config_get_value(m_config, item.section.c_str(), item.name.c_str(), &v);
    bool result = val;
    if (got && G_VALUE_TYPE(&v) == G_TYPE_BOOLEAN) {
        result = (g_value_get_boolean(&v) == TRUE);
    }
    return result;
}

void
SunPinyinConfig::set(const char* key, bool value)
{
    assert(m_config != NULL);
    
    GValue v = {0};
    g_value_init(&v, G_TYPE_BOOLEAN);
    g_value_set_boolean(&v, value?TRUE:FALSE);
    ConfigItem item(key);
    ibus_config_set_value(m_config, item.section.c_str(), item.name.c_str(), &v);
}

std::string
SunPinyinConfig::get(const char* key, const std::string& val)
{
    assert(m_config != NULL);
    
    GValue v = {0};
    gboolean got;
    ConfigItem item(key);
    got = ibus_config_get_value(m_config, item.section.c_str(), item.name.c_str(), &v);
    string result = val;
    if (got && G_VALUE_TYPE(&v) == G_TYPE_STRING) {
        result = string(g_value_get_string(&v));
    }
    return result;
}

void
SunPinyinConfig::set(const char* key, const std::string& val)
{
    assert(m_config != NULL);
    
    GValue v = {0};
    g_value_init(&v, G_TYPE_STRING);
    g_value_set_string(&v, val.c_str());
    ConfigItem item(key);
    ibus_config_set_value(m_config, item.section.c_str(), item.name.c_str(), &v);
}

int
SunPinyinConfig::get(const char* key, int val)
{
    assert(m_config != NULL);
    
    GValue v = {0};
    gboolean got;
    ConfigItem item(key);
    got = ibus_config_get_value(m_config, item.section.c_str(), item.name.c_str(), &v);
    int result = val;
    if (got && G_VALUE_TYPE(&v) == G_TYPE_INT) {
        result =  g_value_get_int(&v);
    }
    return result;
}

void 
SunPinyinConfig::set(const char* key, int value)
{
    assert(m_config != NULL);
    
    GValue v = {0};
    g_value_init(&v, G_TYPE_INT);
    g_value_set_int(&v, value);
    ConfigItem item(key);
    ibus_config_set_value(m_config, item.section.c_str(), item.name.c_str(), &v);
}

std::vector<std::string>
SunPinyinConfig::get(const char *key, const std::vector<std::string>& val)
{
    assert(m_config != NULL);
    
    GValue v = {0};
    gboolean got;
    ConfigItem item(key);
    got = ibus_config_get_value(m_config, item.section.c_str(), item.name.c_str(), &v);
    vector<string> result(val);
    if (got && G_VALUE_TYPE(&v) == G_TYPE_VALUE_ARRAY) {
        result =  get_strings_from_gvalue(&v);
    }
    return result;
}

CSunpinyinSessionFactory::EPyScheme
SunPinyinConfig::get_py_scheme(CSunpinyinSessionFactory::EPyScheme scheme)
{
    string default_name =
        get_scheme_name(scheme);
    string name = get(PINYIN_SCHEME, default_name);
    return get_scheme(name);
}

bool
SunPinyinConfig::is_initial_mode_cn()
{
    string init_mode("Chinese");
    init_mode = get(CONFIG_GENERAL_INITIAL_MODE, init_mode);
    return (init_mode == "Chinese");
}

bool
SunPinyinConfig::is_initial_punct_full()
{
    string init_punct("Full");
    init_punct = get(CONFIG_GENERAL_INITIAL_PUNCT, init_punct);
    return (init_punct == "Full");
}

bool
SunPinyinConfig::is_initial_letter_full()
{
    string init_letter("Half");
    init_letter = get(CONFIG_GENERAL_INITIAL_LETTER, init_letter);
    return (init_letter == "Full");
}

void
SunPinyinConfig::listen_on_changed()
{
    assert(m_config != NULL);
    g_signal_connect(m_config, "value-changed",
                     G_CALLBACK(on_config_value_changed), 0);
}

std::string
SunPinyinConfig::get_scheme_name(CSunpinyinSessionFactory::EPyScheme scheme)
{
    string val = "ShuangPin";
    for (SchemeNames::iterator it = m_scheme_names.begin();
         it != m_scheme_names.end(); ++it) {
        if (it->second == scheme)
            val = it->first;
    }
    return val;
}

CSunpinyinSessionFactory::EPyScheme
SunPinyinConfig::get_scheme(const std::string& name)
{
    CSunpinyinSessionFactory::EPyScheme val = CSunpinyinSessionFactory::SHUANGPIN;
    SchemeNames::iterator it = m_scheme_names.find(name);
    if (it != m_scheme_names.end()) {
        val = it->second;
    }
    return val;
}

vector<string>
get_strings_from_gvalue(GValue *value)
{
    GValueArray *array = (GValueArray *)g_value_get_boxed(value);
    assert(array != NULL);
    vector<string> strings;
    for (unsigned i = 0; i < array->n_values; ++i) {
        GValue *element = &(array->values[i]);
        assert (G_VALUE_TYPE(element) == G_TYPE_STRING && "only array of string is supported");
        strings.push_back(g_value_get_string(element));
    }
    return strings;
}

static COptionEvent
g_value_to_event(const gchar *section, const gchar *name, GValue *value)
{
    string event_name;
    
    if (strlen(section) == 0) {
        event_name = name;
    } else {
        event_name = string(section) + "/" + string(name);
    }
    
    switch (G_VALUE_TYPE(value)) {
    case G_TYPE_INT:
        return COptionEvent(event_name, g_value_get_int(value));
    case G_TYPE_STRING:
        return COptionEvent(event_name, g_value_get_string(value));
    case G_TYPE_BOOLEAN:
        return COptionEvent(event_name,
                            g_value_get_boolean(value)?true:false);
    default:
        // G_TYPE_VALUE_ARRAY() not a constant
        if (G_TYPE_VALUE_ARRAY == G_VALUE_TYPE(value))
            return COptionEvent(event_name, get_strings_from_gvalue(value));
        assert(false && "unknown gvalue");
        return COptionEvent(event_name, 0);
    }   
}

void
SunPinyinConfig::on_config_value_changed(IBusConfig *config,
                                         const gchar *section,
                                         const gchar *name,
                                         GValue *value,
                                         SunPinyinConfig* thiz)
{
    static const char* prefix = "engine/SunPinyin/";
    if (!strstr(section, prefix))
        return;
    const char *sub_section = section + strlen(prefix);
    COptionEvent event = g_value_to_event(sub_section, name, value);
    AOptionEventBus::instance().publishEvent(event);
}

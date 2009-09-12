#include <ibus.h>
#include "ibus_compability.h"
#include "sunpinyin_property.h"

static const char *PROP_STATUS = "status";
static const char *PROP_LETTER = "full_letter";
static const char *PROP_PUNCT  = "full_punct";

SunPinyinProperty *
SunPinyinProperty::create_status_prop(IBusEngine *engine, bool state)
{
    SunPinyinProperty *prop = new SunPinyinProperty(engine, PROP_STATUS);
    prop->m_info[0].label = ibus_text_new_from_ucs4((const gunichar*) L"EN");
    prop->m_info[0].icon  = SUNPINYIN_ICON_DIR"/eng.png";
    prop->m_info[1].label = ibus_text_new_from_ucs4((const gunichar*) L"CN");
    prop->m_info[1].icon  = SUNPINYIN_ICON_DIR"/han.png";
    prop->init(state);

    return prop;
}

SunPinyinProperty *
SunPinyinProperty::create_letter_prop(IBusEngine *engine, bool state)
{
    SunPinyinProperty *prop = new SunPinyinProperty(engine, PROP_LETTER);
    prop->m_info[0].label = ibus_text_new_from_ucs4((const gunichar*) L"f");
    prop->m_info[0].icon  = SUNPINYIN_ICON_DIR"/halfwidth.png";
    prop->m_info[1].label = ibus_text_new_from_ucs4((const gunichar*) L"F");
    prop->m_info[1].icon  = SUNPINYIN_ICON_DIR"/fullwidth.png";
    prop->init(state);
    return prop;
}

SunPinyinProperty *
SunPinyinProperty::create_punct_prop(IBusEngine *engine, bool state)
{
    SunPinyinProperty *prop = new SunPinyinProperty(engine, PROP_PUNCT);
    prop->m_info[0].label = ibus_text_new_from_ucs4((const gunichar*) L"p");
    prop->m_info[0].icon  = SUNPINYIN_ICON_DIR"/enpunc.png";
    prop->m_info[1].label = ibus_text_new_from_ucs4((const gunichar*) L"P");
    prop->m_info[1].icon  = SUNPINYIN_ICON_DIR"/cnpunc.png";

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
    for (int i = 0; i < 2; ++i) {
        PropertyInfo& info = m_info[i];
        if (info.label) {
            g_object_unref(info.label);
            info.label = NULL;
        }
        if (info.tooltip) {
            g_object_unref(info.tooltip);
            info.tooltip = NULL;
        }
    }
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
    ibus_property_set_icon (m_prop, info.icon.c_str());
    ibus_property_set_visible (m_prop, TRUE);
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

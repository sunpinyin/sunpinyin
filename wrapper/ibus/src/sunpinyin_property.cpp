#include <ibus.h>

#include "sunpinyin_property.h"

SunPinyinProperty::SunPinyinProperty(IBusEngine *engine,
                                     const std::string& name,
                                     const std::wstring& label_0,
                                     const std::wstring& label_1,
                                     IBusPropState state)
    : m_engine(engine),
      m_name(name)
{
    m_labels[0] = ibus_text_new_from_ucs4((const gunichar*) label_0.c_str());
    m_labels[1] = ibus_text_new_from_ucs4((const gunichar*)label_1.c_str());
    m_prop = ibus_property_new(name.c_str(),
                               PROP_TYPE_NORMAL,
                               NULL, /* label */ NULL, /* icon */
                               NULL, /* tooltip */ TRUE, /* sensitive */
                               FALSE, /* visible */ state, /* state */
                               NULL);
    ibus_property_set_label(m_prop, m_labels[0]);
}

SunPinyinProperty::~SunPinyinProperty()
{
    for (int i = 0; i < 2; ++i) {
        if (m_labels[i]) {
            g_object_unref(m_labels[i]);
            m_labels[i] = NULL;
        }
    }
    if (m_prop) {
        g_object_unref(m_prop);
        m_prop = NULL;
    }
}

bool
SunPinyinProperty::update(const std::string& name, bool state)
{
    if (name == m_name) {
        update(state);
        return true;
    }
    return false;
}

void
SunPinyinProperty::update(bool state)
{
    IBusText *label = m_labels[state?1:0];
    ibus_property_set_label(m_prop, label);
    ibus_engine_update_property(m_engine, m_prop);
}

IBusProperty *
SunPinyinProperty::get()
{
    return m_prop;
}

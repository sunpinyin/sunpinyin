#include <cassert>
#include "sunpinyin_engine.h"
#include "sunpinyin_config.h"

    
namespace SunPinyinConfig 
{
    static IBusConfig *s_config = NULL;

    struct ConfigInfo
    {
        const char *section;
        const char *name;
        union 
        {
            bool b_default;
            unsigned u_default;
            const char* s_default;
        } u;
    
        ConfigInfo(const char* s, const char* n, bool v)
            : section(s), name(n)
        {
            u.b_default = v ? TRUE : FALSE;
        }
    
        ConfigInfo(const char* s, const char* n, unsigned v)
            : section(s), name(n)
        {
            u.u_default = v;
        }
    
        ConfigInfo(const char* s, const char* n, const char* v)
            : section(s), name(n)
        {
            u.s_default = v;
        }
    };
    template<typename ItemType>
    ConfigItem<ItemType>::ConfigItem(const ConfigInfo& info)
        : m_info(info)
    {}
    
    template<> 
    bool ConfigItem<bool>::get()
    {
        assert(s_config != NULL);
        
        GValue v = {0};
        gboolean got;
        got = ibus_config_get_value(s_config, m_info.section, m_info.name, &v);
        bool result = m_info.u.b_default;
        if (got) {
            result = (g_value_get_boolean(&v) == TRUE);
        }
        return result;
    }
    
    template<>
    std::string ConfigItem<std::string>::get()
    {
        assert(s_config != NULL);
        
        GValue v = {0};
        gboolean got;
        got = ibus_config_get_value(s_config, m_info.section, m_info.name, &v);
        std::string result = m_info.u.s_default;
        if (got) {
            result = std::string(g_value_get_string(&v));
        }
        return result;
    }

    template<>
    unsigned ConfigItem<unsigned>::get()
    {
        assert(s_config != NULL);
        
        GValue v = {0};
        gboolean got;
        got = ibus_config_get_value(s_config, m_info.section, m_info.name, &v);
        unsigned result = m_info.u.u_default;
        if (got) {
            result =  g_value_get_uint(&v);
        }
        return result;
    }
    
    // template<typename ItemType> 
    // void ConfigItem<ItemType>::set(ItemType value)
    // {
    //     assert(false && "not supported type");
    // }
    
    template<> 
    void ConfigItem<bool>::set(bool value)
    {
        assert(s_config != NULL);
        
        GValue v = {0};
        g_value_init(&v, G_TYPE_BOOLEAN);
        g_value_set_boolean(&v, value?TRUE:FALSE);
        ibus_config_set_value(s_config, m_info.section, m_info.name, &v);
    }
    
    template<> 
    void ConfigItem<std::string>::set(std::string value)
    {
        assert(s_config != NULL);
        
        GValue v = {0};
        g_value_init(&v, G_TYPE_STRING);
        g_value_set_string(&v, value.c_str());
        ibus_config_set_value(s_config, m_info.section, m_info.name, &v);
    }
    
    template<> 
    void ConfigItem<unsigned>::set(unsigned value)
    {
        assert(s_config != NULL);
        
        GValue v = {0};
        g_value_init(&v, G_TYPE_UINT);
        g_value_set_uint(&v, value);
        ibus_config_set_value(s_config, m_info.section, m_info.name, &v);
    }

    void
    set_config(IBusConfig *config)
    {
        s_config = config;
    }
    
    // TODO: we may want to put the section/names into an individual .in file.
    //       so we can share the generated .h and .py files in src/ and setup/
    //       respectively
    template<typename ItemType>
    bool
    ConfigItem<ItemType>::on_changed(const gchar *section,
                                     const gchar *name,
                                     GValue *value)
    {
        if (strcmp(m_info.section, section) == 0 &&
        strcmp(m_info.name, name) == 0) {
            switch (g_value_get_gtype (value)) {
            case G_TYPE_BOOLEAN:
                do_change(g_value_get_boolean(value) == TRUE);
                break;
            case G_TYPE_UINT:
                do_change(g_value_get_uint(value));
                break;
            case G_TYPE_STRING:
                do_change(g_value_get_string(value));
                break;
            default:
                // unsupported type
                break;
            }
            return true;
        }
        return false;
    }
    
    static const ConfigInfo config_scheme = ConfigInfo("engine/SunPinyin/Pinyin",
                                                       "Scheme",
                                                       "QuanPin");
    
    PinyinScheme::PinyinScheme()
        : ConfigItem<std::string>(config_scheme)
    {
        m_scheme_names["QuanPin"]   = CSunpinyinSessionFactory::QUANPIN;
        m_scheme_names["ShuangPin"] = CSunpinyinSessionFactory::SHUANGPIN;
    }
    
    const std::string
    PinyinScheme::get_name(CSunpinyinSessionFactory::EPyScheme scheme)
    {
        for (SchemeNames::iterator it = m_scheme_names.begin();
             it != m_scheme_names.end(); ++it) {
            if (it->second == scheme)
                return it->first;
        }
        return "QuanPin";
    }
    
    CSunpinyinSessionFactory::EPyScheme
    PinyinScheme::get_scheme(const std::string& name)
    {
        SchemeNames::iterator it = m_scheme_names.find(name);
        if (it != m_scheme_names.end()) {
            return it->second;
        }
        return CSunpinyinSessionFactory::QUANPIN;
    }
    
    CSunpinyinSessionFactory::EPyScheme
    PinyinScheme::get()
    {
        std::string conf = ConfigItem<std::string>::get();
        return get_scheme(conf);
    }

    void
    PinyinScheme::set(CSunpinyinSessionFactory::EPyScheme scheme)
    {
        std::string name = get_name(scheme);
        ConfigItem<std::string>::set(name);
    }

    SunPinyinEngine * s_engine = NULL;
    
    void
    PinyinScheme::do_change(const std::string& str)
    {
        assert(s_engine != NULL);
        s_engine->update_history_power(get_scheme(str));
    }
    
    static const ConfigInfo config_candidate_window_size = ConfigInfo("engine/SunPinyin/View",
                                                                      "CandidateWindowSize",
                                                                      10U);
    
    CandidateWindowSize::CandidateWindowSize()
        : ConfigItem<unsigned>(config_candidate_window_size)
    {}
    
    void
    CandidateWindowSize::do_change(unsigned size)
    {
        assert(s_engine != NULL);
        s_engine->update_candidate_window_size(size);
    }
    
    static const ConfigInfo config_mode_key_shift = ConfigInfo("engine/SunPinyin/ModeSwitch",
                                                               "Shift",
                                                               true);
    
    ModeKeys::Shift::Shift()
        : ConfigItem<bool>(config_mode_key_shift)
    {}
    
    void
    ModeKeys::Shift::do_change(bool enabled)
    {
        assert(s_engine != NULL);
        s_engine->update_mode_key_shift(enabled);
    }
    
    static const ConfigInfo config_mode_key_shift_control = ConfigInfo("engine/SunPinyin/ModeSwitch",
                                                                       "ShiftControl",
                                                                       false);
    
    ModeKeys::ShiftControl::ShiftControl()
        : ConfigItem<bool>(config_mode_key_shift_control)
    {}
    
    void
    ModeKeys::ShiftControl::do_change(bool enabled)
    {
        assert(s_engine != NULL);
        s_engine->update_mode_key_shift_control(enabled);
    }
    
    static const ConfigInfo config_page_key_minus = ConfigInfo("engine/SunPinyin/PageFlip",
                                                               "MinusEqual",
                                                               false);
    
    PageKeys::MinusEquals::MinusEquals()
        : ConfigItem<bool>(config_page_key_minus)
    {}
    
    void
    PageKeys::MinusEquals::do_change(bool enabled)
    {}
    
    static const ConfigInfo config_page_key_comma = ConfigInfo("engine/SunPinyin/PageFlip",
                                                               "CommaPeriod",
                                                           false);
    
    PageKeys::CommaPeriod::CommaPeriod()
        : ConfigItem<bool>(config_page_key_comma)
    {}
    
    void
    PageKeys::CommaPeriod::do_change(bool enabled)
    {
        assert(s_engine != NULL);
        s_engine->update_page_key_comma(enabled);
    }
    
    static const ConfigInfo config_history_power = ConfigInfo("engine/SunPinyin/Pinyin",
                                                          "HistoryPower",
                                                          3U);

    HistoryPower::HistoryPower()
        : ConfigItem<unsigned>(config_history_power)
    {}

    void
    HistoryPower::do_change(unsigned power)
    {
        assert(s_engine != NULL);
        s_engine->update_history_power(power);
    }
    
    Options::Options()
    {}
    
    void 
    Options::listen_on_changed(SunPinyinEngine *engine)
    {
        assert(s_config != NULL);
        g_signal_connect(s_config, "value-changed",
                         G_CALLBACK(this->on_config_value_changed), this);
        s_engine = engine;
    }
    
    void
    Options::on_config_value_changed(IBusConfig *config,
                                     const gchar *section,
                                     const gchar *name,
                                     GValue *value,
                                     gpointer user_data)
    {
        Options *thiz = reinterpret_cast<Options*>(user_data);
        for (ConfigItems::iterator item = thiz->m_configs.begin();
             item != thiz->m_configs.end(); ++item) {
            assert(*item);
            if ((*item)->on_changed(section, name, value))
                break;
        }
    }
} // namespace SunPinyinConfig





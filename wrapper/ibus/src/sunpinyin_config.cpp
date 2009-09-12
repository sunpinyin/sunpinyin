#include <cassert>

#include "sunpinyin_config.h"

struct ConfigItem
{
    const char *section;
    const char *name;
    union 
    {
        bool b_default;
        unsigned u_default;
        const char* s_default;
    } u;
    
    ConfigItem(const char* s, const char* n, bool v)
        : section(s), name(n)
    {
        u.b_default = v ? TRUE : FALSE;
    }
    
    ConfigItem(const char* s, const char* n, unsigned v)
        : section(s), name(n)
    {
        u.u_default = v;
    }
    
    ConfigItem(const char* s, const char* n, const char* v)
        : section(s), name(n)
    {
        u.s_default = v;
    }
};


static IBusConfig *s_config = NULL;

void
SunPinyinConfig::set_config(IBusConfig *config)
{
    s_config = config;
}

template<typename ItemType> 
ItemType get_config(const ConfigItem& item)
{
    class IncompleteType;
    return IncompleteType();
}

template<> 
bool get_config<bool>(const ConfigItem& item)
{
    assert(s_config != NULL);

    GValue v = {0};
    gboolean got;
    got = ibus_config_get_value(s_config, item.section, item.name, &v);
    bool result = item.u.b_default;
    if (got) {
        result = (g_value_get_boolean(&v) == TRUE);
    }
    return result;
}

template<>
std::string get_config<std::string>(const ConfigItem& item)
{
    assert(s_config != NULL);

    GValue v = {0};
    gboolean got;
    got = ibus_config_get_value(s_config, item.section, item.name, &v);
    std::string result = item.u.s_default;
    if (got) {
        result = std::string(g_value_get_string(&v));
    }
    return result;

}

template<>
unsigned get_config<unsigned>(const ConfigItem& item)
{
    assert(s_config != NULL);

    GValue v = {0};
    gboolean got;
    got = ibus_config_get_value(s_config, item.section, item.name, &v);
    unsigned result = item.u.u_default;
    if (got) {
        result =  g_value_get_uint(&v);
    }
    return result;
}

template<typename ItemType> 
void set_config(const ConfigItem& item, ItemType value)
{
    assert(false && "not supported type");
}

template<> 
void set_config(const ConfigItem& item, bool value)
{
    assert(s_config != NULL);

    GValue v = {0};
    g_value_init(&v, G_TYPE_BOOLEAN);
    g_value_set_boolean(&v, value?TRUE:FALSE);
    ibus_config_set_value(s_config, item.section, item.name, &v);
}

template<> 
void set_config(const ConfigItem& item, const std::string& value)
{
    assert(s_config != NULL);

    GValue v = {0};
    g_value_init(&v, G_TYPE_STRING);
    g_value_set_string(&v, value.c_str());
    ibus_config_set_value(s_config, item.section, item.name, &v);
}

template<> 
void set_config(const ConfigItem& item, unsigned value)
{
    assert(s_config != NULL);

    GValue v = {0};
    g_value_init(&v, G_TYPE_UINT);
    g_value_set_uint(&v, value);
    ibus_config_set_value(s_config, item.section, item.name, &v);
}



// TODO: we may want to put the section/names into an individual .in file.
//       so we can share the generated .h and .py files in src/ and setup/
//       respectively


static const ConfigItem config_candidate_window_size = ConfigItem("engine/SunPinyin/View",
                                                                  "CandidateWindowSize",
                                                                  10U);

unsigned
SunPinyinConfig::CandidateWindowSize::get()
{
    return get_config<unsigned>(config_candidate_window_size);
}

void
SunPinyinConfig::CandidateWindowSize::set(unsigned size)
{
    set_config(config_candidate_window_size, size);
}

static const ConfigItem config_mode_key_shift = ConfigItem("engine/SunPinyin/ModeSwitch",
                                                           "Shift",
                                                           true);

bool
SunPinyinConfig::ModeKeys::use_shift()
{
    return get_config<bool>(config_mode_key_shift);
}

void SunPinyinConfig::ModeKeys::set_shift(bool enabled)
{
    return set_config(config_mode_key_shift, enabled);
}

static const ConfigItem config_mode_key_shift_control = ConfigItem("engine/SunPinyin/ModeSwitch",
                                                                   "ShiftControl",
                                                                   false);

bool
SunPinyinConfig::ModeKeys::use_shift_control()
{
    return get_config<bool>(config_mode_key_shift_control);
}

void
SunPinyinConfig::ModeKeys::set_shift_control(bool enabled)
{
    return set_config(config_mode_key_shift_control, enabled);
}

static const ConfigItem config_page_key_minus = ConfigItem("engine/SunPinyin/PageFlip",
                                                           "MinusEqual",
                                                           false);
bool
SunPinyinConfig::PageKeys::use_minus()
{
    return get_config<bool>(config_page_key_minus);
}

void
SunPinyinConfig::PageKeys::set_minus(bool enabled)
{
    set_config(config_page_key_minus, enabled);
}


static const ConfigItem config_page_key_comma = ConfigItem("engine/SunPinyin/PageFlip",
                                                           "CommaPeriod",
                                                           false);

bool
SunPinyinConfig::PageKeys::use_comma()
{
    return get_config<bool>(config_page_key_comma);
}

void
SunPinyinConfig::PageKeys::set_comma(bool enabled)
{
    set_config(config_page_key_comma, enabled);
}


static const ConfigItem config_scheme = ConfigItem("engine/SunPinyin/Pinyin",
                                                   "Scheme",
                                                   "QuanPin");

CSunpinyinSessionFactory::EPyScheme
SunPinyinConfig::PinyinScheme::get()
{
    std::string conf = get_config<std::string>(config_scheme);
    
    CSunpinyinSessionFactory::EPyScheme scheme;
    if (conf == "QuanPin") {
        scheme = CSunpinyinSessionFactory::QUANPIN;
    } else if (conf == "ShuangPin") {
        scheme = CSunpinyinSessionFactory::SHUANGPIN;
    } else {
        scheme = CSunpinyinSessionFactory::QUANPIN;
    }
    return scheme;
}

void
SunPinyinConfig::PinyinScheme::set(CSunpinyinSessionFactory::EPyScheme scheme)
{
    std::string scheme_str;
    
    switch (scheme) {
    case CSunpinyinSessionFactory::QUANPIN:
        scheme_str = "QuanPin";
        break;
    case CSunpinyinSessionFactory::SHUANGPIN:
        scheme_str = "ShuangPin";
        break;
    default:
        scheme_str = "QuanPin";
    }
    set_config(config_scheme, scheme_str);
}


static const ConfigItem config_history_power = ConfigItem("engine/SunPinyin/Pinyin",
                                                          "HistoryPower",
                                                          3U);
unsigned
SunPinyinConfig::HistoryPower::get()
{
    return get_config<unsigned>(config_history_power);
}

void
SunPinyinConfig::HistoryPower::set(unsigned power)
{
    set_config(config_history_power, power);
}

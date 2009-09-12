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

// TODO: may want to put the section/names into an individual .in file.
//       so we can share the generated .h and .py files in src/ and setup/
//       respectively
static const ConfigItem config_page_key_minus = ConfigItem("engine/SunPinyin/Keyboard",
                                                           "MinusEqual",
                                                           false);

static const ConfigItem config_page_key_comma = ConfigItem("engine/SunPinyin/Keyboard",
                                                           "CommaPeriod",
                                                           false);

static const ConfigItem config_scheme = ConfigItem("engine/SunPinyin/Pinyin",
                                                   "Scheme",
                                                   "QuanPin");

static const ConfigItem config_history_power = ConfigItem("engine/SunPinyin/Pinyin",
                                                          "HistoryPower",
                                                          3U);

static const ConfigItem config_candidate_window_size = ConfigItem("engine/SunPinyin/View",
                                                                  "CandidateWindowSize",
                                                                  10U);
static IBusConfig *s_config = NULL;

void
ibus_sunpinyin_set_config(IBusConfig *config)
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

SunPinyinConfig::SunPinyinConfig()
{}

SunPinyinConfig::~SunPinyinConfig()
{}

unsigned
SunPinyinConfig::get_candidate_window_size()
{
    return get_config<unsigned>(config_candidate_window_size);
}

void
SunPinyinConfig::set_candidate_window_size(unsigned size)
{
    ::set_config(config_candidate_window_size, size);
}

bool
SunPinyinConfig::use_page_keys_minus()
{
    return get_config<bool>(config_page_key_minus);
}

void
SunPinyinConfig::set_page_keys_minus(bool enabled)
{
    ::set_config(config_page_key_minus, enabled);
}


bool
SunPinyinConfig::use_page_keys_comma()
{
    return get_config<bool>(config_page_key_comma);
}

void
SunPinyinConfig::set_page_keys_comma(bool enabled)
{
    ::set_config(config_page_key_comma, enabled);
}

CSunpinyinSessionFactory::EPyScheme
SunPinyinConfig::get_pinyin_scheme()
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
SunPinyinConfig::set_pinyin_scheme(CSunpinyinSessionFactory::EPyScheme scheme)
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
    ::set_config(config_scheme, scheme_str);
}

unsigned
SunPinyinConfig::get_history_power()
{
    return get_config<unsigned>(config_history_power);
}

void
SunPinyinConfig::set_history_power(unsigned power)
{
    ::set_config(config_history_power, power);
}

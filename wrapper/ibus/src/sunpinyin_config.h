#ifndef SUNPINYIN_CONFIG_H
#define SUNPINYIN_CONFIG_H

#include <map>
#include <string>
#include <ibus.h>
#include <imi_options.h>

class SunPinyinEngine;

class SunPinyinConfig
{
    typedef std::map<std::string,
        CSunpinyinSessionFactory::EPyScheme> SchemeNames;
    typedef std::map<std::string,
        EShuangpinType> TypeNames;

    SchemeNames        m_scheme_names;
    TypeNames          m_type_names;
    SunPinyinEngine   *m_engine;

    static IBusConfig *m_config;
    
public:
    SunPinyinConfig();
    
    bool get(const char* key, bool val);
    void set(const char* key, bool val);
    
    unsigned get(const char* key, unsigned val);
    void set(const char* key, unsigned val);
    
    std::string get(const char *key, const std::string& default_val);
    void set(const char* key, const std::string& val);

    CSunpinyinSessionFactory::EPyScheme get_py_scheme(CSunpinyinSessionFactory::EPyScheme);
    void set_py_scheme(CSunpinyinSessionFactory::EPyScheme);

    EShuangpinType get_shuangpin_type(EShuangpinType);
    void set_shuangpin_type(EShuangpinType);
    
    /**
     * gets called in ibus_sunpinyin_init() so that SunPinyinEngine can read 
     * configuration when it starts up
     */
    static void set_config(IBusConfig *);

    void listen_on_changed(SunPinyinEngine *engine);
    
    static void on_config_value_changed(IBusConfig *config,
                                        const gchar *section,
                                        const gchar *name,
                                        GValue *value,
                                        gpointer user_data);

private:
    std::string get_scheme_name(CSunpinyinSessionFactory::EPyScheme scheme);
    CSunpinyinSessionFactory::EPyScheme get_scheme(const std::string& name);
    std::string get_type_name(EShuangpinType);
    EShuangpinType get_type(const std::string& name);
};

#endif // SUNPINYIN_CONFIG_H

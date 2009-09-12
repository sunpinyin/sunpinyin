#ifndef SUNPINYIN_CONFIG_H
#define SUNPINYIN_CONFIG_H

#include <string>
#include <ibus.h>
#include <imi_options.h>

class SunPinyinConfig
{
public:
    SunPinyinConfig();
    ~SunPinyinConfig();
    
    // TODO: ELanguage, EInputStyle
    //       these features are not implemented yet.
    CSunpinyinSessionFactory::EPyScheme get_pinyin_scheme();
    void set_pinyin_scheme(CSunpinyinSessionFactory::EPyScheme);
    
    unsigned get_history_power();
    void set_history_power(unsigned power);

    bool use_page_keys_minus();
    void set_page_keys_minus(bool enabled);
    
    bool use_page_keys_comma();
    void set_page_keys_comma(bool enabled);
    
    unsigned get_candidate_window_size();
    void set_candidate_window_size(unsigned size);
};

/**
 * gets called in ibus_sunpinyin_init() so that SunPinyinEngine can read 
 * configuration when it starts up
 */
void ibus_sunpinyin_set_config(IBusConfig *);

#endif // SUNPINYIN_CONFIG_H

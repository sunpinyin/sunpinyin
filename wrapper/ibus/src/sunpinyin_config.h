#ifndef SUNPINYIN_CONFIG_H
#define SUNPINYIN_CONFIG_H

#include <string>
#include <ibus.h>
#include <imi_options.h>

namespace SunPinyinConfig
{
    // TODO: ELanguage, EInputStyle
    //       these features are not implemented yet.
    struct PinyinScheme
    {
        static CSunpinyinSessionFactory::EPyScheme get();
        static void set(CSunpinyinSessionFactory::EPyScheme);
    };

    struct HistoryPower
    {
        static unsigned get();
        static void set(unsigned power);
    };

    struct ModeKeys
    {
        static bool use_shift();
        static void set_shift(bool enabled);
        static bool use_shift_control();
        static void set_shift_control(bool enabled);
    };
    
    struct PageKeys
    {
        static bool use_minus();
        static void set_minus(bool enabled);
        static bool use_comma();
        static void set_comma(bool enabled);
    };

    // TODO: switch full/half symbol
    //       switch full/half punct
    struct CandidateWindowSize
    {
        static unsigned get();
        static void set(unsigned size);
    };
    /**
     * gets called in ibus_sunpinyin_init() so that SunPinyinEngine can read 
     * configuration when it starts up
     */
    void set_config(IBusConfig *);
}


#endif // SUNPINYIN_CONFIG_H

#ifndef SUNPINYIN_CONFIG_H
#define SUNPINYIN_CONFIG_H

#include <vector>
#include <string>
#include <cassert>
#include <ibus.h>
#include <imi_options.h>

class SunPinyinEngine;

namespace SunPinyinConfig
{
    struct ConfigInfo;
    
    // TODO: ELanguage, EInputStyle
    //       these features are not implemented yet.
    struct Configurable
    {
        virtual bool on_changed(const gchar *section,
                                const gchar *name,
                                GValue *value) = 0;
    };
        
    template <typename ItemType>
    class ConfigItem : public Configurable
    {
    public:
        ConfigItem(const ConfigInfo&);
        ItemType get();
        void set(ItemType);
        bool on_changed(const gchar *section,
                        const gchar *name,
                        GValue *value);
            protected:
        virtual void do_change(bool) {}
        virtual void do_change(unsigned) {}
        virtual void do_change(const std::string&) {}
    protected:
        const ConfigInfo &m_info;
    };
        
    template<typename ItemType> 
    ItemType ConfigItem<ItemType>::get()
    {
        class IncompleteType;
        return IncompleteType();
    }
    template<typename ItemType> 
    void ConfigItem<ItemType>::set(ItemType)
    {
        assert(false && "not supported type");
    }
    template<> bool ConfigItem<bool>::get();
    template<> std::string ConfigItem<std::string>::get();
    template<> unsigned ConfigItem<unsigned>::get();    
    template<> void ConfigItem<bool>::set(bool value);
    template<> void ConfigItem<std::string>::set(std::string value);
    template<> void ConfigItem<unsigned>::set(unsigned value);

    
    class PinyinScheme : public ConfigItem<std::string>
    {
        typedef std::map<const std::string,
                         CSunpinyinSessionFactory::EPyScheme> SchemeNames;
        
        SchemeNames m_scheme_names;
        const std::string get_name(CSunpinyinSessionFactory::EPyScheme);
        CSunpinyinSessionFactory::EPyScheme get_scheme(const std::string&);
    public:
        PinyinScheme();
        CSunpinyinSessionFactory::EPyScheme get();
        void set(CSunpinyinSessionFactory::EPyScheme);
        virtual void do_change(const std::string&);
    };
    

    class CandidateWindowSize : public ConfigItem<unsigned>
    {
    public:
        CandidateWindowSize();
        virtual void do_change(unsigned);
    };

    class HistoryPower : public ConfigItem<unsigned>
    {
    public:
        HistoryPower();
        virtual void do_change(unsigned);
    };
    
    namespace ModeKeys
    {
        class Shift : public ConfigItem<bool>
        {
        public:
            Shift();
            virtual void do_change(bool);
        };
        
        class ShiftControl : public ConfigItem<bool>
        {
        public:
            ShiftControl();
            virtual void do_change(bool);
        };
    }
    
    namespace PageKeys
    {
        class MinusEquals : public ConfigItem<bool>
        {
        public:
            MinusEquals();
            virtual void do_change(bool);
        };
        class CommaPeriod : public ConfigItem<bool>
        {
        public:
            CommaPeriod();
            virtual void do_change(bool);
        };
    }

    // TODO: switch full/half symbol
    //       switch full/half punct
    /**
     * gets called in ibus_sunpinyin_init() so that SunPinyinEngine can read 
     * configuration when it starts up
     */
    void set_config(IBusConfig *);

    class Options
    {
    public:
        CandidateWindowSize    candidate_window_size;
        PinyinScheme           pinyin_scheme;
        HistoryPower           history_power;
        ModeKeys::Shift        mode_use_shift;
        ModeKeys::ShiftControl mode_use_shift_control;
        PageKeys::MinusEquals  page_use_minus;
        PageKeys::CommaPeriod  page_use_comma;
        
    public:
        Options();
        /**
         * call engine's callback funcs when the config changes
         */
        void listen_on_changed(SunPinyinEngine *engine);
        
    private:   
        static void on_config_value_changed(IBusConfig *config,
                                            const gchar *section,
                                            const gchar *name,
                                            GValue *value,
                                            gpointer user_data);
        
    private:
        typedef std::vector<Configurable*> ConfigItems;
        ConfigItems m_configs;
    };
}


#endif // SUNPINYIN_CONFIG_H

#ifndef SUNPINYIN_PROPERTY_H
#define SUNPINYIN_PROPERTY_H

#include <string>
#include <ibus.h>

struct PropertyInfo
{
    PropertyInfo();
    ~PropertyInfo();
    
    std::string icon;
    IBusText   *label;
    IBusText   *tooltip;
};

class SunPinyinProperty
{
    IBusEngine       *m_engine;
    const std::string m_name;
    PropertyInfo      m_info[2];
    IBusProperty     *m_prop;
    bool              m_state;

public:
    static SunPinyinProperty *create_status_prop(IBusEngine * engine,
                                                 bool state = true);
    static SunPinyinProperty *create_letter_prop(IBusEngine * engine,
                                                 bool state = false);
    static SunPinyinProperty *create_punct_prop(IBusEngine * engine,
                                                bool state = false);
    ~SunPinyinProperty();
    bool toggle(const std::string& name);
    void update(bool state);
    bool state() const;
    IBusProperty *get();

private:
    void init(bool state);
    SunPinyinProperty(IBusEngine *engine, const std::string& name);
};

class SetupLauncher
{
    const std::string m_name;
    PropertyInfo      m_info;
    IBusProperty     *m_prop;
public:
    SetupLauncher();
    void launch(const std::string& name);
    IBusProperty *get();
    void init();
};


#endif // SUNPINYIN_PROPERTY_H

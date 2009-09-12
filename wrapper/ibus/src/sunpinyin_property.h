#ifndef SUNPINYIN_PROPERTY_H
#define SUNPINYIN_PROPERTY_H

#include <string>
#include <ibus.h>

class SunPinyinProperty
{
    struct PropertyInfo
    {
        std::string icon;
        IBusText   *label;
        IBusText   *tooltip;
    };
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
    bool update(const std::string& name, bool state);
    void update(bool state);
    const std::string& name() const;
    bool state() const;
    IBusProperty *get();

private:
    void init(bool state);
    SunPinyinProperty(IBusEngine *engine, const std::string& name);
};

#endif // SUNPINYIN_PROPERTY_H

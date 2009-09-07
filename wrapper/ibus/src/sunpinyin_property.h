#ifndef SUNPINYIN_PROPERTY_H
#define SUNPINYIN_PROPERTY_H

#include <string>
#include <ibus.h>

class SunPinyinProperty
{
    IBusEngine *m_engine;
    IBusText *m_labels[2];
    IBusProperty *m_prop;
    const std::string m_name;
    
public:
    SunPinyinProperty(IBusEngine *engine,
                      const std::string& name,
                      const std::wstring& label_1,
                      const std::wstring& label_2,
                      IBusPropState state = PROP_STATE_UNCHECKED);
    ~SunPinyinProperty();
    bool update(const std::string& name, bool state);
    void update(bool state);
    IBusProperty *get();
};

#endif // SUNPINYIN_PROPERTY_H

#ifndef SUNPINYIN_ENGINE_H
#define SUNPINYIN_ENGINE_H

#include <string>
#include <ibus.h>

class SunPinyinLookupTable;
class SunPinyinProperty;
class ICandidateList;
class IPreeditString;
class CIBusWinHandler;
class CIMIView;
class CHotkeyProfile;

class SunPinyinEngine : public IBusEngine
{
public:
    SunPinyinEngine();
    ~SunPinyinEngine();
    /** virtual functions defined by IBusEngineClass */
    /* @{ */
    void init ();
    void destroy ();
    gboolean process_key_event (guint key_val, guint key_code, guint modifiers);
    void focus_in ();
    void focus_out ();
    void reset ();
    void enable ();
    void disable ();
    void page_up ();
    void page_down ();
    void cursor_up ();
    void cursor_down ();
    void commit_string (const std::wstring&);
    void property_activate (const std::string& name, unsigned state = PROP_STATE_UNCHECKED);
    void candidate_clicked (guint index);
    /* @} */

public:
    /* helpers for IBus framework */
    void set_parent_class(IBusEngineClass *);
    
public:
    /* helpers for @ref CIBusWinHandler */
    void update_candidates(const ICandidateList&);
    void update_preedit_string(const IPreeditString&);
    void update_status_property(bool);
    void update_punct_property(bool);
    void update_letter_property(bool);

private:
    void update_lookup_table();
    bool is_valid() const;
    void init_hotkey_profile();
    
private:
    SunPinyinProperty *m_status_prop;
    SunPinyinProperty *m_letter_prop;
    SunPinyinProperty *m_punct_prop;
    SunPinyinProperty *m_shuangpin_prop;
    
    IBusPropList         *m_prop_list;
    SunPinyinLookupTable *m_lookup_table;
    IBusEngineClass      *m_parent; // the meta class if its base class

    CIBusWinHandler *m_wh;
    CIMIView        *m_pv;
    CHotkeyProfile  *m_hotkey_profile;
};

#endif // SUNPINYIN_ENGINE_H

#ifndef ENGINE_IMPL_H
#define ENGINE_IMPL_H

#include <imi_option_event.h>
#include "sunpinyin_config.h"

class SunPinyinLookupTable;
class SunPinyinProperty;
class SetupLauncher;
class ICandidateList;
class IPreeditString;
class CIBusWinHandler;
class CIMIView;
class CHotkeyProfile;

class EngineImpl : public IConfigurable
{
public:
    EngineImpl(IBusEngine *);
    ~EngineImpl();

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
    
    /* helpers for @ref CIBusWinHandler */
    /* @{ */
    void update_candidates(const ICandidateList&);
    void update_preedit_string(const IPreeditString&);
    void update_status_property(bool);
    void update_punct_property(bool);
    void update_letter_property(bool);
    /* @} */

    /* for implemented class */
    bool is_valid() const;

    virtual bool onConfigChanged(const COptionEvent& event);
    
private:
    void update_config();
    void update_history_power();
    void update_cand_window_size();
    void update_mode_key_shift();
    void update_mode_key_control();
    void update_page_key_minus();
    void update_page_key_comma();
    void update_charset_level();
    
    void update_lookup_table();
    
private:
    SunPinyinProperty *m_status_prop;
    SunPinyinProperty *m_letter_prop;
    SunPinyinProperty *m_punct_prop;
    SetupLauncher     *m_setup_prop;

    IBusEngine           *m_ibus_engine;
    IBusPropList         *m_prop_list;
    SunPinyinLookupTable *m_lookup_table;

    CIBusWinHandler *m_wh;
    CIMIView        *m_pv;
    CHotkeyProfile  *m_hotkey_profile;
    
    SunPinyinConfig *m_config;
};

#endif // ENGINE_IMPL_H

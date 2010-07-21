/*
 * Copyright (c) 2009 Kov Chai <tchaikov@gmail.com>
 *
 * The contents of this file are subject to the terms of either the GNU Lesser
 * General Public License Version 2.1 only ("LGPL") or the Common Development and
 * Distribution License ("CDDL")(collectively, the "License"). You may not use this
 * file except in compliance with the License. You can obtain a copy of the CDDL at
 * http://www.opensource.org/licenses/cddl1.php and a copy of the LGPLv2.1 at
 * http://www.opensource.org/licenses/lgpl-license.php. See the License for the 
 * specific language governing permissions and limitations under the License. When
 * distributing the software, include this License Header Notice in each file and
 * include the full text of the License in the License file as well as the
 * following notice:
 * 
 * NOTICE PURSUANT TO SECTION 9 OF THE COMMON DEVELOPMENT AND DISTRIBUTION LICENSE
 * (CDDL)
 * For Covered Software in this distribution, this License shall be governed by the
 * laws of the State of California (excluding conflict-of-law provisions).
 * Any litigation relating to this License shall be subject to the jurisdiction of
 * the Federal Courts of the Northern District of California and the state courts
 * of the State of California, with venue lying in Santa Clara County, California.
 * 
 * Contributor(s):
 * 
 * If you wish your version of this file to be governed by only the CDDL or only
 * the LGPL Version 2.1, indicate your decision by adding "[Contributor]" elects to
 * include this software in this distribution under the [CDDL or LGPL Version 2.1]
 * license." If you don't indicate a single choice of license, a recipient has the
 * option to distribute your version of this file under either the CDDL or the LGPL
 * Version 2.1, or to extend the choice of license to its licensees as provided
 * above. However, if you add LGPL Version 2.1 code and therefore, elected the LGPL
 * Version 2 license, then the option applies only if the new code is made subject
 * to such option by the copyright holder. 
 */

#ifndef SUNPINYIN_ENGINE_H
#define SUNPINYIN_ENGINE_H

#include <string>
#include <ibus.h>
#include <ime-core/imi_option_event.h>
#include "sunpinyin_config.h"
#include "sunpinyin_property.h"
#include "sunpinyin_lookup_table.h"

class ICandidateList;
class IPreeditString;
class CIBusWinHandler;
class CIMIView;
class CHotkeyProfile;

class SunPinyinEngine : public IConfigurable
{
public:
    SunPinyinEngine(IBusEngine *);
    ~SunPinyinEngine();

    void init ();
    void destroy ();
    /* @{ */
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
    void update_mode_key();
    void update_punct_key();
    void update_page_key_minus();
    void update_page_key_comma();
    void update_page_key_bracket();
    void update_page_key(const char* conf_key, bool default_val, 
                         unsigned page_up, unsigned page_down);
    void update_cancel_with_backspace();

    void update_charset_level();
    void update_user_data_dir();
    void update_punct_mappings();
    void update_fuzzy_pinyins();
    void update_correction_pinyins();
    void update_fuzzy_segs();
    void update_shuangpin_type();

    void update_lookup_table();

private:
    ibus::Engine           m_engine;
    ibus::PropList         m_prop_list;

    SunPinyinProperty m_status_prop;
    SunPinyinProperty m_letter_prop;
    SunPinyinProperty m_punct_prop;
    SetupLauncher     m_setup_prop;

    SunPinyinLookupTable m_lookup_table;

    CIBusWinHandler *m_wh;
    CIMIView        *m_pv;
    CHotkeyProfile  *m_hotkey_profile;
    
    SunPinyinConfig  m_config;
};

#endif // SUNPINYIN_ENGINE_H

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

#include <cassert>
#include <algorithm>
#include <sstream>

#include <sunpinyin.h>

#include "sunpinyin_property.h"
#include "sunpinyin_lookup_table.h"
#include "sunpinyin_config.h"
#include "sunpinyin_config_keys.h"
#include "imi_ibus_win.h"
#include "ibus_portable.h"
#include "sunpinyin_engine.h"

using namespace std;

extern ibus::Config config;

SunPinyinEngine::SunPinyinEngine(IBusEngine *engine)
    : m_engine(engine),
      m_status_prop(SunPinyinProperty::create_status_prop(engine)),
      m_letter_prop(SunPinyinProperty::create_letter_prop(engine)),
      m_punct_prop(SunPinyinProperty::create_punct_prop(engine)),
      m_wh(NULL),
      m_pv(NULL),
      m_hotkey_profile(NULL)
{
    CSunpinyinSessionFactory& factory = CSunpinyinSessionFactory::getFactory();

    CSunpinyinSessionFactory::EPyScheme pinyin_scheme =
        m_config.get_py_scheme(CSunpinyinSessionFactory::QUANPIN);
    factory.setPinyinScheme(pinyin_scheme);
    if (pinyin_scheme == CSunpinyinSessionFactory::QUANPIN) {
        update_fuzzy_pinyins();
        update_correction_pinyins();
        update_fuzzy_segs();
    } else {
        update_shuangpin_type();
    }
    update_user_data_dir();
    update_punct_mappings();
    
    factory.setCandiWindowSize(m_config.get(CONFIG_GENERAL_PAGE_SIZE, 10));
    
    m_pv = factory.createSession();
    if (!m_pv)
        return;

    m_hotkey_profile = new CHotkeyProfile();
    m_pv->setHotkeyProfile(m_hotkey_profile);
    
    m_wh = new CIBusWinHandler(this);
    m_pv->attachWinHandler(m_wh);

    m_prop_list = ibus_prop_list_new();
    
    ibus_prop_list_append(m_prop_list, m_status_prop);
    ibus_prop_list_append(m_prop_list, m_letter_prop);
    ibus_prop_list_append(m_prop_list, m_punct_prop);
    ibus_prop_list_append(m_prop_list, m_setup_prop);
    
    update_config();
}

SunPinyinEngine::~SunPinyinEngine()
{
    if (m_pv) {
        CSunpinyinSessionFactory& factory =
            CSunpinyinSessionFactory::getFactory();
        factory.destroySession(m_pv);
    }
    
    delete m_wh;
    delete m_hotkey_profile;
}

static CKeyEvent
translate_key(guint key_val, guint /*key_code*/, guint modifiers)
{
    // XXX: may need to move this logic into CKeyEvent
    if (key_val > 0x20 && key_val < 0x7f // isprint(key_val) && !isspace(key_val)
	&& !(modifiers & IM_CTRL_MASK)) {
        // we only care about key_val here
        return CKeyEvent(key_val, key_val, modifiers);
    } else {
        // what matters is key_code, but ibus sents me key_code as key_val
        return CKeyEvent(key_val, 0, modifiers);
    }
}

gboolean
SunPinyinEngine::process_key_event (guint key_val,
                                    guint key_code,
                                    guint modifiers)
{
    CKeyEvent key = translate_key(key_val, key_code, modifiers);
    
    if ( !m_pv->getStatusAttrValue(CIBusWinHandler::STATUS_ID_CN) ) {
        // we are in English input mode
        if ( !m_hotkey_profile->isModeSwitchKey(key) ) {
            m_hotkey_profile->rememberLastKey(key);
            return FALSE;
        }
    } else if ( m_hotkey_profile->isModeSwitchKey(key) ) {
        m_pv->onKeyEvent(CKeyEvent(IM_VK_ENTER, 0, 0));
        m_pv->setStatusAttrValue(CIMIWinHandler::STATUS_ID_CN, false);
        return TRUE;
    }
    
    return m_pv->onKeyEvent(key);
}

void
SunPinyinEngine::focus_in ()
{
    ibus_engine_register_properties(m_engine, m_prop_list);
    m_pv->updateWindows(CIMIView::PREEDIT_MASK | CIMIView::CANDIDATE_MASK);
}

void
SunPinyinEngine::focus_out ()
{
    reset();
}

void
SunPinyinEngine::reset ()
{
    m_pv->updateWindows(m_pv->clearIC());
}

void
SunPinyinEngine::enable ()
{
    bool is_cn = m_config.is_initial_mode_cn();
    m_status_prop.update(is_cn);
    m_pv->setStatusAttrValue(CIMIWinHandler::STATUS_ID_CN, is_cn);

    bool is_letter_full = m_config.is_initial_letter_full();
    m_letter_prop.update(is_letter_full);
    m_pv->setStatusAttrValue(CIMIWinHandler::STATUS_ID_FULLSYMBOL, is_letter_full);

    bool is_punct_full = m_config.is_initial_punct_full();
    m_punct_prop.update(is_punct_full);
    m_pv->setStatusAttrValue(CIMIWinHandler::STATUS_ID_FULLPUNC, is_punct_full);
}

void
SunPinyinEngine::disable ()
{
}

void
SunPinyinEngine::page_up ()
{
    m_pv->onCandidatePageRequest(-1, true /* relative */);
}

void
SunPinyinEngine::page_down ()
{
    m_pv->onCandidatePageRequest(1, true /* relative */);
}

void
SunPinyinEngine::property_activate (const std::string& property, unsigned /*state*/)
{
    if (m_status_prop.toggle(property)) {
        m_pv->setStatusAttrValue(CIMIWinHandler::STATUS_ID_CN, 
                                 m_status_prop.state());
    } else if (m_letter_prop.toggle(property)) {
        m_pv->setStatusAttrValue(CIMIWinHandler::STATUS_ID_FULLSYMBOL, 
                                 m_letter_prop.state());
    } else if (m_punct_prop.toggle(property)) {
        m_pv->setStatusAttrValue(CIMIWinHandler::STATUS_ID_FULLPUNC, 
                                 m_punct_prop.state());
    } else {
        // try to launch the setup UI
        m_setup_prop.launch(property);
    }
}

void
SunPinyinEngine::candidate_clicked (guint index)
{
    m_pv->onCandidateSelectRequest(index);
}

void
SunPinyinEngine::cursor_up ()
{
    if (m_lookup_table.cursor_up()) {
        update_lookup_table();
    }
}

void
SunPinyinEngine::cursor_down ()
{
    if (m_lookup_table.cursor_down()) {
        update_lookup_table();
    }
}

bool
SunPinyinEngine::onConfigChanged(const COptionEvent& event)
{
    if (event.name == CONFIG_GENERAL_MEMORY_POWER) {
        update_history_power();
    } else if (event.name == CONFIG_GENERAL_PAGE_SIZE) {
        update_cand_window_size();
    } else if (event.name == CONFIG_GENERAL_CHARSET_LEVEL) {
        update_charset_level();
    } else if (event.name == CONFIG_KEYBOARD_MODE_SWITCH) {
        update_mode_key();
    } else if (event.name == CONFIG_KEYBOARD_PUNCT_SWITCH) {
        update_punct_key();
    } else if (event.name == CONFIG_KEYBOARD_PAGE_COMMA) {
        update_page_key_comma();
    } else if (event.name == CONFIG_KEYBOARD_PAGE_MINUS) {
        update_page_key_minus();
    } else if (event.name == CONFIG_KEYBOARD_PAGE_BRACKET) {
        update_page_key_bracket();
    } else if (event.name == CONFIG_QUANPIN_FUZZYSEGS_ENABLED) {
        update_fuzzy_segs();
    } else if (event.name == CONFIG_KEYBOARD_CANCEL_BACKSPACE) {
        update_cancel_with_backspace();
    }
    
    return false;
}

void
SunPinyinEngine::update_config()
{
    update_history_power();
    update_cand_window_size();
    update_charset_level();
    update_page_key_minus();
    update_page_key_comma();
    update_page_key_bracket();
    update_mode_key();
    update_punct_key();
    update_cancel_with_backspace();
    update_punct_mappings();
    // update_quanpin_config();
    // update_shuangpin_config();
}

void
SunPinyinEngine::commit_string (const std::wstring& str)
{
    IBusText *text;
    text = ibus_text_new_from_ucs4((const gunichar*) str.c_str());
    ibus_engine_commit_text(m_engine, text);
}

void
SunPinyinEngine::update_candidates(const ICandidateList& cl)
{
    if (m_lookup_table.update_candidates(cl) > 0)
        update_lookup_table();
    else
        ibus_engine_hide_lookup_table (m_engine);
}

void
SunPinyinEngine::update_lookup_table()
{
    ibus_engine_update_lookup_table(m_engine, m_lookup_table, TRUE);
}

bool
SunPinyinEngine::is_valid() const
{
    return m_pv != NULL;
}

int decorate_preedit_char(IBusText *text, const IPreeditString& preedit,
                          int begin, int end, unsigned type,
                          unsigned long fg_color)
{
    int i = begin;
    while (i < end && (preedit.charTypeAt(i) & type) == type)
        ++i;
    end = i;
    int len = end - begin;
    if (len > 0) {
        ibus_text_append_attribute(text, IBUS_ATTR_TYPE_FOREGROUND,
                                   fg_color, begin, end);
    }
    return len;
}


enum {ORANGE = 0xE76F00, GRAY_BLUE = 0x35556B, WHITE = 0xFFFFFF, BLACK = 0x000000};

void decorate_preedit_string_using_char_type(IBusText *text, const IPreeditString& preedit)
{
    for (int i = 0, size = preedit.charTypeSize(); i < size; ) {
        int len = 0;
        if ((len = decorate_preedit_char(text, preedit, i, size, preedit.PINYIN_CHAR, 
                                         GRAY_BLUE)) > 0) {
            i += len;
        } else if ((len = decorate_preedit_char(text, preedit, i, size,
                                                preedit.BOUNDARY,
                                                GRAY_BLUE)) > 0) {
            i += len;
        } else {
            ++i;
        }
    }
}

    
void decorate_preedit_string_using_caret_pos(IBusText *text, const IPreeditString& preedit, int caret)
{
    if (caret < preedit.size()) {
        // add underline, otherwise gtk app won't have the same color scheme with that of x11 apps
        ibus_text_append_attribute(text, IBUS_ATTR_TYPE_UNDERLINE, IBUS_ATTR_UNDERLINE_SINGLE,
                                   caret, preedit.size());
        ibus_text_append_attribute(text, IBUS_ATTR_TYPE_FOREGROUND, WHITE,
                                   caret, preedit.size());
        ibus_text_append_attribute(text, IBUS_ATTR_TYPE_BACKGROUND, GRAY_BLUE,
                                   caret, preedit.size());
    }
}

void
SunPinyinEngine::update_preedit_string(const IPreeditString& preedit)
{
    const int len = preedit.size();
    if (len > 0) {
        IBusText *text = ibus_text_new_from_ucs4((const gunichar*) preedit.string());
        
        
        const int caret = preedit.caret();
        if (caret < len) {
            decorate_preedit_string_using_caret_pos(text, preedit, caret);
        } else {
            decorate_preedit_string_using_char_type(text, preedit);
        }
        
        ibus_engine_update_preedit_text(m_engine, text, caret, TRUE);
    } else {
        ibus_engine_hide_preedit_text(m_engine);
    }
}

void
SunPinyinEngine::update_status_property(bool cn)
{
    m_status_prop.update(cn);
}

void
SunPinyinEngine::update_punct_property(bool full)
{
    m_punct_prop.update(full);
}

void
SunPinyinEngine::update_letter_property(bool full)
{
    m_letter_prop.update(full);
}

void
SunPinyinEngine::update_history_power()
{
    unsigned power = m_config.get(CONFIG_GENERAL_MEMORY_POWER, 3);
    CIMIContext* ic = m_pv->getIC();
    assert(ic);
    ic->setHistoryPower(power);
}

void
SunPinyinEngine::update_charset_level()
{
    unsigned charset = m_config.get(CONFIG_GENERAL_CHARSET_LEVEL, GBK);
    CIMIContext* ic = m_pv->getIC();
    assert(ic);
    charset &= 3;               // charset can only be 0,1,2 or 3
    ic->setCharsetLevel(charset);
}

void
SunPinyinEngine::update_cand_window_size()
{
    unsigned size = m_config.get(CONFIG_GENERAL_PAGE_SIZE, 10);
    m_pv->setCandiWindowSize(size);
}

void
SunPinyinEngine::update_mode_key()
{
    string mode_switch("Shift");
    mode_switch = m_config.get(CONFIG_KEYBOARD_MODE_SWITCH, mode_switch);

    CKeyEvent shift_l  (IM_VK_SHIFT_L, 0, IM_SHIFT_MASK|IM_RELEASE_MASK);
    CKeyEvent shift_r  (IM_VK_SHIFT_R, 0, IM_SHIFT_MASK|IM_RELEASE_MASK);
    CKeyEvent control_l(IM_VK_CONTROL_L, 0, IM_CTRL_MASK|IM_RELEASE_MASK);
    CKeyEvent control_r(IM_VK_CONTROL_R, 0, IM_CTRL_MASK|IM_RELEASE_MASK);
    
    if (mode_switch == "Shift") {
        m_hotkey_profile->removeModeSwitchKey(control_l);
        m_hotkey_profile->removeModeSwitchKey(control_r);
        m_hotkey_profile->addModeSwitchKey(shift_l);
        m_hotkey_profile->addModeSwitchKey(shift_r);
    } else if (mode_switch == "Control") {
        m_hotkey_profile->removeModeSwitchKey(shift_l);
        m_hotkey_profile->removeModeSwitchKey(shift_r);
        m_hotkey_profile->addModeSwitchKey(control_l);
        m_hotkey_profile->addModeSwitchKey(control_r);
    }
}

void
SunPinyinEngine::update_punct_key()
{
    string punct_switch("ControlComma");
    punct_switch = m_config.get(CONFIG_KEYBOARD_PUNCT_SWITCH, punct_switch);
    if (punct_switch == "ControlComma") {
        m_hotkey_profile->setPunctSwitchKey(CKeyEvent(IM_VK_COMMA, 0, IM_CTRL_MASK));
    } else if (punct_switch == "ControlPeriod") {
        m_hotkey_profile->setPunctSwitchKey(CKeyEvent(IM_VK_PERIOD, 0, IM_CTRL_MASK));
    }
}

void
SunPinyinEngine::update_page_key_minus()
{
    update_page_key(CONFIG_KEYBOARD_PAGE_MINUS, false,
                    IM_VK_MINUS, IM_VK_EQUALS);
}

void
SunPinyinEngine::update_page_key_comma()
{
    update_page_key(CONFIG_KEYBOARD_PAGE_COMMA, false,
                    IM_VK_COMMA, IM_VK_PERIOD);
}

void
SunPinyinEngine::update_page_key_bracket()
{
    update_page_key(CONFIG_KEYBOARD_PAGE_BRACKET, false,
                    IM_VK_OPEN_BRACKET, IM_VK_CLOSE_BRACKET);
}

void
SunPinyinEngine::update_page_key(const char* conf_key, bool default_val, 
                            unsigned page_up, unsigned page_down)
{
    bool enabled = m_config.get(conf_key, default_val);

    if (enabled) {
        m_hotkey_profile->addPageUpKey(CKeyEvent(page_up, 0));
        m_hotkey_profile->addPageDownKey(CKeyEvent(page_down, 0));
    } else {
        m_hotkey_profile->removePageUpKey(CKeyEvent(page_up, 0));
        m_hotkey_profile->removePageDownKey(CKeyEvent(page_down, 0));
    }
}

void
SunPinyinEngine::update_cancel_with_backspace()
{
    bool enabled = m_config.get(CONFIG_KEYBOARD_CANCEL_BACKSPACE, true);
    m_pv->setCancelOnBackspace(enabled);
}

string_pairs parse_pairs(const vector<string>& strings)
{
    string_pairs pairs;
    for (vector<string>::const_iterator pair = strings.begin();
         pair != strings.end(); ++pair) {
        
        std::string::size_type found = pair->find(':');
        if (found == pair->npos || pair->length() < 3)
            continue;
        if (found == 0 && (*pair)[0] == ':')
            found = 1;
        
        pairs.push_back(make_pair(pair->substr(0, found),
                                  pair->substr(found+1)));
    }
    return pairs;
}

// the mappings in default_pairs will override the ones in user_pairs
string_pairs merge_pairs(const string_pairs& default_pairs,
                         const string_pairs& user_pairs)
{
    typedef std::map<string, int> Indexes;
    Indexes indexes;
    int index = 0;
    for (string_pairs::const_iterator it = default_pairs.begin();
         it != default_pairs.end(); ++it, ++index) {
        Indexes::iterator found = indexes.find(it->first);
        if (found == indexes.end()) {
            indexes[it->first] = index;
        } else {
            // it is a paired punct.
            indexes[it->first] = -found->second;
        }
    }
    string_pairs result(default_pairs);
    for (string_pairs::const_iterator it = user_pairs.begin();
         it != user_pairs.end(); ++it) {
        Indexes::iterator found = indexes.find(it->first);
        if (found == indexes.end()) {
            result.push_back(*it);
        } else if (found->second >= 0) {
            result[found->second] = *it;
        } else {
            // it is a paired punct,
            // but we don't support this kind of mapping yet,
            // so quietly ignore it.
        }
    }
    return result;
}

void
SunPinyinEngine::update_punct_mappings()
{
    CSimplifiedChinesePolicy& policy = ASimplifiedChinesePolicy::instance();
    if (m_config.get(PINYIN_PUNCTMAPPING_ENABLED, false)) {
        vector<string> mappings;
        mappings = m_config.get(PINYIN_PUNCTMAPPING_MAPPINGS, mappings);
        string_pairs pairs(merge_pairs(policy.getDefaultPunctMapping(),
                                       parse_pairs(mappings)));
        policy.setPunctMapping(pairs);
    }
}

void
SunPinyinEngine::update_user_data_dir()
{
    stringstream user_data_dir;
    user_data_dir << g_get_home_dir()
                  << G_DIR_SEPARATOR_S << ".sunpinyin";
    ASimplifiedChinesePolicy::instance().setUserDataDir(user_data_dir.str());
}

void
SunPinyinEngine::update_fuzzy_pinyins()
{
    bool enabled = m_config.get(QUANPIN_FUZZY_ENABLED, false);
    AQuanpinSchemePolicy::instance().setFuzzyForwarding(enabled);
    AShuangpinSchemePolicy::instance().setFuzzyForwarding(enabled);
    if (!enabled)
        return;
    vector<string> fuzzy_pinyins;
    fuzzy_pinyins = m_config.get(QUANPIN_FUZZY_PINYINS, fuzzy_pinyins);
    AQuanpinSchemePolicy::instance().setFuzzyPinyinPairs(parse_pairs(fuzzy_pinyins));
    AShuangpinSchemePolicy::instance().setFuzzyPinyinPairs(parse_pairs(fuzzy_pinyins));
}

void
SunPinyinEngine::update_correction_pinyins()
{
    bool enabled = m_config.get(QUANPIN_AUTOCORRECTION_ENABLED, false);
    AQuanpinSchemePolicy::instance().setAutoCorrecting(enabled);
    if (!enabled)
        return;
    vector<string> correction_pinyins;
    correction_pinyins = m_config.get(QUANPIN_AUTOCORRECTION_PINYINS, correction_pinyins);
    AQuanpinSchemePolicy::instance().setAutoCorrectionPairs(parse_pairs(correction_pinyins));
}

void
SunPinyinEngine::update_fuzzy_segs()
{
    bool enable_fuzzy_segs = m_config.get(CONFIG_QUANPIN_FUZZYSEGS_ENABLED, false);
    AQuanpinSchemePolicy::instance().setFuzzySegmentation(enable_fuzzy_segs);
    bool enable_inner_fuzzy = m_config.get(CONFIG_QUANPIN_INNERFUZZY_ENABLED, false);
    AQuanpinSchemePolicy::instance().setInnerFuzzySegmentation(CONFIG_QUANPIN_INNERFUZZY_ENABLED);
}

void
SunPinyinEngine::update_shuangpin_type()
{
    EShuangpinType shuangpin_type = MS2003;
    shuangpin_type = (EShuangpinType) m_config.get(SHUANGPIN_TYPE, (int) shuangpin_type);
    AShuangpinSchemePolicy::instance().setShuangpinType(shuangpin_type);
}

#include <cassert>
#include <algorithm>
#include <imi_view.h>
#include <imi_options.h>
#include <imi_keys.h>
#include <imi_option_keys.h>

#include "debug.h"
#include "sunpinyin_property.h"
#include "sunpinyin_lookup_table.h"
#include "sunpinyin_config.h"
#include "sunpinyin_config_keys.h"
#include "imi_ibus_win.h"

#include "engine_impl.h"

using namespace std;

EngineImpl::EngineImpl(IBusEngine *ibus_engine)
    : m_ibus_engine(ibus_engine), m_wh(NULL), m_pv(NULL), m_hotkey_profile(NULL)
{
    m_prop_list = ibus_prop_list_new();
    
    m_status_prop = SunPinyinProperty::create_status_prop(m_ibus_engine);
    ibus_prop_list_append(m_prop_list, m_status_prop->get());
    
    m_letter_prop = SunPinyinProperty::create_letter_prop(m_ibus_engine);
    ibus_prop_list_append(m_prop_list, m_letter_prop->get());
    
    m_punct_prop = SunPinyinProperty::create_punct_prop(m_ibus_engine);
    ibus_prop_list_append(m_prop_list, m_punct_prop->get());

    m_setup_prop = new SetupLauncher();
    ibus_prop_list_append(m_prop_list, m_setup_prop->get());
    
    m_lookup_table = new SunPinyinLookupTable();
    
    CSunpinyinSessionFactory& factory = CSunpinyinSessionFactory::getFactory();
    
    m_config = new SunPinyinConfig();
    addRef();
    
    factory.setPinyinScheme(m_config->get_py_scheme(CSunpinyinSessionFactory::QUANPIN));
    factory.setCandiWindowSize(m_config->get(CONFIG_GENERAL_PAGE_SIZE, 10));
    
    m_pv = factory.createSession();
    if (!m_pv)
        return;

    m_hotkey_profile = new CHotkeyProfile();
    m_pv->setHotkeyProfile(m_hotkey_profile);
    
    m_wh = new CIBusWinHandler(this);
    m_pv->attachWinHandler(m_wh);
    
    update_config();
}

EngineImpl::~EngineImpl()
{
    if (m_pv) {
        CSunpinyinSessionFactory& factory =
            CSunpinyinSessionFactory::getFactory();
        factory.destroySession(m_pv);
    }
    
    delete m_wh;
    delete m_hotkey_profile;
    
    release();
    delete m_config;
    
    delete m_lookup_table;

    if (m_prop_list) {
        g_object_unref (m_prop_list);
    }
    delete m_status_prop;
    delete m_letter_prop;
    delete m_punct_prop;
    delete m_setup_prop;
}

static CKeyEvent
translate_key(guint key_val, guint key_code, guint modifiers)
{
    // XXX: may need to move this logic into CKeyEvent
    if (isascii(key_val) && !isspace(key_val)) {
        // we only care about key_val here
        return CKeyEvent(0, key_val, modifiers);
    } else {
        // what matters is key_code, but ibus sents me key_code as key_val
        return CKeyEvent(key_val, 0, modifiers);
    }
}

gboolean
EngineImpl::process_key_event (guint key_val,
                               guint key_code,
                               guint modifiers)
{
    ibus::log << __func__ << "(): " 
              << "key_val = " << hex << key_val << ", "
              << "key_code = " << hex << key_code << ", "
              << "modifiers = " << hex << modifiers << endl;

    CKeyEvent key = translate_key(key_val, key_code, modifiers);
    
    if ( !m_pv->getStatusAttrValue(CIBusWinHandler::STATUS_ID_CN) ) {
        // we are in English input mode
        if ( !m_hotkey_profile->isModeSwitchKey(key) ) {
            m_hotkey_profile->rememberLastKey(key);
            return FALSE;
        }
    }
    return m_pv->onKeyEvent(key);
}

void
EngineImpl::focus_in ()
{
    ibus_engine_register_properties(m_ibus_engine, m_prop_list);
    m_pv->updateWindows(CIMIView::PREEDIT_MASK | CIMIView::CANDIDATE_MASK);
}

void
EngineImpl::focus_out ()
{
    reset();
}

void
EngineImpl::reset ()
{
    m_pv->updateWindows(m_pv->clearIC());
}

void
EngineImpl::enable ()
{
    focus_in();
}

void
EngineImpl::disable ()
{
    
}

void
EngineImpl::page_up ()
{
    m_pv->onCandidatePageRequest(-1, true /* relative */);
}

void
EngineImpl::page_down ()
{
    m_pv->onCandidatePageRequest(1, true /* relative */);
}

void
EngineImpl::property_activate (const std::string& property, unsigned state)
{
    if (m_status_prop->toggle(property)) {
        m_pv->setStatusAttrValue(CIMIWinHandler::STATUS_ID_CN, 
                                 m_status_prop->state());
    } else if (m_letter_prop->toggle(property)) {
        m_pv->setStatusAttrValue(CIMIWinHandler::STATUS_ID_FULLSYMBOL, 
                                 m_letter_prop->state());
    } else if (m_punct_prop->toggle(property)) {
        m_pv->setStatusAttrValue(CIMIWinHandler::STATUS_ID_FULLPUNC, 
                                 m_punct_prop->state());
    } else {
        // try to launch the setup UI
        m_setup_prop->launch(property);
    }
}

void
EngineImpl::candidate_clicked (guint index)
{
    m_pv->onCandidateSelectRequest(index);
}

void
EngineImpl::cursor_up ()
{
    if (m_lookup_table->cursor_up()) {
        update_lookup_table();
    }
}

void
EngineImpl::cursor_down ()
{
    if (m_lookup_table->cursor_down()) {
        update_lookup_table();
    }
}

bool
EngineImpl::onConfigChanged(const COptionEvent& event)
{
    ibus::log << __func__ << ": " << event.name << endl;
    if (event.name == CONFIG_GENERAL_MEMORY_POWER) {
        update_history_power();
    } else if (event.name == CONFIG_GENERAL_PAGE_SIZE) {
        update_cand_window_size();
    } else if (event.name == CONFIG_GENERAL_CHARSET_LEVEL) {
        update_charset_level();
    } else if (event.name == CONFIG_KEYBOARD_MODE_SWITCH_SHIFT) {
        update_mode_key_shift();
    } else if (event.name == CONFIG_KEYBOARD_MODE_SWITCH_CONTROL) {
        update_mode_key_control();
    } else if (event.name == CONFIG_KEYBOARD_PAGE_COMMA) {
        update_page_key_comma();
    } else if (event.name == CONFIG_KEYBOARD_PAGE_MINUS) {
        update_page_key_minus();
    } else if (event.name == CONFIG_KEYBOARD_PAGE_BRACKET) {
        update_page_key_bracket();
    }
    
    return false;
}

void
EngineImpl::update_config()
{
    update_history_power();
    update_cand_window_size();
    update_charset_level();
    update_page_key_minus();
    update_page_key_comma();
    update_page_key_bracket();
    update_mode_key_shift();
    update_mode_key_control();
    update_punct_mappings();
    // update_quanpin_config();
    // update_shuangpin_config();
}

void
EngineImpl::commit_string (const std::wstring& str)
{
    IBusText *text;
    text = ibus_text_new_from_ucs4((const gunichar*) str.c_str());
    ibus_engine_commit_text(m_ibus_engine, text);
    g_object_unref(text);
}

void
EngineImpl::update_candidates(const ICandidateList& cl)
{
    if (m_lookup_table->update_candidates(cl) > 0)
        update_lookup_table();
    else
        ibus_engine_hide_lookup_table (m_ibus_engine);
}

void
EngineImpl::update_lookup_table()
{
    ibus_engine_update_lookup_table(m_ibus_engine, m_lookup_table->get(), TRUE);
}

bool
EngineImpl::is_valid() const
{
    return m_pv != NULL;
}

int decorate_preedit_char(IBusText *text, const IPreeditString& preedit,
                          int begin, int end, unsigned type,
                          unsigned long fg_color)
{
    int i = begin;
    while (i < end && preedit.charTypeAt(i) & type)
        ++i;
    end = i;
    int len = end - begin;
    if (len > 0) {
        ibus_text_append_attribute(text, IBUS_ATTR_TYPE_FOREGROUND,
                                   fg_color, begin, end);
    }
    return len;
}

void decorate_preedit_string(IBusText *text, const IPreeditString& preedit)
{
    for (int i = 0, size = preedit.charTypeSize(); i < size; ) {
        int len = 0;
        if ((len = decorate_preedit_char(text, preedit, i, size, preedit.ILLEGAL, 
                                         0xE76F00)) > 0) {
            i += len;
        } else if ((len = decorate_preedit_char(text, preedit, i, size, 
                                                preedit.USER_CHOICE, 
                                                0x35556B)) > 0) {
            i += len;
        } else {
            ++i;
        }
    }
}

void
EngineImpl::update_preedit_string(const IPreeditString& preedit)
{
    const int len = preedit.size();
    if (len > 0) {
        IBusText *text = ibus_text_new_from_ucs4((const gunichar*) preedit.string());
        decorate_preedit_string(text, preedit);
        
        const int caret = preedit.caret();
        if (caret > 0 && caret <= len) {
            // TODO: fake a caret?
        }
        ibus_engine_update_preedit_text(m_ibus_engine, text, caret, TRUE);
        g_object_unref(text);
    } else {
        ibus_engine_hide_preedit_text(m_ibus_engine);
    }
}

void
EngineImpl::update_status_property(bool cn)
{
    m_status_prop->update(cn);
}

void
EngineImpl::update_punct_property(bool full)
{
    m_punct_prop->update(full);
}

void
EngineImpl::update_letter_property(bool full)
{
    m_letter_prop->update(full);
}

void
EngineImpl::update_history_power()
{
    unsigned power = m_config->get(CONFIG_GENERAL_MEMORY_POWER, 3);
    CIMIContext* ic = m_pv->getIC();
    assert(ic);
    ic->setHistoryPower(power);
}

void
EngineImpl::update_charset_level()
{
    unsigned charset = m_config->get(CONFIG_GENERAL_CHARSET_LEVEL, 2);
    CIMIContext* ic = m_pv->getIC();
    assert(ic);
    charset &= 3;               // charset can only be 0,1,2,3
    ic->setCharsetLevel(charset);
}

void
EngineImpl::update_cand_window_size()
{
    unsigned size = m_config->get(CONFIG_GENERAL_PAGE_SIZE, 10);
    m_pv->setCandiWindowSize(size);
}

void
EngineImpl::update_mode_key_shift()
{
    bool enabled = m_config->get(CONFIG_KEYBOARD_MODE_SWITCH_SHIFT, true);
    if (enabled) {
        m_hotkey_profile->setModeSwitchKey(
            CKeyEvent(IM_VK_SHIFT, 0, IM_SHIFT_MASK|IM_RELEASE_MASK));
    }
}

void
EngineImpl::update_mode_key_control()
{
    bool enabled = m_config->get(CONFIG_KEYBOARD_MODE_SWITCH_CONTROL, false);
    
    if (enabled) {
        m_hotkey_profile->setModeSwitchKey(
            CKeyEvent(IM_VK_SHIFT, 0, IM_ALT_MASK|IM_RELEASE_MASK));
    }
}

void
EngineImpl::update_page_key_minus()
{
    update_page_key(CONFIG_KEYBOARD_PAGE_MINUS, false,
                    IM_VK_MINUS, IM_VK_EQUALS);
}

void
EngineImpl::update_page_key_comma()
{
    update_page_key(CONFIG_KEYBOARD_PAGE_COMMA, false,
                    IM_VK_COMMA, IM_VK_PERIOD);
}

void
EngineImpl::update_page_key_bracket()
{
    update_page_key(CONFIG_KEYBOARD_PAGE_BRACKET, false,
                    IM_VK_OPEN_BRACKET, IM_VK_CLOSE_BRACKET);
}

void
EngineImpl:: update_page_key(const char* conf_key, bool default_val, 
                             unsigned page_up, unsigned page_down)
{
    bool enabled = m_config->get(conf_key, default_val);

    if (enabled) {
        m_hotkey_profile->addPageUpKey(CKeyEvent(0, page_up));
        m_hotkey_profile->addPageDownKey(CKeyEvent(0, page_down));
    } else {
        m_hotkey_profile->removePageUpKey(CKeyEvent(0, page_up));
        m_hotkey_profile->removePageDownKey(CKeyEvent(0, page_down));
    }
}

void
EngineImpl::update_punct_mappings()
{
    vector<string> mappings;
    ibus::log << __func__ << ":" << m_config->get(PINYIN_PUNCTMAPPING_ENABLED, false) << endl;
    
    if (!m_config->get(PINYIN_PUNCTMAPPING_ENABLED, false))
        return;
    
    mappings = m_config->get(PINYIN_PUNCTMAPPING_MAPPINGS, mappings);
    CPairParser parser;
    parser.parse(mappings);
    ASimplifiedChinesePolicy::instance().setPunctMapping(parser.get_pairs());
}

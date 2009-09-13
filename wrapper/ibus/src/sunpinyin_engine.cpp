#include <iostream>
#include <iomanip>
#include <fstream>

#include <cassert>
#include <algorithm>
#include <imi_view.h>
#include <imi_options.h>
#include <imi_keys.h>

#include "sunpinyin_property.h"
#include "sunpinyin_lookup_table.h"
#include "sunpinyin_config.h"
#include "imi_ibus_win.h"

#include "sunpinyin_engine.h"

using namespace std;
namespace config = SunPinyinConfig;

namespace ibus
{
    fstream log("/tmp/ibus.log", fstream::app|fstream::out);
}

SunPinyinEngine::SunPinyinEngine()
    : m_status_prop(NULL),
      m_letter_prop(NULL),
      m_punct_prop(NULL),
      m_prop_list(NULL),
      m_lookup_table(NULL),
      m_parent(NULL),
      m_wh(NULL),
      m_pv(NULL),
      m_hotkey_profile(NULL)
{}

SunPinyinEngine::~SunPinyinEngine()
{}

void
SunPinyinEngine::init ()
{
    m_prop_list = ibus_prop_list_new();
    
    m_status_prop = SunPinyinProperty::create_status_prop(this);
    ibus_prop_list_append(m_prop_list, m_status_prop->get());
    
    m_letter_prop = SunPinyinProperty::create_letter_prop(this);
    ibus_prop_list_append(m_prop_list, m_letter_prop->get());
    
    m_punct_prop = SunPinyinProperty::create_punct_prop(this);
    ibus_prop_list_append(m_prop_list, m_punct_prop->get());

    m_setup_prop = new SetupLauncher();
    ibus_prop_list_append(m_prop_list, m_setup_prop->get());
    
    m_lookup_table = new SunPinyinLookupTable();
    
    CSunpinyinSessionFactory& factory = CSunpinyinSessionFactory::getFactory();
    
    m_options = new SunPinyinConfig::Options();
    m_options->listen_on_changed(this);
    
    factory.setPinyinScheme(m_options->pinyin_scheme.get());
    factory.setCandiWindowSize(m_options->candidate_window_size.get());
    
    m_pv = factory.createSession();
    if (!m_pv)
        return;
    update_history_power(m_options->history_power.get());
    
    m_hotkey_profile = new CHotkeyProfile();
    init_hotkey_profile();
    m_pv->setHotkeyProfile(m_hotkey_profile);
    
    
    m_wh = new CIBusWinHandler(this);
    m_pv->attachWinHandler(m_wh);
}

void
SunPinyinEngine::init_hotkey_profile()
{
    update_page_key_minus(m_options->page_use_minus.get());
    update_page_key_comma(m_options->page_use_comma.get());
    update_mode_key_shift(m_options->mode_use_shift.get());
    update_mode_key_shift_control(m_options->mode_use_shift_control.get());
}


void
SunPinyinEngine::destroy ()
{
    if (m_pv) {
        CSunpinyinSessionFactory& factory = CSunpinyinSessionFactory::getFactory();
        factory.destroySession(m_pv);
        m_pv = NULL;
    }
    
    delete m_wh;
    m_wh = NULL;
    
    delete m_hotkey_profile;
    m_hotkey_profile = NULL;
    
    delete m_lookup_table;
    m_lookup_table = NULL;

    delete m_options;
    m_options = NULL;
    
    if (m_prop_list) {
        g_object_unref (m_prop_list);
        m_prop_list = NULL;
    }
    delete m_status_prop;
    m_status_prop = NULL;
    
    delete m_letter_prop;
    m_letter_prop = NULL;

    delete m_punct_prop;
    m_punct_prop = NULL;
    
    delete m_setup_prop;
    m_setup_prop = NULL;
    
    IBUS_OBJECT_CLASS (m_parent)->destroy ((IBusObject *)this);
}

void
SunPinyinEngine::set_parent_class(IBusEngineClass *klass)
{
    m_parent = klass;
}

static CKeyEvent
translate_key(guint key_val, guint key_code, guint modifiers)
{
    // XXX: may need to move this logic into CKeyEvent
    if (isascii(key_val) && !isspace(key_val)) {
        // we only care about key_val here
        return CKeyEvent(key_code, key_val, modifiers);
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
    if (!is_valid()) return FALSE;
    
    ibus::log << __func__ << "(): " 
              << "key_val = " << hex << key_val << ", "
              << "key_code = " << hex << key_code << ", "
              << "modifiers = " << hex << modifiers << endl;

    CKeyEvent key = translate_key(key_val, key_code, modifiers);
    
    if ( !m_pv->getStatusAttrValue(CIBusWinHandler::STATUS_ID_CN) ) {
        // we are in English input mode
        if ( !m_hotkey_profile->isModeSwitchKey(key) )
            return FALSE;
    }
    return m_pv->onKeyEvent(key);
}

void
SunPinyinEngine::focus_in ()
{
    if (!is_valid()) return;
    
    ibus_engine_register_properties(this, m_prop_list);
    m_pv->updateWindows(CIMIView::PREEDIT_MASK | CIMIView::CANDIDATE_MASK);
    m_parent->focus_in(this);
}

void
SunPinyinEngine::focus_out ()
{
    if (!is_valid()) return;
    
    reset();
    m_parent->focus_out(this);
}

void
SunPinyinEngine::reset ()
{
    if (!is_valid()) return;
    
    m_pv->updateWindows(m_pv->clearIC());
    m_parent->reset(this);
}

void
SunPinyinEngine::enable ()
{
    if (!is_valid()) return;
    
    focus_in();
    m_parent->enable(this);
}

void
SunPinyinEngine::disable ()
{
    if (!is_valid()) return;
    
    m_parent->disable(this);
}

void
SunPinyinEngine::page_up ()
{
    if (!is_valid()) return;
    
    m_pv->onCandidatePageRequest(-1, true /* relative */);
    m_parent->page_up(this);
}

void
SunPinyinEngine::page_down ()
{
    if (!is_valid()) return;
    
    m_pv->onCandidatePageRequest(1, true /* relative */);
    m_parent->page_down(this);
}

void
SunPinyinEngine::property_activate (const std::string& property, unsigned state)
{
    if (!is_valid()) return;
    
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
        // try to launch setup UI
        m_setup_prop->launch(property);
    }
    // TODO: shuangpin
    m_parent->property_activate(this, property.c_str(), state);
}

void
SunPinyinEngine::candidate_clicked (guint index)
{
    m_pv->onCandidateSelectRequest(index);
}

void
SunPinyinEngine::cursor_up ()
{
    if (!is_valid()) return;
    
    if (m_lookup_table->cursor_up()) {
        update_lookup_table();
        m_parent->cursor_up(this);
    }
}

void
SunPinyinEngine::cursor_down ()
{
    if (!is_valid()) return;
    
    if (m_lookup_table->cursor_down()) {
        update_lookup_table();
        m_parent->cursor_down(this);
    }
}

void
SunPinyinEngine::commit_string (const std::wstring& str)
{
    IBusText *text;
    text = ibus_text_new_from_ucs4((const gunichar*) str.c_str());
    ibus_engine_commit_text(this, text);
    g_object_unref(text);
}

void
SunPinyinEngine::update_candidates(const ICandidateList& cl)
{
    if (m_lookup_table->update_candidates(cl) > 0)
        update_lookup_table();
    else
        ibus_engine_hide_lookup_table (this);
}

void
SunPinyinEngine::update_lookup_table()
{
    ibus_engine_update_lookup_table(this, m_lookup_table->get(), TRUE);
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
SunPinyinEngine::update_preedit_string(const IPreeditString& preedit)
{
    const int len = preedit.size();
    if (len > 0) {
        IBusText *text = ibus_text_new_from_ucs4((const gunichar*) preedit.string());
        decorate_preedit_string(text, preedit);
        
        const int caret = preedit.caret();
        if (caret > 0 && caret <= len) {
            // 
        }
        ibus::log << __func__ << "(): " 
                  << "caret = " << caret << ", "
                  << "len = " << len
                  << endl;
        ibus_engine_update_preedit_text(this, text, caret, TRUE);
        g_object_unref(text);
    } else {
        ibus_engine_hide_preedit_text(this);
    }
}

void
SunPinyinEngine::update_status_property(bool cn)
{
    m_status_prop->update(cn);
}

void
SunPinyinEngine::update_punct_property(bool full)
{
    m_punct_prop->update(full);
}

void
SunPinyinEngine::update_letter_property(bool full)
{
    m_letter_prop->update(full);
}

void
SunPinyinEngine::update_pinyin_scheme(int scheme)
{
    // TODO
}

void
SunPinyinEngine::update_history_power(unsigned power)
{
    CIMIContext* ic = m_pv->getIC();
    assert(ic);
    ic->setHistoryPower(power);
}

void
SunPinyinEngine::update_candidate_window_size(unsigned size)
{
    // TODO
}

void
SunPinyinEngine::update_mode_key_shift(bool enabled)
{
    if (enabled) {
        m_hotkey_profile->setModeSwitchKey(
            CKeyEvent(IM_VK_SHIFT, 0, IM_SHIFT_MASK|IM_RELEASE_MASK));
    }
}

void
SunPinyinEngine::update_mode_key_shift_control(bool enabled)
{
    if (enabled) {
        m_hotkey_profile->setModeSwitchKey(
            CKeyEvent(IM_VK_SHIFT, 0, IM_ALT_MASK|IM_RELEASE_MASK));
    }
}

void
SunPinyinEngine::update_page_key_minus(bool enabled)
{
    if (enabled) {
        m_hotkey_profile->addPageUpKey(CKeyEvent(IM_VK_MINUS));
        m_hotkey_profile->addPageDownKey(CKeyEvent(IM_VK_EQUALS));
    }
}

void
SunPinyinEngine::update_page_key_comma(bool enabled)
{
    if (enabled) {
        m_hotkey_profile->addPageUpKey(CKeyEvent(IM_VK_COMMA));
        m_hotkey_profile->addPageDownKey(CKeyEvent(IM_VK_PERIOD));
    }
}


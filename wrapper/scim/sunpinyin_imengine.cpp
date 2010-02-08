/*
 * Copyright (c) 2007 Kov Chai <tchaikov@gmail.com>
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

#define Uses_STL_AUTOPTR
#define Uses_STL_FUNCTIONAL
#define Uses_STL_VECTOR
#define Uses_STL_IOSTREAM
#define Uses_STL_FSTREAM
#define Uses_STL_ALGORITHM
#define Uses_STL_MAP
#define Uses_STL_UTILITY
#define Uses_STL_IOMANIP
#define Uses_C_STDIO
#define Uses_SCIM_UTILITY
#define Uses_SCIM_IMENGINE
#define Uses_SCIM_ICONV
#define Uses_SCIM_CONFIG_BASE
#define Uses_SCIM_CONFIG_PATH
#define Uses_SCIM_LOOKUP_TABLE
#define Uses_SCIM_DEBUG

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <imi_options.h>
#include <imi_view.h>
#include <ic_history.h>

#include <scim.h>

#include "imi_scimwin.h"
#include "sunpinyin_utils.h"
#include "sunpinyin_keycode.h"
#include "sunpinyin_lookup_table.h"
#include "sunpinyin_imengine.h"
#include "sunpinyin_imengine_config_keys.h"
#include "sunpinyin_private.h"

#define SCIM_PROP_STATUS                  "/IMEngine/SunPinyin/Status"
#define SCIM_PROP_LETTER                  "/IMEngine/SunPinyin/Letter"
#define SCIM_PROP_PUNCT                   "/IMEngine/SunPinyin/Punct"

#ifndef SCIM_SUNPINYIN_DATADIR
    #define SCIM_SUNPINYIN_DATADIR            "/usr/share/scim/sunpinyin"
#endif

#ifndef SCIM_ICONDIR
    #define SCIM_ICONDIR                      "/usr/share/scim/icons"
#endif

#ifndef SCIM_SUNPINYIN_ICON_FILE
    #define SCIM_SUNPINYIN_ICON_FILE       (SCIM_ICONDIR "/sunpinyin_logo.xpm")
#endif

#define SCIM_FULL_LETTER_ICON              (SCIM_ICONDIR "/full-letter.png")
#define SCIM_HALF_LETTER_ICON              (SCIM_ICONDIR "/half-letter.png")
#define SCIM_FULL_PUNCT_ICON               (SCIM_ICONDIR "/full-punct.png")
#define SCIM_HALF_PUNCT_ICON               (SCIM_ICONDIR "/half-punct.png")

using namespace scim;

static IMEngineFactoryPointer _scim_pinyin_factory (0); 

static ConfigPointer _scim_config (0);

static Property _status_property   (SCIM_PROP_STATUS, "");
static Property _letter_property   (SCIM_PROP_LETTER, "");
static Property _punct_property    (SCIM_PROP_PUNCT, "");

extern "C" {
    void scim_module_init (void)
    {
        SCIM_DEBUG_IMENGINE (3) << "scim_module_init\n";
        bindtextdomain (GETTEXT_PACKAGE, SCIM_SUNPINYIN_LOCALEDIR);
        bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    }

    void scim_module_exit (void)
    {
        _scim_pinyin_factory.reset ();
        _scim_config.reset ();
    }

    uint32 scim_imengine_module_init (const ConfigPointer &config)
    {
        SCIM_DEBUG_IMENGINE (3) << "module_init\n";
        _status_property.set_tip (_("The status of the current input method. Click to change it."));
        _status_property.set_label ("英");
        
        _letter_property.set_icon (SCIM_HALF_LETTER_ICON);
        _letter_property.set_tip (_("The input mode of the letters. Click to toggle between half and full."));
        _letter_property.set_label (_("Full/Half Letter"));

        _punct_property.set_icon (SCIM_HALF_PUNCT_ICON);
        _punct_property.set_tip (_("The input mode of the puncutations. Click to toggle between half and full."));
        _punct_property.set_label (_("Full/Half Punct"));

        _scim_config = config;
        return 1;
    }

    IMEngineFactoryPointer scim_imengine_module_create_factory (uint32 engine)
    {
        SCIM_DEBUG_IMENGINE (3) << "entering scim_imengine_module_create_factory()\n";
        if (engine != 0) return IMEngineFactoryPointer (0);
        if (_scim_pinyin_factory.null ()) {
            SunPyFactory *factory = new SunPyFactory (_scim_config); 
            if (factory->valid ())
                _scim_pinyin_factory = factory;
            else
                delete factory;
        }
        return _scim_pinyin_factory;
    }
}

// implementation of SunPyFactory
SunPyFactory::SunPyFactory (const ConfigPointer &config)
    : m_config (config),
      m_valid (false)
{
    SCIM_DEBUG_IMENGINE (3) << "SunPyFactory()\n";
    set_languages ("zh_CN");
    m_name = utf8_mbstowcs ("SunPinyin");
    m_valid = init ();
    m_reload_signal_connection = m_config->signal_connect_reload (slot (this, &SunPyFactory::reload_config));
    m_hotkey_profile = new CHotkeyProfile();
}

bool
SunPyFactory::init ()
{
    bool valid = true;
    
    if (m_config) {
        valid = load_user_config();
    }
    
    // postpone the load_user_data() to the ctor of SunPyInstance
    return valid;
}

bool
SunPyFactory::load_user_config()
{
    // Load configurations.

    return true;
}

SunPyFactory::~SunPyFactory ()
{
    SCIM_DEBUG_IMENGINE (3) << "~SunPyFactory()\n";
    m_reload_signal_connection.disconnect ();
    delete m_hotkey_profile;
}

WideString
SunPyFactory::get_name () const
{
    return m_name;
}

WideString
SunPyFactory::get_authors () const
{
    return utf8_mbstowcs (
                String (_("Lei Zhang, <Phill.Zhang@sun.com>; Shuguagn Yan, <Ervin.Yan@sun.com>")));
}

WideString
SunPyFactory::get_credits () const
{
    return utf8_mbstowcs (
        String (_("Ported by Kov Chai, <tchaikov@gmail.com>")));
}

WideString
SunPyFactory::get_help () const
{
    String help =
        String (_("Hot Keys:"
                  "\n\n  Shift+Alt:\n"
                  "    Switch between English/Chinese mode."
                  "\n\n  Control+period:\n"
                  "    Switch between full/half width punctuation mode."
                  "\n\n  Shift+space:\n"
                  "    Switch between full/half width letter mode."
                  "\n\n  PageUp:\n"
                  "    Page up in lookup table."
                  "\n\n  PageDown:\n"
                  "    Page down in lookup table."
                  "\n\n  Esc:\n"
                  "    Cancel current syllable.\n"));
    return utf8_mbstowcs (help);
}

String
SunPyFactory::get_uuid () const
{
    return String ("3240fe82-585a-4f4a-96b3-0cad779c3b51");
}

String
SunPyFactory::get_icon_file () const
{
    return String (SCIM_SUNPINYIN_ICON_FILE);
}

IMEngineInstancePointer
SunPyFactory::create_instance (const String& encoding, int id)
{
    SCIM_DEBUG_IMENGINE (3) <<  "SunPyFactory::create_instance(" << id << ")\n";    
    return new SunPyInstance (this, m_hotkey_profile, encoding, id);
}

void
SunPyFactory::reload_config (const ConfigPointer &config)
{
    m_config = config;
    m_valid = init ();
}

// implementation of SunPyInstance
SunPyInstance::SunPyInstance (SunPyFactory *factory,
                              CHotkeyProfile *hotkey_profile,
                              const String& encoding,
                              int id)
    : IMEngineInstanceBase (factory, encoding, id),
      m_factory (factory),
      m_pv (0),
      m_wh (0),
      m_hotkey_profile (hotkey_profile),
      m_lookup_table (0),
      m_focused (false)
{
    SCIM_DEBUG_IMENGINE (3) << get_id() << ": SunPyInstance()\n";
    create_session(hotkey_profile);
    if (!m_pv) return;
    m_reload_signal_connection = factory->m_config->signal_connect_reload (slot (this, &SunPyInstance::reload_config));
    init_lookup_table_labels ();
}

SunPyInstance::~SunPyInstance ()
{
    SCIM_DEBUG_IMENGINE (3) <<  get_id() << ": ~SunPyInstance()\n";
    m_reload_signal_connection.disconnect ();
    destroy_session();
}

static CKeyEvent
translate_key(const KeyEvent& key)
{
    // XXX: may need to move this logic into CKeyEvent
    if (isprint(key.code) && !isspace(key.code) && !(key.mask & IM_CTRL_MASK)) {
        // we only care about key_val here
        return CKeyEvent(0, key.code, key.mask);
    } else {
        // what matters is key_code, but ibus sents me key_code as key_val
        return CKeyEvent(key.code, 0, key.mask);
    }
}

bool
SunPyInstance::process_key_event (const KeyEvent& key)
{
    SCIM_DEBUG_IMENGINE (3) <<  get_id() << ": process_key_event(" << m_focused << ", "  <<
        key.code << ", " <<
        key.mask << ", " <<
        key.layout << ")\n";
        
    if (!m_focused) return false;

    CKeyEvent ev = translate_key(key);
    
    if ( !m_pv->getStatusAttrValue(CScimWinHandler::STATUS_ID_CN) ) {
        // we are in English input mode
        if ( !m_hotkey_profile->isModeSwitchKey(ev) ) {
            m_hotkey_profile->rememberLastKey(ev);
            return false;
        }
    }
    return ( key.is_key_release() ||
             m_pv->onKeyEvent(ev) );
}

void
SunPyInstance::select_candidate (unsigned int item)
{
    m_pv->onCandidateSelectRequest(item);
//  m_pv->makeSelection(item);
}

void
SunPyInstance::update_lookup_table_page_size (unsigned int page_size)
{
    if (page_size > 0) {
        SCIM_DEBUG_IMENGINE (3) << ": update_lookup_table_page_size(" << page_size << ")\n";
        m_pv->setCandiWindowSize(page_size);
        m_lookup_table->set_page_size(page_size);
    }
}

void
SunPyInstance::lookup_table_page_up ()
{
    lookup_page_up();
    m_pv->onCandidatePageRequest(-1, true);
}

void
SunPyInstance::lookup_page_up()
{
    m_lookup_table->page_up();
    //    m_lookup_table->set_page_size(m_pv->s_CandiWindowSize);
}

void
SunPyInstance::lookup_page_down()
{
    m_lookup_table->page_down();
    //    m_lookup_table->set_page_size(m_pv->s_CandiWindowSize);
}

void
SunPyInstance::lookup_table_page_down ()
{
    // XXX, it would be great, if View class expose a page_up() method
    //    m_pv->onKeyEvent(IM_VK_PAGE_DOWN, 0, 0);
    // classic View overrides this method
    // but modern View uses the default dummy implementation
    lookup_page_down ();
    m_pv->onCandidatePageRequest(1, true);
}

void
SunPyInstance::move_preedit_caret (unsigned int /*pos*/)
{
}

void
SunPyInstance::reset ()
{
    SCIM_DEBUG_IMENGINE (3) << get_id() << ": reset()\n";
    
    m_lookup_table->clear ();

    hide_lookup_table ();
    hide_preedit_string ();
    //hide_aux_string ();
    m_pv->updateWindows(m_pv->clearIC());
    //refresh_all_properties ();
}

void
SunPyInstance::focus_in ()
{
    SCIM_DEBUG_IMENGINE(3) << get_id() << ": focus_in ()\n";
    m_focused = true;
    
    initialize_all_properties ();
    
    hide_preedit_string ();
    //hide_aux_string ();
    
    init_lookup_table_labels ();
    
    //hide_aux_string ();

    m_pv->updateWindows(CIMIView::PREEDIT_MASK | CIMIView::CANDIDATE_MASK);
}

void
SunPyInstance::focus_out ()
{
    SCIM_DEBUG_IMENGINE(3) << get_id() << ": focus_out ()\n";
    m_focused = false;
}

void
SunPyInstance::trigger_property (const String &property)
{
    SCIM_DEBUG_IMENGINE (3) << get_id() << ": trigger_property(" << property << ")\n";
    
    if (property == SCIM_PROP_STATUS) {
        const int is_CN = m_pv->getStatusAttrValue(CIMIWinHandler::STATUS_ID_CN);
        m_pv->setStatusAttrValue(CIMIWinHandler::STATUS_ID_CN, is_CN?0:1);
    } else if (property == SCIM_PROP_LETTER) {
        const int is_fullsymbol = m_pv->getStatusAttrValue(CIMIWinHandler::STATUS_ID_FULLSYMBOL);
        m_pv->setStatusAttrValue(CIMIWinHandler::STATUS_ID_FULLSYMBOL, is_fullsymbol?0:1);
    } else if (property == SCIM_PROP_PUNCT) {
        const int is_fullpunc = m_pv->getStatusAttrValue(CIMIWinHandler::STATUS_ID_FULLPUNC);
        m_pv->setStatusAttrValue(CIMIWinHandler::STATUS_ID_FULLPUNC, is_fullpunc?0:1);
    }
}


void
SunPyInstance::init_lookup_table_labels ()
{
    m_pv->setCandiWindowSize(10);
    m_lookup_table->set_page_size (10);
    m_lookup_table->show_cursor ();
}

void
SunPyInstance::initialize_all_properties ()
{
    PropertyList proplist;

    proplist.push_back (_status_property);
    proplist.push_back (_letter_property);
    proplist.push_back (_punct_property);

    register_properties (proplist);
    refresh_all_properties ();
}

void
SunPyInstance::refresh_all_properties ()
{
    SCIM_DEBUG_IMENGINE (3) << get_id() << ": refresh_all_properties()\n";
    m_wh->updateStatus(CIMIWinHandler::STATUS_ID_CN,
                       m_pv->getStatusAttrValue(CIMIWinHandler::STATUS_ID_CN));
    m_wh->updateStatus(CIMIWinHandler::STATUS_ID_FULLPUNC,
                       m_pv->getStatusAttrValue(CIMIWinHandler::STATUS_ID_FULLPUNC));
    m_wh->updateStatus(CIMIWinHandler::STATUS_ID_FULLSYMBOL, 
                       m_pv->getStatusAttrValue(CIMIWinHandler::STATUS_ID_FULLSYMBOL));
}


void
SunPyInstance::refresh_status_property(bool cn)
{
    SCIM_DEBUG_IMENGINE (3) << get_id() << ": refresh_status_property(" << cn << ")\n";
    if (!cn) {
        reset();
    }
    _status_property.set_label(cn ? "中" : "英");
    update_property(_status_property);
}

void
SunPyInstance::refresh_fullsymbol_property(bool full)
{
    SCIM_DEBUG_IMENGINE (3) << get_id() << ": refresh_fullsimbol_property(" << full << ")\n";
    _letter_property.set_icon(
        full ? SCIM_FULL_LETTER_ICON : SCIM_HALF_LETTER_ICON);
    update_property(_letter_property);
}

void
SunPyInstance::refresh_fullpunc_property(bool full)
{
    _punct_property.set_icon(
        full ? SCIM_FULL_PUNCT_ICON : SCIM_HALF_PUNCT_ICON);
    update_property(_punct_property);
}

void
SunPyInstance::create_session(CHotkeyProfile *hotkey_profile)
{
    SCIM_DEBUG_IMENGINE (3) << get_id() <<  ": create_session()\n";

    CSunpinyinSessionFactory& factory = CSunpinyinSessionFactory::getFactory();
    factory.setPinyinScheme(CSunpinyinSessionFactory::QUANPIN);
    factory.setCandiWindowSize(10);
    m_pv = factory.createSession();
    if (!m_pv) {
        SCIM_DEBUG_IMENGINE (3) << get_id() <<  " factory.createSession() failed\n";
        return;
    }
    
    m_pv->setHotkeyProfile(hotkey_profile);

    m_lookup_table = new SunLookupTable();
    m_wh = new CScimWinHandler(this, m_lookup_table);
    m_pv->attachWinHandler(m_wh);
}

void
SunPyInstance::destroy_session()
{
    SCIM_DEBUG_IMENGINE (3) << get_id() <<  ": destroy_session()\n";
    
    // wh and ic are not pointers, I don't think it's necessary to delete them
    // either
    delete m_pv;
    delete m_wh;
    delete m_lookup_table;
    
    m_pv = 0;
    m_wh = 0;
    m_lookup_table = 0;
}

AttributeList
SunPyInstance::build_preedit_attribs (const IPreeditString* ppd)
{
    AttributeList attrs;
    const int sz = ppd->charTypeSize();
    for (int i = 0; i < sz; ) {
        const int ct = ppd->charTypeAt(i);
        if (ct & IPreeditString::ILLEGAL) {
            const int start = i;
            for (++i; (i<sz) && (ppd->charTypeAt(i) & IPreeditString::ILLEGAL); ++i) ;
            attrs.push_back( Attribute(start, i-start,
                                       SCIM_ATTR_DECORATE, SCIM_ATTR_DECORATE_REVERSE));
        } else if (ct & IPreeditString::NORMAL_CHAR) {
            if (ct & IPreeditString::USER_CHOICE) {
                const int start = i;
                for (++i; (i<sz) && (ppd->charTypeAt(i) & IPreeditString::USER_CHOICE); ++i) ;
                attrs.push_back( Attribute(start, i-start,
                                           SCIM_ATTR_DECORATE, SCIM_ATTR_DECORATE_UNDERLINE));
            } else {
                ++i;
            }
        } else {
            ++i;
        }
    }
    return attrs;
}

void
SunPyInstance::redraw_preedit_string (const IPreeditString* ppd)
{
    SCIM_DEBUG_IMENGINE (3) << get_id() <<  ": redraw_preedit_string()\n";
    if (ppd->size() != 0) {
        AttributeList attrs;
        const int caret = ppd->caret();
        if (caret > 0 && caret <= ppd->size()) {
            attrs.push_back( Attribute(ppd->candi_start(),
                                       ppd->charTypeSize(),
                                       SCIM_ATTR_DECORATE, SCIM_ATTR_DECORATE_REVERSE));
        }
        update_preedit_string( wstr_to_widestr(ppd->string(), ppd->size()) );
        show_preedit_string ();
        update_preedit_caret (caret);
    } else {
        hide_preedit_string ();
    }
}

void
SunPyInstance::redraw_lookup_table(const ICandidateList* pcl)
{
    SCIM_DEBUG_IMENGINE (3) << get_id() << ": redraw_lookup_table()\n";
    
    m_lookup_table->update(*pcl);
    if (m_lookup_table->number_of_candidates()) {
        update_lookup_table(*m_lookup_table);
        show_lookup_table();
    } else {
        hide_lookup_table();
    }
}

void
SunPyInstance::reload_config(const ConfigPointer &config)
{
    SCIM_DEBUG_IMENGINE (3) << get_id() << ": reload_config()\n";
    reset();
    if (m_factory->valid()) {
        m_factory->load_user_config();
    }
}

/*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
 * 
 * Copyright (c) 2010 Yong Sun <mail@yongsun.me>
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

#import "imi_session_wrapper.h"

CSunpinyinSessionWrapper::CSunpinyinSessionWrapper(id ic) : m_ic(ic), m_pv(0), m_wh(0), m_hotkey_profile(0)
{
    CSunpinyinSessionFactory& factory = CSunpinyinSessionFactory::getFactory();
    
    m_pv = factory.createSession();
    
    if (!m_pv)
        return;

    m_hotkey_profile = new CHotkeyProfile();
    m_pv->setHotkeyProfile (m_hotkey_profile);

    m_wh = new CIMKitWindowHandler(m_ic);
    m_pv->attachWinHandler (m_wh);
    
    apply_configuration ();
}

CSunpinyinSessionWrapper::~CSunpinyinSessionWrapper()
{
    if (m_pv) {
        CSunpinyinSessionFactory& factory = CSunpinyinSessionFactory::getFactory();
        factory.destroySession(m_pv);
    }

    delete m_hotkey_profile;
    delete m_wh;
}

void CSunpinyinSessionWrapper::switchInputMode (bool isEnglish, ECommitPolicies policy)
{
    if (!isValid())
        return;

    if (isEnglish) {
        // We need two spaces to commit in modern style
        if (COMMIT_PINYIN_STRING == policy)
            m_pv->onKeyEvent (CKeyEvent(IM_VK_ENTER, 0, 0));
        else if (COMMIT_CONVERTED_SENTENCE == policy)
            m_pv->onCandidateSelectRequest (0);
    }
}

bool CSunpinyinSessionWrapper::onConfigChanged (const COptionEvent& event)
{
    if (!isValid())
        return false;

    if (event.name == CONFIG_GENERAL_PAGE_SIZE) {
        update_cand_window_size(event.get_int());
    } else if (event.name == CONFIG_GENERAL_CHARSET_LEVEL) {
        update_charset_level(event.get_int());
    } else if (event.name == CONFIG_KEYBOARD_PAGE_COMMA) {
        update_page_key_comma(event.get_bool());
    } else if (event.name == CONFIG_KEYBOARD_PAGE_MINUS) {
        update_page_key_minus(event.get_bool());
    } else if (event.name == CONFIG_KEYBOARD_PAGE_BRACKET) {
        update_page_key_bracket(event.get_bool());
    } else if (event.name == CONFIG_KEYBOARD_PAGE_ARROWS) {
        update_page_key_arrows(event.get_bool());
    } else if (event.name == CONFIG_KEYBOARD_MISC_CANCELONBSP) {
        m_pv->setCancelOnBackspace(CSessionConfigStore::instance().m_cancel_on_backspace);
    }
    
    return false;
}

void CSunpinyinSessionWrapper::apply_configuration()
{
    m_hotkey_profile->setPunctSwitchKey(CKeyEvent(IM_VK_PERIOD, IM_VK_PERIOD, IM_CTRL_MASK));    
    m_hotkey_profile->setSymbolSwitchKey(CKeyEvent(IM_VK_SPACE, IM_VK_SPACE, IM_ALT_MASK));
    m_hotkey_profile->setCandiDeleteKey(CKeyEvent(0, 0, IM_CTRL_MASK|IM_SUPER_MASK));

    update_page_key_minus  (CSessionConfigStore::instance().m_paging_by_minus_equals);
    update_page_key_comma  (CSessionConfigStore::instance().m_paging_by_comma_period);
    update_page_key_bracket(CSessionConfigStore::instance().m_paging_by_brackets);
    update_page_key_arrows (CSessionConfigStore::instance().m_paging_by_arrows);
    
    m_pv->setCancelOnBackspace(CSessionConfigStore::instance().m_cancel_on_backspace);
}

void CSunpinyinSessionWrapper::update_cand_window_size(unsigned size)
{
    m_pv->setCandiWindowSize(size);
}

void CSunpinyinSessionWrapper::update_page_key_minus(bool enable)
{
    update_page_key(IM_VK_MINUS, IM_VK_EQUALS, enable);
}

void CSunpinyinSessionWrapper::update_page_key_comma(bool enable)
{
    update_page_key(IM_VK_COMMA, IM_VK_PERIOD, enable);
}

void CSunpinyinSessionWrapper::update_page_key_bracket(bool enable)
{
    update_page_key(IM_VK_OPEN_BRACKET, IM_VK_CLOSE_BRACKET, enable);
}

void CSunpinyinSessionWrapper::update_page_key_arrows(bool enable)
{
    update_page_key(IM_VK_UP, IM_VK_DOWN, enable);
}

void CSunpinyinSessionWrapper::update_page_key(unsigned page_up, unsigned page_down, bool enable)
{
    if (enable) {
        m_hotkey_profile->addPageUpKey(CKeyEvent(page_up, page_up));
        m_hotkey_profile->addPageDownKey(CKeyEvent(page_down, page_down));
    } else {
        m_hotkey_profile->removePageUpKey(CKeyEvent(page_up, page_up));
        m_hotkey_profile->removePageDownKey(CKeyEvent(page_down, page_down));
    }
}

void CSunpinyinSessionWrapper::update_charset_level(unsigned charset)
{
    // charset can only be 0,1,2 or 3
    m_pv->getIC()->setCharsetLevel(charset & 3);
}

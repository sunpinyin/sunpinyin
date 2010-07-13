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

#import "imi_options.h"
#import "imi_option_keys.h"
#import "imi_imkitwin.h"

#define CONFIG_GENERAL_PAGE_SIZE         "General/PageSize"

#define CONFIG_KEYBOARD_PAGE_COMMA       "Keyboard/Page/CommaPeriod"
#define CONFIG_KEYBOARD_PAGE_MINUS       "Keyboard/Page/MinusEquals"
#define CONFIG_KEYBOARD_PAGE_BRACKET     "Keyboard/Page/Brackets"
#define CONFIG_KEYBOARD_PAGE_ARROWS      "Keyboard/Page/Arrows"

#define CONFIG_KEYBOARD_MISC_CANCELONBSP "Keyboard/Misc/CancelOnBackspace"

typedef enum {
    COMMIT_PINYIN_STRING        = 0,
    COMMIT_CONVERTED_SENTENCE   = 1,
    RESERVE_CONTEXT             = 2,
} ECommitPolicies;

struct CSessionConfigStore : private CNonCopyable
{
    bool        m_paging_by_comma_period;
    bool        m_paging_by_minus_equals;
    bool        m_paging_by_brackets;
    bool        m_paging_by_arrows;
    bool        m_cancel_on_backspace;

    static CSessionConfigStore& instance () 
    {
        static CSessionConfigStore inst;
        return inst;
    }
};

class CSunpinyinSessionWrapper : public IConfigurable, private CNonCopyable
{
public:
     CSunpinyinSessionWrapper (id ic);
    ~CSunpinyinSessionWrapper ();
	
	bool isValid () const 
	    {return m_pv != NULL;}
	
	unsigned clear(void)
	    {return isValid()? m_pv->clearIC(): 0;}
	
	bool onKeyEvent(const CKeyEvent& event)
	    {return isValid()? m_pv->onKeyEvent(event): false;}	
    
    void setStatusAttrValue(int key, int value)
        {if (m_pv) m_pv->setStatusAttrValue(key, value);}
	
    bool onConfigChanged (const COptionEvent& event);
    
    void switchInputMode (bool isEnglish, ECommitPolicies policy);

private:
    void apply_configuration();
    
    void update_cand_window_size(unsigned);
    void update_charset_level(unsigned);
    
    void update_page_key_minus(bool);
    void update_page_key_comma(bool);
    void update_page_key_bracket(bool);
    void update_page_key_arrows(bool);
    void update_page_key(unsigned, unsigned, bool);
    
private:
    id                   m_ic;
    CIMIView            *m_pv;
    CIMKitWindowHandler	*m_wh;
    CHotkeyProfile      *m_hotkey_profile;
};

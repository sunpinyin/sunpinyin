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

#include "sunpinyin_utils.h"
#include "sunpinyin_keycode.h"

using namespace scim;

KeyEvent SunKeyEvent::m_key;

/**
 * translate from SCIM's KeyEvent to SunKeyEvent
 */
SunKeyEvent::SunKeyEvent(const KeyEvent& key)
    :  code(0), value(0), modifier(0)
{
    code = key.code;
    // refer to definition of struct ImeKeyRec in sunpinyin/ime/iiim/cle/ime.h
    value = key.get_ascii_code();
    if (key.is_shift_down())     modifier |= IM_SHIFT_MASK;
    if (key.is_control_down())   modifier |= IM_CTRL_MASK;
    if (key.is_alt_down())       modifier |= IM_ALT_MASK;
    
    m_is_shift = is_shift_released(m_key, key);
    m_key = key;
}

bool
SunKeyEvent::is_release() const
{
    return !is_shift() && m_key.is_key_release();
}

bool
SunKeyEvent::is_shift() const
{
    return m_is_shift;
}

// because shift by its own is a modifier key, we need take more care of it
bool
SunKeyEvent::is_key_shift(const KeyEvent& key)
{
    return ( key.code == SCIM_KEY_Shift_L &&
             !(key.mask & ~(SCIM_KEY_ShiftMask | SCIM_KEY_ReleaseMask)));
    // ensure that none of modifier keys is pressed except for shift
}

bool
SunKeyEvent::is_shift_released(const KeyEvent& prev_key, const KeyEvent& curr_key)
{
    return ( is_key_shift(curr_key) &&
             curr_key.is_key_release() &&
             is_key_shift(prev_key));
}

WideString
wstr_to_widestr(const TWCHAR* wstr)
{
    WideString wide_str;
    copy(wstr, wstr+WCSLEN(wstr)+1, back_inserter(wide_str));
    return wide_str;
}

WideString
wstr_to_widestr(const TWCHAR* wstr, size_t len)
{
    WideString wide_str;
    copy(wstr, wstr+len+1, back_inserter(wide_str));
    return wide_str;
}


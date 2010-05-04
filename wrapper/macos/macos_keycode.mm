/*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
 * 
 * Copyright (c) 2008 Sun Microsystems, Inc. All Rights Reserved.
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

#import "macos_keycode.h"

static unsigned osx_modifiers_to_ime_modifiers (unsigned modifiers) 
{
    unsigned ret = 0;

    if (modifiers & OSX_SHIFT_MASK)
        ret |= IM_SHIFT_MASK;
    if (modifiers & OSX_CTRL_MASK)
        ret |= IM_CTRL_MASK;
    if (modifiers & OSX_ALT_MASK)
        ret |= IM_ALT_MASK;
    if (modifiers & OSX_COMMAND_MASK)
        ret |= IM_SUPER_MASK;

    return ret;
}

static unsigned osx_keycode_to_ime_keycode (unsigned keycode) 
{
    unsigned ret = 0;

    switch (keycode) {
        case OSX_VK_SPACE:
            ret = IM_VK_SPACE;
            break;
        case OSX_VK_MINUS:
            ret = IM_VK_MINUS;
            break;
        case OSX_VK_EQUALS:
            ret = IM_VK_EQUALS;
            break;
        case OSX_VK_COMMA:
            ret = IM_VK_COMMA;
            break;
        case OSX_VK_PERIOD:
            ret = IM_VK_PERIOD;
            break;
        case OSX_VK_OPEN_BRACKET:
            ret = IM_VK_OPEN_BRACKET;
            break;
        case OSX_VK_CLOSE_BRACKET:
            ret = IM_VK_CLOSE_BRACKET;
            break;
        case OSX_VK_BACK_QUOTE:
            ret = IM_VK_BACK_QUOTE;
            break;
        case OSX_VK_ENTER:
            ret = IM_VK_ENTER;
            break;
        case OSX_VK_BACK_SPACE:
            ret = IM_VK_BACK_SPACE;
            break;
        case OSX_VK_ESCAPE:
            ret = IM_VK_ESCAPE;
            break;
        case OSX_VK_PAGE_UP:
            ret = IM_VK_PAGE_UP;
            break;
        case OSX_VK_PAGE_DOWN:
            ret = IM_VK_PAGE_DOWN;
            break;
        case OSX_VK_END:
            ret = IM_VK_END;
            break;
        case OSX_VK_HOME:
            ret = IM_VK_HOME;
            break;
        case OSX_VK_LEFT:
            ret = IM_VK_LEFT;
            break;
        case OSX_VK_UP:
            ret = IM_VK_UP;
            break;
        case OSX_VK_RIGHT:
            ret = IM_VK_RIGHT;
            break;
        case OSX_VK_DOWN:
            ret = IM_VK_DOWN;
            break;
        case OSX_VK_DELETE:
            ret = IM_VK_DELETE;
            break;
        case OSX_VK_CONTROL_L:
            ret = IM_VK_CONTROL_L;
            break;
        case OSX_VK_CONTROL_R:
            ret = IM_VK_CONTROL_R;
            break;
        case OSX_VK_SHIFT_L:
            ret = IM_VK_SHIFT_L;
            break;
        case OSX_VK_SHIFT_R:
            ret = IM_VK_SHIFT_R;
            break;
        case OSX_VK_ALT:
            ret = IM_VK_ALT;
            break;
        default:
            ret = 0;
            break;
    }
    
    return ret;
}

CKeyEvent osx_keyevent_to_ime_keyevent (unsigned keyCode, unsigned keyChar, unsigned modifiers)
{
    unsigned value = osx_keycode_to_ime_keycode (keyCode);
    unsigned mod = osx_modifiers_to_ime_modifiers (modifiers);
    return CKeyEvent (value, keyChar, mod);
}


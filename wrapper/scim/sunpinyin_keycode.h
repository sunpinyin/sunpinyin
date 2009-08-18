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

#ifndef SUNPINYIN_KEYCODE_H
#define SUNPINYIN_KEYCODE_H

#define Uses_SCIM_TYPES
#define Uses_SCIM_EVENT
#include <scim.h>

using namespace scim;

#define IM_SHIFT_MASK        SCIM_KEY_ShiftMask
#define IM_CTRL_MASK         SCIM_KEY_ControlMask
#define IM_META_MASK         SCIM_KEY_MetaMask
#define IM_ALT_MASK          SCIM_KEY_AltMask

#define IM_VK_DELETE         SCIM_KEY_Delete
#define IM_VK_ENTER          SCIM_KEY_Return
#define IM_VK_BACK_SPACE     SCIM_KEY_BackSpace
#define IM_VK_TAB            SCIM_KEY_Tab
#define IM_VK_SHIFT          SCIM_KEY_Shift_L
#define IM_VK_CONTROL        SCIM_KEY_Control_L
#define IM_VK_ALT            SCIM_KEY_Alt_L
#define IM_VK_CAPS_LOCK      SCIM_KEY_Caps_Lock
#define IM_VK_ESCAPE         SCIM_KEY_Escape
#define IM_VK_SPACE          SCIM_KEY_space
#define IM_VK_PAGE_UP        SCIM_KEY_Page_Up
#define IM_VK_PAGE_DOWN      SCIM_KEY_Page_Down
#define IM_VK_END            SCIM_KEY_End
#define IM_VK_HOME           SCIM_KEY_Home
#define IM_VK_LEFT           SCIM_KEY_Left
#define IM_VK_UP             SCIM_KEY_Up
#define IM_VK_RIGHT          SCIM_KEY_Right
#define IM_VK_DOWN           SCIM_KEY_Down

#define IM_VK_PAGE_UP        SCIM_KEY_Page_Up
#define IM_VK_PAGE_DOWN      SCIM_KEY_Page_Down
#define IM_VK_OPEN_BRACKET   SCIM_KEY_bracketleft
#define IM_VK_CLOSE_BRACKET  SCIM_KEY_bracketright
#define IM_VK_COMMA          SCIM_KEY_comma
#define IM_VK_PERIOD         SCIM_KEY_period
#define IM_VK_MINUS          SCIM_KEY_minus
#define IM_VK_EQUALS         SCIM_KEY_equal

#define IM_VK_SLASH          SCIM_KEY_slash
#define IM_VK_BACK_QUOTE     SCIM_KEY_grave
#define IM_VK_QUOTE          SCIM_KEY_apostrophe
#define IM_VK_QUOTEDBL       SCIM_KEY_quotedbl

#define IM_VK_0              SCIM_KEY_0
#define IM_VK_1              SCIM_KEY_1
#define IM_VK_2              SCIM_KEY_2
#define IM_VK_3              SCIM_KEY_3
#define IM_VK_4              SCIM_KEY_4
#define IM_VK_5              SCIM_KEY_5
#define IM_VK_6              SCIM_KEY_6
#define IM_VK_7              SCIM_KEY_7
#define IM_VK_8              SCIM_KEY_8
#define IM_VK_9              SCIM_KEY_9

#define IM_VK_K              SCIM_KEY_k

#endif//SUNPINYIN_KEYCODE_H

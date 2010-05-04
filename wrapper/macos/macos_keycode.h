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

#ifndef _MACOS_KEYCODE_H_
#define _MACOS_KEYCODE_H_

#import "imi_keys.h"

/* get the definitions from <AppKit/NSEvent.h> and <Carbon/Events.h> */

#define OSX_SHIFT_MASK        1 << 17
#define OSX_CTRL_MASK         1 << 18
#define OSX_ALT_MASK          1 << 19
#define OSX_COMMAND_MASK      1 << 20

#define OSX_VK_SPACE          0x31
#define OSX_VK_MINUS          0x1B
#define OSX_VK_EQUALS         0x18
#define OSX_VK_COMMA          0x2B
#define OSX_VK_PERIOD         0x2F
#define OSX_VK_OPEN_BRACKET   0x21
#define OSX_VK_CLOSE_BRACKET  0x1E
#define OSX_VK_BACK_QUOTE     0x32

#define OSX_VK_ENTER          0x24
#define OSX_VK_BACK_SPACE     0x33
#define OSX_VK_ESCAPE         0x35
#define OSX_VK_PAGE_UP        0x74
#define OSX_VK_PAGE_DOWN      0x79
#define OSX_VK_END            0x77
#define OSX_VK_HOME           0x73
#define OSX_VK_LEFT           0x7B
#define OSX_VK_UP             0x7E
#define OSX_VK_RIGHT          0x7C
#define OSX_VK_DOWN           0x7D
#define OSX_VK_DELETE         0x75

#define OSX_VK_CONTROL_L      0x3B
#define OSX_VK_CONTROL_R      0x3E
#define OSX_VK_SHIFT_L        0x38
#define OSX_VK_SHIFT_R        0x3C
#define OSX_VK_ALT            0x3A

extern CKeyEvent osx_keyevent_to_ime_keyevent (unsigned, unsigned, unsigned);

#endif /* _MACOS_KEYCODE_H_ */

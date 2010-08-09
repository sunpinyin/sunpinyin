/*
 * Copyright (c) 2010 Mike Qin <mikeandmore@gmail.com>
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

#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "common.h" 

__BEGIN_DECLS

typedef struct {
    int x;
    int y;
} position_t;

typedef struct {
    int modifiers;
    int keysym;
} hotkey_t;

typedef char varchar[128];

typedef enum {
    TRIGGER_KEY = 0,
    ENG_KEY,
    ICBAR_POS,
    PREEDIT_OPACITY,
    PREEDIT_COLOR,
    PREEDIT_FONT,
    PREEDIT_FONT_COLOR,
    SHUANGPIN,
    SHUANGPIN_SCHEME,
    CANDIDATES_SIZE,
    PAGE_MINUS_PLUS,
    PAGE_COMMA_PERIOD,
    PAGE_PAREN,
    FUZZY_SEGMENTATION,
    FUZZY_INNER_SEGMENTATION,
    CANCEL_ON_BACKSPACE,
    SMART_PUNCT,
} setting_key_t;

#define MAX_KEY 0xff

void  settings_init();
void  settings_destroy();

void  settings_load();
void  settings_save();

void  settings_get(setting_key_t key, void* data);
void  settings_set(setting_key_t key, void* data);

void  settings_set_int(setting_key_t key, int value);
void  settings_set_double(setting_key_t key, double value);
void  settings_set_string(setting_key_t key, const char* str);

int    settings_get_int(setting_key_t key);
double settings_get_double(setting_key_t key);

__END_DECLS

#endif /* _SETTINGS_H_ */

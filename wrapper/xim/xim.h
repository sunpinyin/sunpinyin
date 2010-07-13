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

#ifndef _XIM_H_
#define _XIM_H_

#include <X11/X.h>
#include <X11/Xproto.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <XimProto.h>
#include <IMdkit.h>
#include <Xi18n.h>

#include "common.h" 

__BEGIN_DECLS

#define STATE_MASK 0x0D

typedef struct _XIMS XIMHandle;

XIMHandle* create_xim_server(const char* server_name, const char* locale);

void      xim_start_preedit (XIMHandle* handle);
void      xim_cancel_preedit(XIMHandle* handle);
void      xim_commit_preedit(XIMHandle* handle, const char* result_str);

/* provided by preedit backend */
void      preedit_init(void);
void      preedit_finalize(void);

void      preedit_set_handle(XIMHandle* handle);
void      preedit_reload(void);

void      preedit_move(int x, int y);
void      preedit_on_key(XIMHandle* handle, unsigned int keycode,
                         unsigned int state);
bool      preedit_status(void);
void      preedit_pause(void);
void      preedit_go_on(void);
void      preedit_set_full(bool full);
void      preedit_set_chinese_punc(bool chn_punc);
void      preedit_omit_next_punct();

__END_DECLS

#endif /* _XIM_H_ */

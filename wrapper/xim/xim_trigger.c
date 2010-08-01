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

#include "xim.h"
#include "ic.h"
#include "common.h"
#include "settings.h"

#include <stdio.h>

static void
__toggle_preedit_status(XIMHandle* handle, bool flag)
{
    if (flag)
        xim_start_preedit(handle);
    else
        xim_cancel_preedit(handle);
}

static bool last_shift_available = false;

static bool
__filter_forward_triger_key(XIMHandle* handle, IC* ic, XKeyEvent* evt)
{
    KeySym sym = 0;
    hotkey_t hk;

    settings_get(TRIGGER_KEY, &hk);
    XLookupString(evt, NULL, 0, &sym, NULL);

    unsigned int masked_state = (evt->state & STATE_MASK);

    if (masked_state == hk.modifiers && sym == hk.keysym) {
        if (evt->type == KeyRelease) return true;
        ic->is_enabled = !ic->is_enabled;
        __toggle_preedit_status(handle, ic->is_enabled);
        icmgr_refresh();
        return true;
    }

    settings_get(ENG_KEY, &hk);
    if ((masked_state & hk.modifiers) == hk.modifiers && sym == hk.keysym) {
        if (evt->type == KeyPress) {
            last_shift_available = true;
            return true;
        } else if (evt->type == KeyRelease && last_shift_available) {
            last_shift_available = false;
            if (ic->is_enabled) {
                ic->is_english = !ic->is_english;
                icmgr_refresh();
            }
        }
        return true;
    } else {
        last_shift_available = false;
    }

    return false;
}

static void
__do_forward_event(XIMHandle* handle, IMForwardEventStruct* proto)
{
    /* by pass the event */
    IMForwardEventStruct fe;
    memset(&fe, 0, sizeof (fe));
 
    fe.major_code = XIM_FORWARD_EVENT;
    fe.icid = proto->icid;
    fe.connect_id = proto->connect_id;
    fe.sync_bit = 0;
    fe.serial_number = 0L;
    fe.event = proto->event;
 
    IMForwardEvent(handle, (XPointer) &fe);
}

extern void __move_preedit(IC* ic);

int
_xim_forward_event(XIMHandle* handle, IMForwardEventStruct* proto)
{
    LOG("%d", proto->icid);
    XKeyEvent* evt = (XKeyEvent*) &(proto->event);
    IC* ic = icmgr_get_current();
    if (ic == NULL) {
        icmgr_set_current(proto->icid);
        ic = icmgr_get_current();
        if (ic == NULL)
            return 1;
    }

    /* some of swing applications like netbeans won't set focus,
     * we have to set focus ourself
     */
    if (ic->icid != proto->icid || preedit_status() == false) {
        icmgr_set_current(proto->icid);
        ic = icmgr_get_current();
        icmgr_refresh();
    }
    
    if (!__filter_forward_triger_key(handle, ic, evt)) {
        int masked_state = evt->state & STATE_MASK;
        if (masked_state != ShiftMask && masked_state != 0) {
            __do_forward_event(handle, proto);
        } else if (!ic->is_enabled || ic->is_english) {
            __do_forward_event(handle, proto);
        } else {
            KeySym sym;
            XLookupString(evt, NULL, 0, &sym, NULL);
            if ((sym <= 0x20 || sym > 0x7E) && preedit_status() == false) {
                __do_forward_event(handle, proto);
            } else if (sym >= 0x30 && sym <= 0x39
                       && preedit_status() == false) {
                // digit key pressed
                if (settings_get_int(SMART_PUNCT) && evt->type == KeyPress) {
                    preedit_omit_next_punct();
                }
                
                __do_forward_event(handle, proto);
            } else {
                if (evt->type == KeyPress) {
                    __move_preedit(ic);
                    preedit_on_key(handle, sym, evt->state);
                } else {
                    LOG("ignore");
                }
            }
        }
    }
    return 1;
}

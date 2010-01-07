/**
 * @file see ``LICENSE`` for further details
 * @author Mike
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
    if (evt->state == hk.modifiers && sym == hk.keysym) {
        if (evt->type == KeyRelease) return true;
        ic->is_enabled = !ic->is_enabled;
        __toggle_preedit_status(handle, ic->is_enabled);
        icmgr_refresh_ui();
        return true;
    }

    settings_get(ENG_KEY, &hk);
    if ((evt->state & hk.modifiers) == hk.modifiers && sym == hk.keysym) {
        if (evt->type == KeyPress) {
            last_shift_available = true;
            return true;
        } else if (evt->type == KeyRelease && last_shift_available) {
            last_shift_available = false;
            if (ic->is_enabled) {
                ic->is_english = !ic->is_english;
                icmgr_refresh_ui();
                if (ic->is_english)
                    preedit_pause();
                else
                    preedit_go_on();
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
    DEBUG("%d", proto->icid);
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
        icmgr_refresh_ui();
    }
    
    if (!__filter_forward_triger_key(handle, ic, evt)) {

        if (evt->state != ShiftMask && evt->state != 0)
            __do_forward_event(handle, proto);
        else if (!ic->is_enabled || ic->is_english)
            __do_forward_event(handle, proto);
        else {
            KeySym sym;
            XLookupString(evt, NULL, 0, &sym, NULL);
            if ((sym <= 0x20 || sym > 0x7E) && preedit_status() == false)
                __do_forward_event(handle, proto);
            else if (sym >= 0x30 && sym <= 0x39 && preedit_status() == false)
                __do_forward_event(handle, proto);
            else {
                if (evt->type == KeyPress) {
                    __move_preedit(ic);
                    if (preedit_status()) {
                        if (sym == 0x2D) // '-' into page up
                            sym = XK_Page_Up;
                        if (sym == 0x3D) // '=' into page down
                            sym = XK_Page_Down;
                    }
                    
                    preedit_on_key(handle, sym, evt->state);
                } else {
                    LOG("ignore");
                }
            }
        }
    }
    return 1;
}

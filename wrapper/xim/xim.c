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

#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <langinfo.h>
#include <locale.h>
#include <iconv.h>
#include <signal.h>
#include <stdlib.h>
#include <stdbool.h>

#include "xim.h"
#include "xmisc.h"
#include "ic.h"
#include "common.h"
#include "settings.h"


static int __preedit_x;
static int __preedit_y;
static int __input_style;
static Window __client_window;
static Window __focus_window;

static int
_xim_open(XIMHandle* handle, IMOpenStruct* proto)
{
    LOG("XIM_OPEN %d", proto->connect_id);
    return 1;
}

static int
_xim_close(XIMHandle* handle, IMCloseStruct* proto)
{
    LOG("XIM_CLOSE %d", proto->connect_id);
    return 1;
}

static int
__xim_ic_events(IMChangeICStruct* proto)
{
    XICAttribute* ic_attr = proto->ic_attr;
    XICAttribute* pre_attr = proto->preedit_attr;

    int i = 0;
    for (i = 0; i < (int) proto->ic_attr_num; i++) {
        if (strcmp(XNInputStyle, ic_attr[i].name) == 0) {
            LOG("got input style %d", __input_style);
            __input_style = * (int*) ic_attr[i].value;
        } else if (strcmp(XNClientWindow, ic_attr[i].name) == 0) {
            LOG("got client window");
            __client_window = * (Window*) ic_attr[i].value;
        } else if (strcmp(XNFocusWindow, ic_attr[i].name) == 0) {
            LOG("got focus window");
            __focus_window = * (Window*) ic_attr[i].value;
        }
    }
    for (i = 0; i < (int) proto->preedit_attr_num; i++) {
        if (strcmp(XNSpotLocation, pre_attr[i].name) == 0) {
            XPoint* point = pre_attr[i].value;
            __preedit_x = point->x;
            __preedit_y = point->y;
            LOG("position (%d, %d)", __preedit_x, __preedit_y);
        }
    }
    return 1;
}

static int
_xim_create_ic(XIMHandle* handle, IMChangeICStruct* proto)
{
    IC* ic = icmgr_create_ic(proto->connect_id);
    __xim_ic_events(proto);
    proto->icid = ic->icid;
    ic->client_window = __client_window;
    ic->offset_x = __preedit_x;
    ic->offset_y = __preedit_y;
    LOG("XIM_CREATE_IC %d", proto->icid);
    
    return 1;
}

static int
_xim_destroy_ic(XIMHandle* handle, IMChangeICStruct* proto)
{
    LOG("XIM_DESTROY_IC %d", proto->icid);
    icmgr_destroy_ic(proto->icid);
    icmgr_refresh();
    return 1;
}

void
__move_preedit(IC* ic)
{
    int root_x, root_y;
    get_window_position(ic->client_window, &root_x, &root_y);
    LOG("root: %d, %d offset: %d,%d", root_x, root_y,
        ic->offset_x, ic->offset_y);
    if (ic->offset_x == 0 && ic->offset_y == 0) {
        int height;
        get_window_size(ic->client_window, NULL, &height);
        root_x += 4;
        root_y += height;
    } else {
        root_x += ic->offset_x;
        root_y += ic->offset_y;
    }

    preedit_move(root_x, root_y);
}

static int
_xim_set_ic_values(XIMHandle* handle, IMChangeICStruct* proto)
{
    __xim_ic_events(proto);
    IC* ic = icmgr_get(proto->icid);
    /* some crapy swing application will have synchronization problems */
    if (ic == NULL)
        return 1;

    LOG("XIM_SET_IC_VALUES %d", proto->icid);
    ic->offset_x = __preedit_x;
    ic->offset_y = __preedit_y;
    IC* cur_ic = icmgr_get_current();

    /* if we change the current ic position, we might wanna
     * move it along the way
     */
    if (cur_ic != NULL && ic->icid == cur_ic->icid) {
        __move_preedit(ic);
    }
    return 1;
}

static int
_xim_get_ic_values(XIMHandle* handle, IMChangeICStruct* proto)
{
    LOG("XIM_GET_IC_VALUES %d", proto->icid);
    XICAttribute* ic_attr = proto->ic_attr;
    
    int i;
    for (i = 0; i < (int) proto->ic_attr_num; i++) {
        if (strcmp(XNFilterEvents, ic_attr[i].name) == 0) {
            ic_attr[i].value = malloc(sizeof(CARD32));
            *((CARD32*) ic_attr[i].value) = KeyPressMask | KeyPress;
            ic_attr[i].value_length = sizeof(CARD32);
        }
    }
    return 1;
}

static int
_xim_trigger_notify(XIMHandle* handle, IMTriggerNotifyStruct* proto)
{
    LOG("trigger key pressed, %d", proto->icid);
    IC* ic = icmgr_get(proto->icid);
    if (ic == NULL)
        return 1;

    icmgr_set_current(proto->icid);
    ic->is_enabled = true;
    xim_start_preedit(handle);
    icmgr_refresh();
    return 1;
}

static int
_xim_set_ic_focus(XIMHandle* handle, IMChangeFocusStruct* proto)
{
    DEBUG("%d", proto->icid);
    LOG("set focus on ic %d %d", proto->icid, preedit_status());
    /* if use didn't finish typing, we won't focus to new context */
    if (preedit_status() == false) {
        icmgr_set_current(proto->icid);
    }
    icmgr_refresh();

    return 1;
}

static int
_xim_unset_ic_focus(XIMHandle* handle, IMChangeFocusStruct* proto)
{
    LOG("unset focus on ic %d", proto->icid);

    IC* ic = icmgr_get_current();
    if (ic != NULL && ic->icid == proto->icid && preedit_status() == false) {
        icmgr_clear_current();
        icmgr_refresh();
    }
    return 1;
}

extern int _xim_forward_event(XIMHandle* handle,
                              IMForwardEventStruct* proto);

static int
_imdkit_protocol_hanlder(XIMHandle* handle, IMProtocol* proto)
{
    assert(handle != NULL);
    assert(proto != NULL);

    switch (proto->major_code) {
    case XIM_OPEN:
        return _xim_open(handle, (IMOpenStruct *) proto);
    case XIM_CLOSE:
        return _xim_close(handle, (IMCloseStruct*) proto);
    case XIM_CREATE_IC:
        return _xim_create_ic(handle, (IMChangeICStruct*) proto);
    case XIM_DESTROY_IC:
        return _xim_destroy_ic(handle, (IMChangeICStruct*) proto);
    case XIM_SET_IC_VALUES:
        return _xim_set_ic_values(handle, (IMChangeICStruct*) proto);
    case XIM_GET_IC_VALUES:
        return _xim_get_ic_values(handle, (IMChangeICStruct*) proto);
    case XIM_TRIGGER_NOTIFY:
          return _xim_trigger_notify(handle, (IMTriggerNotifyStruct*) proto);
    case XIM_FORWARD_EVENT:
        return _xim_forward_event(handle, (IMForwardEventStruct *) proto);
    case XIM_SET_IC_FOCUS:
        return _xim_set_ic_focus(handle, (IMChangeFocusStruct *) proto);
    case XIM_UNSET_IC_FOCUS:
        return _xim_unset_ic_focus(handle, (IMChangeFocusStruct *) proto);
    default:
        LOG("unhandled major code %d", proto->major_code);
        return 1;
    }
}

static XIMHandle*
_open_imdkit(const char* _server_name, const char* _locale)
{
    XIMStyle ims_styles_onspot [] = {
        XIMPreeditPosition | XIMStatusArea,        //OverTheSpot
        XIMPreeditPosition | XIMStatusNothing,     //OverTheSpot
        XIMPreeditPosition | XIMStatusNone,        //OverTheSpot
        XIMPreeditNothing  | XIMStatusNothing,     //Root
        XIMPreeditNothing  | XIMStatusNone,        //Root
        0
    };
    XIMEncoding ims_encodings[] = {
        "COMPOUND_TEXT",
        0
    };

    /* this is rarely documentated, the trigger condition is
     * 
     * keycode == keysym && (state & modifier_mask) == modifier
     *
     * where keycode and state is the user pressed
     */
    hotkey_t hk;
    settings_get(TRIGGER_KEY, &hk);
    XIMTriggerKey trigger = {
        .keysym = hk.keysym,
        .modifier = hk.modifiers,
        .modifier_mask = STATE_MASK
    };

    XIMTriggerKeys keys;
    XIMStyles styles;
    XIMEncodings encodings;
 
    styles.count_styles =
        sizeof (ims_styles_onspot)/sizeof (XIMStyle) - 1;
    styles.supported_styles = ims_styles_onspot;
 
    encodings.count_encodings =
        sizeof (ims_encodings)/sizeof (XIMEncoding) - 1;
    encodings.supported_encodings = ims_encodings;

    keys.count_keys = 1;
    keys.keylist = &trigger;

    Window win = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy),
                                     0, 0, 1, 1, 1, 0, 0);
    XSelectInput(dpy, win,
                 ExposureMask | ButtonPressMask | ButtonReleaseMask
                 | ButtonMotionMask | VisibilityChangeMask);
    
    XIMHandle* handle =
        IMOpenIM(dpy,
                 IMModifiers, "Xi18n",
                 IMServerWindow, win,
                 IMServerName, _server_name,
                 IMLocale, _locale,
                 IMServerTransport, "X/",
                 IMInputStyles, &styles,
                 IMEncodingList, &encodings,
                 IMProtocolHandler, _imdkit_protocol_hanlder,
                 IMFilterEventMask, KeyPressMask | KeyReleaseMask,
                 IMOnKeysList, &keys,
                 NULL);
    if (handle == NULL) {
        fprintf(stderr, "Startup xim server failed.\n");
        fprintf(stderr, "Your locale is %s, please file a bug.", _locale);
    }
    return handle;
}

XIMHandle*
create_xim_server(const char* server_name, const char* locale)
{
    XIMHandle* handle = _open_imdkit(server_name, locale);
    icmgr_init();
    return handle;
}

void
xim_start_preedit(XIMHandle* handle)
{
    IC* ic = icmgr_get_current();
    if (ic == NULL)
        return;

    IMPreeditStateStruct ps;
    ps.icid = ic->icid;
    ps.connect_id = ic->connect_id;
    IMPreeditStart(handle, (XPointer) &ps);
}

void
xim_cancel_preedit(XIMHandle* handle)
{
    IC* ic = icmgr_get_current();
    if (ic == NULL)
        return;

    IMPreeditStateStruct ps;
    ps.icid = ic->icid;
    ps.connect_id = ic->connect_id;
    IMPreeditEnd(handle, (XPointer) &ps);
}

void
xim_commit_preedit(XIMHandle* handle, const char* result_str)
{
    IC* ic = icmgr_get_current();
    if (ic == NULL)
        return;
    
    XTextProperty tp;
    IMCommitStruct cs;
    Xutf8TextListToTextProperty(dpy, (char**) &result_str, 1,
                                XCompoundTextStyle, &tp);

    cs.major_code = XIM_COMMIT;
    cs.icid = ic->icid;
    cs.connect_id = ic->connect_id;
    cs.flag = XimLookupChars;
    cs.commit_string = (char*) tp.value;
    IMCommitString(handle, (XPointer) &cs);
    XFree(tp.value);
}

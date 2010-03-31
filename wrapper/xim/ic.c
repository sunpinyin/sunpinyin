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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/Xos.h>
#include <X11/Xfuncs.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#include "common.h"
#include "ic.h"
#include "xmisc.h"
#include "xim.h"

static IC* icmaps[MAX_IC_NUM];
static IC ics[MAX_IC_NUM];

static size_t free_stack_sz = 0;
static IC* free_stack[MAX_IC_NUM];

static IC* current_ic;

// check if this ic is available
static int
__find_application_pid(Window w)
{
    if (w == DefaultRootWindow(dpy))
        return 0;

    Atom actual_type;
    int actual_format;
    unsigned long nitems;
    unsigned long bytes;
    unsigned char* prop;
    int status = XGetWindowProperty(
        dpy, w, XInternAtom(dpy, "_NET_WM_PID", True), 0,
        1024L, False, AnyPropertyType, &actual_type, &actual_format, &nitems,
        &bytes, (unsigned char**) &prop);
    if (status != 0) {
        if (status == BadRequest)
            return 0;
        return -1;
    }
    if (!prop) {
        Window parent;
        Window root;
        Window* children = NULL;
        unsigned int sz = 0;
        status = XQueryTree(dpy, w, &root, &parent, &children, &sz);
        if (status != 0) {
            if (status == BadRequest)
                return 0;
            return -1;
        }
        if (children)
            XFree(children);
        return __find_application_pid(parent);
    } else {
        // TODO: is this portable?
        return prop[1] * 256 + prop[0];
    }
}

static bool
__ic_available(IC* ic)
{
    int pid = __find_application_pid(ic->client_window);
    if (pid == -1)
        return false;
    if (pid == 0)
        return true;
    // verify if a process is running
    char path[256];
    snprintf(path, 255, "/proc/%d", pid);
    struct stat buf;
    if (stat(path, &buf) != 0) {
        LOG("GC can't catch the process %d", pid);
        return false;
    }
    if (!S_ISDIR(buf.st_mode)) {
        LOG("GC can't catch the process %d", pid);
        return false;
    }
    return true;
}

static void
__scan_all_ic()
{
    int i = 0;
    for (i = 0; i < MAX_IC_NUM; i++) {
        IC* ic = icmaps[i];
        if (ic == NULL)
            continue;
        
        if (__ic_available(ic) == false) {
            LOG("GC detected garbage %d", ic->icid);
            icmgr_destroy_ic(i);
        }
    }
}

extern void icmgr_init_ui(void);
extern void icmgr_refresh_ui(void);

static void
__reset_ic(IC* ic)
{
    int id = ic->icid;
    memset(ic, 0, sizeof(IC));
    ic->icid = id;
    ic->is_chn_punc = true;
}

void
icmgr_init(void)
{
    memset(ics, 0, sizeof(IC) * MAX_IC_NUM);
    int i;
    for (i = 0; i < MAX_IC_NUM; i++) {
        ics[i].icid = i + 1;
        __reset_ic(&ics[i]);
        
        free_stack[free_stack_sz] = &ics[i];
        free_stack_sz++;
    }
    current_ic = NULL;

    icmgr_init_ui();
}

void
icmgr_finalize(void)
{
    memset(icmaps, 0, sizeof(IC*) * MAX_IC_NUM);
    current_ic = NULL;
}


IC*
icmgr_create_ic(int connect_id)
{
    static int created_cnt = 0;
    
    created_cnt++;
    if (created_cnt == GC_THRESHOLD || free_stack_sz < MAX_IC_NUM / 3) {
        __scan_all_ic();
        created_cnt = 0;
    }
    
    if (free_stack_sz == 0) {
        LOG("Error free stack empty!!");
        return NULL;
    }
    
    free_stack_sz--;
    IC* ic = free_stack[free_stack_sz];
    
    icmaps[ic->icid] = ic;
    __reset_ic(ic);
    
    /* icmgr_set_current(ic->icid); */
    
    ic->connect_id = connect_id;

    /* current_ic = ic; */
    return ic;
}

void
icmgr_destroy_ic(int icid)
{
    IC* ic = icmaps[icid];
    if (ic == NULL)
        return;

    memset(ic, 0, sizeof(IC));
    ic->icid = icid;
    
    icmaps[icid] = NULL;
    
    // return to free stack
    free_stack[free_stack_sz] = ic;
    free_stack_sz++;
    
    current_ic = NULL;
}

bool
icmgr_set_current(int icid)
{
    IC* ic = icmaps[icid];
    if (ic == NULL)
        return false;
    current_ic = ic;
    return true;
}

IC*
icmgr_get_current(void)
{
    return current_ic;
}

IC*
icmgr_get(int icid)
{
    return icmaps[icid];
}

void
icmgr_clear_current(void)
{
    current_ic = NULL;
}

void
icmgr_refresh()
{
    if (current_ic == NULL) {
        icmgr_refresh_ui();
        return;
    }
    
    /* refresh preedit */
    if (current_ic->is_enabled) {
        if (current_ic->is_english && preedit_status())
            preedit_pause();
        else
            preedit_go_on();
        
        preedit_set_full(current_ic->is_full);
        preedit_set_chinese_punc(current_ic->is_chn_punc);
    } else {
        preedit_pause();
    }
    icmgr_refresh_ui();
}

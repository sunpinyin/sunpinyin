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

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <signal.h>
#include <langinfo.h>

#include "xim.h"
#include "xmisc.h"
#include "ic.h"
#include "settings.h"

#define XIM_NAME "xsunpinyin"

static void
on_app_sig(int sig)
{
    if (sig == SIGUSR1) {
        /* reload the settings */
        settings_load();
        preedit_reload();
    } else {
        preedit_finalize();
        icmgr_finalize();

        settings_save();
        settings_destroy();
    
        exit(0);
    }
}

static int
_xerror_handler (Display *dpy, XErrorEvent *e)
{
    // display closed
    if (e->error_code == 0x9E) {
        on_app_sig(SIGINT);
        exit(0);
    }
    printf(
        "XError: "
        "serial=%lu error_code=%d request_code=%d minor_code=%d resourceid=%lu",
        e->serial, e->error_code, e->request_code, e->minor_code, e->resourceid);
    return 1;
}

#define ALL_LOCALES_STRING "ca,cs,en,es,et,eu,fr,zh,zu"

int 
main(int argc, char* argv[])
{
    if (argc == 2 && strcmp(argv[1], "-d") == 0) {
        int pid = fork();
        if (pid < 0)
            return -1;
        else if (pid > 0)
            return 0;
    }
    
    init_display(&argc, &argv);
    XSetErrorHandler (_xerror_handler);

    settings_init();
    settings_load();
    
    preedit_init();

    /* check if the codeset is utf-8 */
    if (strcmp(nl_langinfo(CODESET), "UTF-8") != 0) {
        fprintf(stderr, "Warning: using codeset %s might cause xsunpinyin unable to trigger.\n", nl_langinfo(CODESET));
    }

    /* guess the locale */
    char* locale = getenv("LC_CTYPE");
    if (locale == NULL) {
        locale = getenv("LANG");
        if (locale == NULL) {
            fprintf(stderr, "Can't guess locale.\n");
            return -1;
        }
    }
    char final_locale[256];
    snprintf(final_locale, 256, "%s,%s", ALL_LOCALES_STRING, locale);

    XIMHandle* hdl = create_xim_server(XIM_NAME, final_locale);
    preedit_set_handle(hdl);
    preedit_reload();

    signal(SIGUSR1, on_app_sig);
    signal(SIGHUP, on_app_sig);
    signal(SIGINT, on_app_sig);
    signal(SIGTERM, on_app_sig);

    gtk_main();
    
    return 0;
}

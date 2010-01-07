/**
 * @file see ``LICENSE`` for further details
 * @author Mike
 */

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <signal.h>

#include "xim.h"
#include "xmisc.h"
#include "ic.h"
#include "settings.h"

#define XIM_NAME "xsunpinyiin"

static void
on_app_exit(int sig)
{
    preedit_finalize();
    icmgr_finalize();

    settings_save();
    settings_destroy();
    
    exit(0);
}

static int
_xerror_handler (Display *dpy, XErrorEvent *e)
{
    if (e->error_code == 0x9E) {
        on_app_exit(SIGINT);
        exit(0);
    }
    g_debug (
        "XError: "
        "serial=%lu error_code=%d request_code=%d minor_code=%d resourceid=%lu",
        e->serial, e->error_code, e->request_code, e->minor_code, e->resourceid);
    return 1;
}

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
    preedit_init();

    settings_init();
    settings_load();

    XIMHandle* hdl = create_xim_server(XIM_NAME, getenv("LANG"));
    preedit_set_handle(hdl);
    preedit_reload_ui();

    signal(SIGHUP, on_app_exit);
    signal(SIGINT, on_app_exit);
    signal(SIGTERM, on_app_exit);

    gtk_main();
    
    return 0;
}

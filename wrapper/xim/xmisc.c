/**
 * @file see ``LICENSE`` for further details
 * @author Mike
 */

#include <gdk/gdkx.h>

#include "xmisc.h"

void
get_window_position(Window w, int* x, int* y)
{
    int tx = 0, ty = 0;
    Window child;
    if (XTranslateCoordinates(dpy, w, DefaultRootWindow(dpy),
                              0, 0,
                              &tx, &ty, &child) < 0) {
        printf("ERROR\n");
        return;
    }
    if (x != NULL)
        (*x) = tx;
    if (y != NULL)
        (*y) = ty;
}

void
get_window_size(Window w, int* width, int* height)
{
    XWindowAttributes attrs;
    if (XGetWindowAttributes(dpy, w, &attrs) < 0) {
        printf("ERROR\n");
    }
    if (width != NULL)
        (*width) = attrs.width;
    if (height != NULL)
        (*height) = attrs.height;
}

void
get_screen_size(int* width, int* height)
{
    get_window_size(DefaultRootWindow(dpy), width, height);
}

Display* dpy = NULL;

void
init_display(int* argc, char*** argv)
{
    gtk_init(argc, argv);
    dpy = GDK_DISPLAY();
}

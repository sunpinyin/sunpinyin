/**
 * @file see ``LICENSE`` for further details
 * @author Mike
 */

#ifndef _XMISC_H_
#define _XMISC_H_

#include <X11/Xproto.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>

__BEGIN_DECLS

void get_window_position(Window w, int* x, int* y);
void get_window_size(Window w, int* width, int* height);
void get_screen_size(int* width, int* height);

void init_display(int* argc, char*** argv);

extern Display* dpy;

__END_DECLS

#endif /* _XMISC_H_ */

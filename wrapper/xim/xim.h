/**
 * @file see ``LICENSE`` for further details
 * @author Mike
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

#include <stdbool.h>

__BEGIN_DECLS

typedef struct _XIMS XIMHandle;

XIMHandle* create_xim_server(const char* server_name, const char* locale);

void      xim_start_preedit (XIMHandle* handle);
void      xim_cancel_preedit(XIMHandle* handle);
void      xim_commit_preedit(XIMHandle* handle, const char* result_str);

/* provided by preedit backend */
void      preedit_init(void);
void      preedit_finalize(void);

void      preedit_set_handle(XIMHandle* handle);
void      preedit_reload_ui(void);

void      preedit_move(int x, int y);
void      preedit_on_key(XIMHandle* handle, unsigned int keycode,
                         unsigned int state);
bool      preedit_status(void);
void      preedit_pause(void);
void      preedit_go_on(void);

__END_DECLS

#endif /* _XIM_H_ */

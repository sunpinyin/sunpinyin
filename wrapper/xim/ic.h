/**
 * @file see ``LICENSE`` for further details
 * @author Mike
 */

#ifndef _IC_H_
#define _IC_H_

#include <stdbool.h>

#include <X11/Xproto.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>

__BEGIN_DECLS

#define MAX_IC_NUM 0x7fff
#define GC_THRESHOLD 4096

/* input context */
typedef struct
{
    Window client_window;
    bool   is_enabled;
    bool   is_english;
    int    icid;
    int    connect_id;
    int    offset_x;
    int    offset_y;
} IC;

/* input context manager */

void icmgr_init       (void);
void icmgr_finalize   (void);

IC*  icmgr_create_ic  (int connect_id);
void icmgr_destroy_ic (int icid);
bool icmgr_set_current(int icid);
IC*  icmgr_get        (int icid);

IC*  icmgr_get_current  (void);
void icmgr_clear_current(void);
void icmgr_refresh_ui   (void);

__END_DECLS

#endif /* _IC_H_ */

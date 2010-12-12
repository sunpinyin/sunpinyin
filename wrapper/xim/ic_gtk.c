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
#include "ic.h"
#include "ui.h"
#include "common.h"
#include "settings.h"

static GtkWidget* icbar_window;
static GtkWidget* icbar_status_btn;
static GtkWidget* icbar_full_btn;
static GtkWidget* icbar_chn_punc_btn;

static GtkStatusIcon* icbar_tray;
static GtkWidget* popup_menu;

#define LOGO_FILE SUNPINYIN_XIM_ICON_DIR"/sunpinyin-logo.png"
#define ENG_ICON_PNG SUNPINYIN_XIM_ICON_DIR"/eng.png"
#define HAN_ICON_PNG SUNPINYIN_XIM_ICON_DIR"/han.png"
#define FULL_ICON_PNG SUNPINYIN_XIM_ICON_DIR"/full.png"
#define HALF_ICON_PNG SUNPINYIN_XIM_ICON_DIR"/half.png"
#define CHN_PUNC_ICON_PNG SUNPINYIN_XIM_ICON_DIR"/chnpunc.png"
#define ENG_PUNC_ICON_PNG SUNPINYIN_XIM_ICON_DIR"/engpunc.png"


static void
toggle_english(GtkToggleButton* btn, gpointer userdata)
{
    icmgr_toggle_english();
    icmgr_refresh();
}

static void
toggle_full(GtkToggleButton* btn, gpointer userdata)
{
    icmgr_toggle_full();
    icmgr_refresh();
}

static void
toggle_chn_punc(GtkToggleButton* btn, gpointer userdata)
{
    icmgr_toggle_punc();
    icmgr_refresh();
}

static bool begin_drag = false;
static int drag_offset_x = -1;
static int drag_offset_y = -1;

static void
icbar_on_button_press(GtkWidget* wnd, GdkEventButton* evt, gpointer data)
{
    if (evt->button == 1) {
        begin_drag = true;
        drag_offset_x = (int) evt->x;
        drag_offset_y = (int) evt->y;
    }
}

static void
icbar_on_button_release(GtkWidget* wnd, GdkEventButton* evt, gpointer data)
{
    if (evt->button == 1) {
        begin_drag = false;
        position_t pos;
        gtk_window_get_position(GTK_WINDOW(icbar_window),
                                &(pos.x), &(pos.y));
        settings_set(ICBAR_POS, &pos);
    }
}

static void
icbar_on_motion(GtkWidget* wnd, GdkEventMotion* evt, gpointer data)
{
    if (begin_drag) {
        gtk_window_move(GTK_WINDOW(icbar_window),
                        (int) evt->x_root - drag_offset_x,
                        (int) evt->y_root - drag_offset_y);
    }
}

static void
__toggle_mode(gboolean mode, GtkWidget* btn,
              const char* img_path1, const char* img_path2)
{
    GtkWidget* img_wid = gtk_tool_button_get_icon_widget(
        GTK_TOOL_BUTTON(btn));
    const char* img_path = NULL;
    
    if (mode) {
        img_path = img_path1;
    } else {
        img_path = img_path2;
    }
    gtk_image_set_from_file(GTK_IMAGE(img_wid), img_path);
    gtk_image_set_pixel_size(GTK_IMAGE(img_wid), 20);
}

static GtkWidget*
__create_btn(bool stock, const char* img_path, GCallback cb)
{
    GtkWidget* img_wid;
    if (stock) {
        img_wid = gtk_image_new_from_icon_name(img_path, GTK_ICON_SIZE_MENU);
    } else {
        img_wid = gtk_image_new_from_file(ENG_ICON_PNG);
    }
    gtk_image_set_pixel_size(GTK_IMAGE(img_wid), 20);
    GtkWidget* res = GTK_WIDGET(gtk_tool_button_new(img_wid, ""));
    g_signal_connect(res, "clicked", cb, NULL);
    return res;
}

static gboolean
icmgr_gtk_init(const char* name)
{
    icbar_window = ui_create_window();
    
    gtk_widget_add_events(icbar_window,
                          GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK
                          | GDK_POINTER_MOTION_MASK);
    g_signal_connect(icbar_window, "button-press-event",
                     G_CALLBACK(icbar_on_button_press), NULL);
    g_signal_connect(icbar_window, "motion-notify-event",
                     G_CALLBACK(icbar_on_motion), NULL);
    g_signal_connect(icbar_window, "button-release-event",
                     G_CALLBACK(icbar_on_button_release), NULL);

    icbar_status_btn = __create_btn(false, ENG_ICON_PNG,
                                    G_CALLBACK(toggle_english));
    icbar_full_btn = __create_btn(false, FULL_ICON_PNG,
                                  G_CALLBACK(toggle_full));
    icbar_chn_punc_btn = __create_btn(false, CHN_PUNC_ICON_PNG,
                                      G_CALLBACK(toggle_chn_punc));
    
    GtkWidget* logo = gtk_image_new_from_file(LOGO_FILE);
    gtk_widget_add_events(logo,
                          GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
    
    GtkWidget* box = gtk_hbox_new(false, 1);

    gtk_box_pack_start(GTK_BOX(box), logo, false, false, 1);
    gtk_box_pack_start(GTK_BOX(box), icbar_status_btn, false, false, 1);
    gtk_box_pack_start(GTK_BOX(box), icbar_full_btn, false, false, 1);
    gtk_box_pack_start(GTK_BOX(box), icbar_chn_punc_btn, false, false, 1);
    
    gtk_container_add(GTK_CONTAINER(icbar_window), box);
    
    gtk_widget_show_all(box);
    return TRUE;
}

static void
icmgr_gtk_refresh(void)
{
    IC* ic = icmgr_get_current();
    if (ic == NULL || !ic->is_enabled) {
        gtk_widget_hide(icbar_window);
        return;
    }

    __toggle_mode(ic->is_english, icbar_status_btn,
                  ENG_ICON_PNG, HAN_ICON_PNG);
    __toggle_mode(ic->is_full, icbar_full_btn,
                  FULL_ICON_PNG, HALF_ICON_PNG);
    __toggle_mode(ic->is_chn_punc, icbar_chn_punc_btn,
                  CHN_PUNC_ICON_PNG, ENG_PUNC_ICON_PNG);    
    
    position_t pos;
    settings_get(ICBAR_POS, &pos);
    gtk_window_move(GTK_WINDOW(icbar_window), pos.x, pos.y);
    gtk_widget_show(icbar_window);
}

static void
icmgr_gtk_dispose(void)
{
    gtk_widget_destroy(icbar_window);
}

static const char*
icmgr_gtk_get_name(void)
{
    return "classic";
}

IC_UI icmgr_gtk = {
    .get_name = icmgr_gtk_get_name,
    .init = icmgr_gtk_init,
    .refresh = icmgr_gtk_refresh,
    .dispose = icmgr_gtk_dispose,
};

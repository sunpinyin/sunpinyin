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
#include "skin.h"
#include "common.h"
#include "settings.h"

static skin_info_t* info;
static const char* skin_name;
static skin_window_t* icbar_wind;
static skin_button_t* icbar_status_btn;
static skin_button_t* icbar_full_btn;
static skin_button_t* icbar_punc_btn;

static void
toggle_english(GtkWidget* wid, GdkEventButton* evt, void* userdata)
{
    icmgr_toggle_english();
    icmgr_refresh();
}

static void
toggle_full(GtkWidget* wid, GdkEventButton* evt, void* userdata)
{
    icmgr_toggle_full();
    icmgr_refresh();
}

static void
toggle_punc(GtkWidget* wid, GdkEventButton* evt, void* userdata)
{
    icmgr_toggle_punc();
    icmgr_refresh();
}

static void
icbar_on_release(GtkWidget* wid, GdkEventMotion* evt, void* userdata)
{
    position_t pos;
    gtk_window_get_position(GTK_WINDOW(wid), &(pos.x), &(pos.y));
    settings_set(ICBAR_POS, &pos);
}

static gboolean
icmgr_skin_init(const char* name)
{
    info = ui_skin_new(name);
    if (!info) return FALSE;
    skin_name = name;
    icbar_wind = skin_window_new(GTK_WINDOW(ui_create_window()),
                                 info->icbar_background,
                                 0, 0, 0, 0, 1);
    skin_window_set_drag_to_move(icbar_wind, TRUE);

    skin_window_set_release_cb(icbar_wind, G_CALLBACK(icbar_on_release), NULL);
    
    /* creating the buttons */
    icbar_status_btn = skin_button_new(info->eng_btn.normal1,
                                       info->eng_btn.highlight1,
                                       info->eng_btn.pressdown1);
    icbar_full_btn = skin_button_new(info->full_btn.normal1,
                                     info->full_btn.highlight1,
                                     info->full_btn.pressdown1);
    icbar_punc_btn = skin_button_new(info->punc_btn.normal1,
                                     info->punc_btn.highlight1,
                                     info->punc_btn.pressdown1);

    /* setting up events */
    skin_button_set_release_cb(icbar_status_btn,
                               G_CALLBACK(toggle_english), NULL);
    skin_button_set_release_cb(icbar_full_btn,
                               G_CALLBACK(toggle_full), NULL);
    skin_button_set_release_cb(icbar_punc_btn,
                               G_CALLBACK(toggle_punc), NULL);
    
    /* adding the buttons to the icbar */
    skin_window_add_button(icbar_wind, icbar_status_btn, info->eng_btn.x,
                           info->eng_btn.y);
    skin_window_add_button(icbar_wind, icbar_full_btn, info->full_btn.x,
                           info->full_btn.y);
    skin_window_add_button(icbar_wind, icbar_punc_btn, info->punc_btn.x,
                           info->punc_btn.y);
    return TRUE;
}

static void
toggle_mode(gboolean mode, skin_button_t* btn, skin_button_info_t* info)
{
    if (mode) {
        skin_button_set_image(btn, info->normal1, info->highlight1,
                              info->pressdown1);
    } else {
        skin_button_set_image(btn, info->normal2, info->highlight2,
                              info->pressdown2);
    }
}

static void
icmgr_skin_refresh(void)
{
    IC* ic = icmgr_get_current();
    if (ic == NULL || !ic->is_enabled) {
        gtk_widget_hide(GTK_WIDGET(icbar_wind->widget));
        return;
    }
    toggle_mode(ic->is_english, icbar_status_btn, &(info->eng_btn));
    toggle_mode(ic->is_full, icbar_full_btn, &(info->full_btn));
    toggle_mode(ic->is_chn_punc, icbar_punc_btn, &(info->punc_btn));

    position_t pos;
    settings_get(ICBAR_POS, &pos);
    gtk_window_move(GTK_WINDOW(icbar_wind->widget), pos.x, pos.y);
    gtk_widget_show(icbar_wind->widget);
}

static void
icmgr_skin_dispose(void)
{
    ui_skin_destroy(info);
    skin_button_destroy(icbar_status_btn);
    skin_button_destroy(icbar_full_btn);
    skin_button_destroy(icbar_punc_btn);
    skin_window_destroy(icbar_wind);   
}

static const char*
icmgr_skin_get_name(void)
{
    return skin_name;
}

IC_UI icmgr_skin = {
    .get_name = icmgr_skin_get_name,
    .init = icmgr_skin_init,
    .refresh = icmgr_skin_refresh,
    .dispose = icmgr_skin_dispose,
};


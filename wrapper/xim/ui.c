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
#include "common.h"
#include "ui.h"

#define ENG_ICON SUNPINYIN_XIM_ICON_DIR"/eng.svg"
#define HAN_ICON SUNPINYIN_XIM_ICON_DIR"/han.svg"
#define LOGO_FILE_BIG SUNPINYIN_XIM_ICON_DIR"/sunpinyin-logo-big.png"

static GtkStatusIcon* icbar_tray;
static GtkWidget* popup_menu;

static void
show_ui_about(GtkWidget* wid, gpointer user_data)
{
    GError* error = NULL;
    GdkPixbuf* logo_pixbuf = 
        gdk_pixbuf_new_from_file(LOGO_FILE_BIG, &error);
    
    gtk_show_about_dialog(NULL,
                          "program-name", XIM_PROGRAM_NAME,
                          "logo", logo_pixbuf,
                          "version", XIM_VERSION,
                          "website", XIM_WEBSITE,
                          "comments", XIM_COMMENTS,
                          NULL);
}

static void
launch_preferences(GtkWidget* wid, gpointer user_data)
{
    system("xsunpinyin-preferences&");
}

static void
status_icon_popup_menu(GtkStatusIcon *status_icon, guint button,
                       guint activate_time, gpointer user_data)
{
    gtk_widget_show_all(popup_menu);
    gtk_menu_popup(GTK_MENU(popup_menu), NULL, NULL,
                   gtk_status_icon_position_menu, status_icon,
                   button, activate_time);
}

void
ui_tray_init(void)
{
    icbar_tray = gtk_status_icon_new_from_file(ENG_ICON);
    GtkWidget* setting_menu_item =
        gtk_image_menu_item_new_from_stock(GTK_STOCK_PREFERENCES, NULL);
    g_signal_connect(setting_menu_item, "activate",
                     G_CALLBACK(launch_preferences), NULL);

    GtkWidget* about_menu_item =
        gtk_image_menu_item_new_from_stock(GTK_STOCK_ABOUT, NULL);
    g_signal_connect(about_menu_item, "activate",
                     G_CALLBACK(show_ui_about), NULL);

    /* construct the popup menu */
    popup_menu = gtk_menu_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(popup_menu), setting_menu_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(popup_menu), about_menu_item);

    g_signal_connect(icbar_tray, "popup-menu",
                     G_CALLBACK(status_icon_popup_menu), NULL);
}

void
ui_tray_refresh(void)
{
    IC* ic = icmgr_get_current();
    const char* filepath = HAN_ICON;
    if (ic == NULL || !ic->is_enabled || ic->is_english) {
        filepath = ENG_ICON;
    }
    gtk_status_icon_set_from_file(icbar_tray, filepath);
}

GtkWidget*
ui_create_window()
{
    GtkWidget* window = gtk_window_new(GTK_WINDOW_POPUP);
    gtk_window_set_decorated(GTK_WINDOW(window), false);
    gtk_window_set_deletable(GTK_WINDOW(window), false);
    gtk_window_set_accept_focus(GTK_WINDOW(window), false);
    gtk_window_set_focus_on_map(GTK_WINDOW(window), false);
    gtk_window_set_keep_above(GTK_WINDOW(window), true);
    gtk_window_set_skip_pager_hint(GTK_WINDOW(window), true);
    gtk_window_set_skip_taskbar_hint(GTK_WINDOW(window), true);
    return window;
}

static GdkPixbuf*
load_pixbuf(const char* skin_name, const char* filename)
{
    char filepath[256];
    snprintf(filepath, 256, "%s/.sunpinyin/xim_skins/%s/%s.png",
             getenv("HOME"), skin_name, filename);
    return gdk_pixbuf_new_from_file(filepath, NULL);
}

#define FILL_PIXBUF(name)                       \
    info->name = load_pixbuf(skin_name, name)

static void
fill_button_pixbuf(skin_button_info_t* info, const char* skin_name,
                   const char* normal1, const char* normal2,
                   const char* highlight1, const char* highlight2,
                   const char* pressdown1, const char* pressdown2)
{
    FILL_PIXBUF(normal1);
    FILL_PIXBUF(normal2);
    FILL_PIXBUF(highlight1);
    FILL_PIXBUF(highlight2);
    FILL_PIXBUF(pressdown1);
    FILL_PIXBUF(pressdown2);
}

static void
fill_label_info(skin_label_info_t* info, FILE* fp)
{
    fscanf(fp, "%d %d\n", &(info->x), &(info->y));
    fgets(info->font, 256, fp);
    /* remove the last \n */
    info->font[strlen(info->font) - 1] = 0;

    fscanf(fp, "%lf %lf %lf %lf\n", &(info->color_r), &(info->color_g),
           &(info->color_b), &(info->color_a));
}

skin_info_t*
ui_skin_new(const char* skin_name)
{
    char filepath[256];
    snprintf(filepath, 256, "%s/.sunpinyin/xim_skins/%s/info",
             getenv("HOME"), skin_name);
    FILE* fp = fopen(filepath, "r");
    if (!fp) {
        fprintf(stderr, "Cannot open skin %s\n", skin_name);
        return NULL;
    }
    skin_info_t* info = malloc(sizeof(skin_info_t));
    fscanf(fp, "%d %d %d %d %d %d\n", &(info->eng_btn.x), &(info->eng_btn.y),
           &(info->full_btn.x), &(info->full_btn.y),
           &(info->punc_btn.x), &(info->punc_btn.y));

    fill_label_info(&(info->preedit_label), fp);
    fill_label_info(&(info->candidate_label), fp);

    fscanf(fp, "%d %d\n", &(info->offset_x), &(info->offset_y));
    fscanf(fp, "%d %d %d %d\n", &(info->top), &(info->left), &(info->bottom),
           &(info->right));
    
    fclose(fp);

    fill_button_pixbuf(&(info->eng_btn), skin_name, "eng", "han",
                       "eng-hover", "han-hover", "eng-press", "han-press");
    fill_button_pixbuf(&(info->full_btn), skin_name, "full", "half",
                       "full-hover", "half-hover", "full-press", "half-press");
    fill_button_pixbuf(&(info->punc_btn), skin_name, "han-punc", "eng-punc",
                       "han-punc-hover", "eng-punc-hover",
                       "han-punc-press", "eng-punc-press");
    info->icbar_background = load_pixbuf(skin_name, "icbar");
    info->preedit_background = load_pixbuf(skin_name, "preedit");
    return info;
}

static void
free_button_info(skin_button_info_t* info)
{
    g_object_unref(info->normal1);
    g_object_unref(info->highlight1);
    g_object_unref(info->pressdown1);
    g_object_unref(info->normal2);
    g_object_unref(info->highlight2);
    g_object_unref(info->pressdown2);
}

void
ui_skin_destroy(skin_info_t* info)
{
    g_object_unref(info->icbar_background);
    g_object_unref(info->preedit_background);

    free_button_info(&(info->eng_btn));
    free_button_info(&(info->full_btn));
    free_button_info(&(info->punc_btn));
}

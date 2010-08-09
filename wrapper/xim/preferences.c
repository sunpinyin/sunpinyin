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

#include <X11/X.h>
#include <X11/Xproto.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>

#include <unistd.h>
#include <sys/types.h>

#include "settings.h"
#include "xmisc.h"

#define UI_FILE SUNPINYIN_XIM_SETTING_DIR"/settings_ui.xml"

static GtkWidget* main_wnd = NULL;
static GtkToggleButton* trigger_ctrl_check = NULL;
static GtkToggleButton* english_ctrl_check = NULL;
static GtkToggleButton* trigger_shift_check = NULL;
static GtkToggleButton* english_shift_check = NULL;
static GtkComboBox* trigger_combo = NULL;
static GtkComboBox* english_combo = NULL;
static GtkColorButton* background_color_btn = NULL;
static GtkFontButton* font_btn = NULL;
static GtkColorButton* font_color_btn = NULL;
static GtkAdjustment* opacity_value = NULL;
static GtkAdjustment* ncandidates = NULL;
static GtkToggleButton* minus_plus_check = NULL;
static GtkToggleButton* comma_period_check = NULL;
static GtkToggleButton* paren_check = NULL;
static GtkToggleButton* fuzzy_seg_check = NULL;
static GtkToggleButton* fuzzy_inner_seg_check = NULL;
static GtkToggleButton* cancel_on_backspace_check = NULL;
static GtkToggleButton* smart_punct_check = NULL;
static GtkToggleButton* shuangpin_check = NULL;
static GtkComboBox* shuangpin_combo = NULL;

#define RETRIEVE(name, macro)                                   \
    name = macro(gtk_builder_get_object(builder, # name))

static const char* ui_shuangpin_schemes[] =
{
    "MS2003", "ABC", "ZiRanMa", "PinYin++", "ZiGuang", "XiaoHe",
};

#define UI_SHUANGPIN_SCHEMES_NUM 6

static const int ui_keysym_model[] =
{
    XK_space, XK_Control_L, XK_Control_R, XK_Shift_L, XK_Shift_R
};

#define UI_KEYSYM_MODEL_NUM 5

#define INIT_KEY_SETTING(prefix)                                        \
    do {                                                                \
        if (hk.modifiers & ControlMask) {                               \
            gtk_toggle_button_set_active(prefix##_ctrl_check, TRUE);    \
        }                                                               \
        if (hk.modifiers & ShiftMask)                                   \
            gtk_toggle_button_set_active(prefix##_shift_check, TRUE);   \
        int i;                                                          \
        for (i = 0; i < UI_KEYSYM_MODEL_NUM; i++) {                     \
            if (hk.keysym == ui_keysym_model[i]) {                      \
                gtk_combo_box_set_active(prefix##_combo, i);            \
                break;                                                  \
            }                                                           \
        }                                                               \
    } while(0)

#define INIT_COLOR_SETTING(widget_name)                         \
    do {                                                        \
        GdkColor color;                                         \
        gdk_color_parse(colorstr, &color);                      \
        gtk_color_button_set_color((widget_name), &color);      \
    } while(0)

static void
init_settings(void)
{
    settings_init();
    settings_load();
    
    hotkey_t hk;
    settings_get(TRIGGER_KEY, &hk);
    INIT_KEY_SETTING(trigger);

    settings_get(ENG_KEY, &hk);
    INIT_KEY_SETTING(english);

    varchar colorstr;
    settings_get(PREEDIT_COLOR, colorstr);
    INIT_COLOR_SETTING(background_color_btn);

    settings_get(PREEDIT_FONT_COLOR, colorstr);
    INIT_COLOR_SETTING(font_color_btn);

    varchar fontstr;
    settings_get(PREEDIT_FONT, fontstr);
    gtk_font_button_set_font_name(font_btn, fontstr);

    
    gtk_adjustment_set_value(opacity_value,
                             settings_get_double(PREEDIT_OPACITY));
    
    gtk_adjustment_set_value(ncandidates, settings_get_int(CANDIDATES_SIZE));

    gtk_toggle_button_set_active(minus_plus_check,
                                 settings_get_int(PAGE_MINUS_PLUS));
    gtk_toggle_button_set_active(comma_period_check,
                                 settings_get_int(PAGE_COMMA_PERIOD));
    gtk_toggle_button_set_active(paren_check,
                                 settings_get_int(PAGE_PAREN));
    
    gtk_toggle_button_set_active(fuzzy_seg_check,
                                 settings_get_int(FUZZY_SEGMENTATION));
    gtk_toggle_button_set_active(fuzzy_inner_seg_check,
                                 settings_get_int(FUZZY_INNER_SEGMENTATION));

    gtk_toggle_button_set_active(cancel_on_backspace_check,
                                 settings_get_int(CANCEL_ON_BACKSPACE));

    gtk_toggle_button_set_active(smart_punct_check,
                                 settings_get_int(SMART_PUNCT));

    fprintf(stderr, "%d\n", settings_get_int(SHUANGPIN));
    gtk_toggle_button_set_active(shuangpin_check,
                                 settings_get_int(SHUANGPIN));
    varchar scheme;
    int i;
    settings_get(SHUANGPIN_SCHEME, scheme);
    for (i = 0; i < UI_SHUANGPIN_SCHEMES_NUM; i++) {
        if (strcmp(ui_shuangpin_schemes[i], scheme) == 0) {
            gtk_combo_box_set_active(shuangpin_combo, i);
            break;
        }
    }

}

static void
init(void)
{
    GtkBuilder* builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, UI_FILE, NULL);
    main_wnd = GTK_WIDGET(gtk_builder_get_object(builder, "settings_dialog"));

    RETRIEVE(trigger_ctrl_check, GTK_TOGGLE_BUTTON);
    RETRIEVE(english_ctrl_check, GTK_TOGGLE_BUTTON);
    RETRIEVE(trigger_shift_check, GTK_TOGGLE_BUTTON);
    RETRIEVE(english_shift_check, GTK_TOGGLE_BUTTON);
    RETRIEVE(trigger_combo, GTK_COMBO_BOX);
    RETRIEVE(english_combo, GTK_COMBO_BOX);
    RETRIEVE(background_color_btn, GTK_COLOR_BUTTON);
    RETRIEVE(font_btn, GTK_FONT_BUTTON);
    RETRIEVE(font_color_btn, GTK_COLOR_BUTTON);
    RETRIEVE(opacity_value, GTK_ADJUSTMENT);
    RETRIEVE(ncandidates, GTK_ADJUSTMENT);
    RETRIEVE(minus_plus_check, GTK_TOGGLE_BUTTON);
    RETRIEVE(comma_period_check, GTK_TOGGLE_BUTTON);
    RETRIEVE(paren_check, GTK_TOGGLE_BUTTON);
    RETRIEVE(fuzzy_seg_check, GTK_TOGGLE_BUTTON);
    RETRIEVE(fuzzy_inner_seg_check, GTK_TOGGLE_BUTTON);
    RETRIEVE(cancel_on_backspace_check, GTK_TOGGLE_BUTTON);
    RETRIEVE(smart_punct_check, GTK_TOGGLE_BUTTON);
    RETRIEVE(shuangpin_check, GTK_TOGGLE_BUTTON);
    RETRIEVE(shuangpin_combo, GTK_COMBO_BOX);

    init_settings();
    
    gtk_builder_connect_signals(builder, NULL);
    
    g_object_unref(builder);
}

#define REFRESH_KEY_SETTING(prefix)                                     \
    do {                                                                \
        hk.modifiers = hk.keysym = 0;                                   \
        if (gtk_toggle_button_get_active(prefix##_ctrl_check)) {        \
            hk.modifiers |= ControlMask;                                \
        }                                                               \
        if (gtk_toggle_button_get_active(prefix##_shift_check)) {       \
            hk.modifiers |= ShiftMask;                                  \
        }                                                               \
        int idx = gtk_combo_box_get_active(prefix##_combo);             \
        if (idx >= 0)                                                   \
            hk.keysym = ui_keysym_model[idx];                           \
    } while (0)

#define REFRESH_COLOR_SETTING(widget_name)                              \
    do {                                                                \
        GdkColor color;                                                 \
        gtk_color_button_get_color((widget_name), &color);              \
        snprintf(colorstr, sizeof(varchar), "#%.2X%.2X%.2X",            \
                 color.red >> 8, color.green >> 8, color.blue >> 8);    \
    } while(0)

static void
send_reload()
{
    /* notify all running xsunpinyin with this user */
    char cmd[256];
    snprintf(cmd, 256, "/usr/bin/pkill -10 '^xsunpinyin$' -u %d", getuid());
    system(cmd);
}

void
state_changed()
{
    hotkey_t hk;
    REFRESH_KEY_SETTING(trigger);
    settings_set(TRIGGER_KEY, &hk);

    REFRESH_KEY_SETTING(english);
    settings_set(ENG_KEY, &hk);

    varchar colorstr;
    REFRESH_COLOR_SETTING(background_color_btn);
    settings_set(PREEDIT_COLOR, colorstr);

    REFRESH_COLOR_SETTING(font_color_btn);
    settings_set(PREEDIT_FONT_COLOR, colorstr);

    /* font and size information */
    settings_set(PREEDIT_FONT, (void*) gtk_font_button_get_font_name(font_btn));

    /* font color information */
    settings_set_double(PREEDIT_OPACITY,
                        gtk_adjustment_get_value(opacity_value));

    settings_set_int(CANDIDATES_SIZE,
                     gtk_adjustment_get_value(ncandidates));

    /* page up and down trigger */
    settings_set_int(PAGE_MINUS_PLUS,
                     gtk_toggle_button_get_active(minus_plus_check));
    settings_set_int(PAGE_COMMA_PERIOD,
                     gtk_toggle_button_get_active(comma_period_check));
    settings_set_int(PAGE_PAREN,
                     gtk_toggle_button_get_active(paren_check));

    /* fuzzy segmentation */
    settings_set_int(FUZZY_SEGMENTATION,
                     gtk_toggle_button_get_active(fuzzy_seg_check));
    settings_set_int(FUZZY_INNER_SEGMENTATION,
                     gtk_toggle_button_get_active(fuzzy_inner_seg_check));

    /* cancel on backspace */
    settings_set_int(CANCEL_ON_BACKSPACE,
                     gtk_toggle_button_get_active(cancel_on_backspace_check));

    /* smart punctuation */
    settings_set_int(SMART_PUNCT,
                     gtk_toggle_button_get_active(smart_punct_check));

    settings_set_int(SHUANGPIN, gtk_toggle_button_get_active(shuangpin_check));
    int sche_idx = gtk_combo_box_get_active(shuangpin_combo);
    if (sche_idx < UI_SHUANGPIN_SCHEMES_NUM)
        settings_set_string(SHUANGPIN_SCHEME, ui_shuangpin_schemes[sche_idx]);

    settings_save();
    send_reload();
}

int main(int argc, char *argv[])
{
    init_display(&argc, &argv);
    init();
    
    gtk_widget_show(main_wnd);
    
    gtk_main();
    return 0;
}

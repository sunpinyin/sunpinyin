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

#define UI_FILE SUNPINYIN_XIM_DATA_DIR"/settings_ui.xml"

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

#define RETRIEVE(name, macro)                                   \
    name = macro(gtk_builder_get_object(builder, # name))

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

    double scale;
    settings_get(PREEDIT_OPACITY, &scale);
    gtk_adjustment_set_value(opacity_value, scale);
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
        int idx = gtk_combo_box_get_active(prefix##_combo);            \
        if (idx >= 0)                                                   \
            hk.keysym = ui_keysym_model[idx];                           \
    } while (0)

#define REFRESH_COLOR_SETTING(widget_name)                              \
    do {                                                                \
        GdkColor color;                                                 \
        gtk_color_button_get_color((widget_name), &color);              \
        snprintf(colorstr, sizeof(varchar), "#%.2X%.2X%.2X",               \
                 (guint8) color.red, (guint8) color.green,              \
                 (guint8) color.blue);                                  \
    } while(0)

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
    double scale = gtk_adjustment_get_value(opacity_value);
    settings_set(PREEDIT_OPACITY, &scale);
    
    settings_save();

    /* notify all running xsunpinyin with this user */
    char cmd[256];
    snprintf(cmd, 256, "/usr/bin/pkill -10 '^xsunpinyin$' -u %d", getuid());
    system(cmd);
}

int main(int argc, char *argv[])
{
    init_display(&argc, &argv);
    init();
    
    gtk_widget_show(main_wnd);
    
    gtk_main();
    return 0;
}


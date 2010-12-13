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

#include <stdlib.h>
#include <string.h>
#include "skin.h"

struct _skin_window_priv_t
{
    skin_label_t* labels;
    skin_button_t* btns;

    /* current expose environment */
    skin_button_t* highlight_btn;
    skin_button_t* pressdown_btn;

    GCallback      press_cb;
    GCallback      release_cb;
    GCallback      motion_cb;

    void*          press_cb_data;
    void*          release_cb_data;
    void*          motion_cb_data;

    /* drag to move positions */
    gboolean       enable_drag_to_move;
    int            drag_init_x;
    int            drag_init_y;
    gboolean       drag_begin;
};

struct _skin_button_priv_t
{
    skin_button_t* next;
    skin_window_t* parent;

    GCallback press_cb;
    GCallback release_cb;

    void*     press_cb_data;
    void*     release_cb_data;
};

struct _skin_label_priv_t
{
    skin_label_t*  next;
    skin_window_t* wind;
};

static void
zoom_and_composite(GdkPixbuf* src,
                   GdkPixbuf* dst,
                   int        x,
                   int        y,
                   int        width,
                   int        height,
                   int        dst_x,
                   int        dst_y,
                   int        dst_width,
                   int        dst_height)
{
    if (width == 0 || height == 0)
        return;
    GdkPixbuf* sub = gdk_pixbuf_new_subpixbuf(src, x, y, width, height);
    double x_scale = 1.0 * dst_width / width;
    double y_scale = 1.0 * dst_height / height;
    
    gdk_pixbuf_scale(sub, dst, dst_x, dst_y, dst_width, dst_height,
                     dst_x, dst_y, x_scale, y_scale, GDK_INTERP_BILINEAR);
    g_object_unref(sub);
}

static void
cairo_paint_pixbuf(cairo_t*   cr,
                   GdkPixbuf* buf,
                   int        off_x,
                   int        off_y)
{
    gdk_cairo_set_source_pixbuf(cr, buf, off_x, off_y);
    cairo_paint(cr);
}

static void
paint_background_with_mask(GtkWidget*      wid,
                           skin_window_t*  wind)
{
    GdkPixbuf* bg = wind->background_image;
    int width, height;
    int bg_width, bg_height;
    int top, left, bottom, right;
    
    width = wid->allocation.width;
    height = wid->allocation.height;
    bg_width = gdk_pixbuf_get_width(bg);
    bg_height = gdk_pixbuf_get_height(bg);
    
    top = wind->margin_top;
    left = wind->margin_left;
    right = wind->margin_right;
    bottom = wind->margin_bottom;

    GdkPixbuf* newbg = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8,
                                       width, height);

    // 4 corners
    gdk_pixbuf_copy_area(bg, 0, 0, left, top, newbg, 0, 0);
    gdk_pixbuf_copy_area(bg, bg_width - right, 0, right, top,
                         newbg, width - right, 0);
    gdk_pixbuf_copy_area(bg, 0, bg_height - bottom, left, bottom,
                         newbg, 0, height - bottom);
    gdk_pixbuf_copy_area(bg, bg_width - right, bg_height - bottom,
                         right, bottom,
                         newbg, width - right, height - bottom);

    // 4 margins and the center area
    int content_width = bg_width - left - right;
    int content_height = bg_height - top - bottom;
    int new_content_width = width - left - right;
    int new_content_height = height - top - bottom;

    // top
    zoom_and_composite(bg, newbg, left, 0, content_width, top,
                       left, 0, new_content_width, top);
    // bottom
    zoom_and_composite(bg, newbg,
                       left, bg_height - bottom, content_width, bottom,
                       left, height - bottom, new_content_width, bottom);
    // left
    zoom_and_composite(bg, newbg,
                       0, top, left, content_height,
                       0, top, left, new_content_height);
    // right
    zoom_and_composite(bg, newbg,
                       bg_width - right, top, right, content_height,
                       width - right, top, right, new_content_height);
    // center
    zoom_and_composite(bg, newbg,
                       left, top, content_width, content_height,
                       left, top, new_content_width, new_content_height);

    gdk_window_clear(wid->window);
    // paint it
    cairo_t* cr = gdk_cairo_create(wid->window);
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.0);
    cairo_paint(cr);
    cairo_paint_pixbuf(cr, newbg, 0, 0);
    cairo_destroy(cr);
    // set the mask
    GdkBitmap* mask;
    gdk_pixbuf_render_pixmap_and_mask(newbg, NULL, &mask,
                                      wind->alpha_mask_threshold);
    gdk_window_shape_combine_mask(wid->window, mask, 0, 0);
    
    g_object_unref(newbg);    
}

static void
paint_buttons(GtkWidget*     wid,
              skin_window_t* wind)
{
    cairo_t* cr = gdk_cairo_create(wid->window);
    skin_button_t* btn = wind->priv->btns;
    for (; btn != NULL; btn = btn->priv->next) {
        if (btn == wind->priv->pressdown_btn) {
            cairo_paint_pixbuf(cr, btn->pressdown_image, btn->x, btn->y);
        } else if (btn == wind->priv->highlight_btn) {
            cairo_paint_pixbuf(cr, btn->highlight_image, btn->x, btn->y);
        } else {
            cairo_paint_pixbuf(cr, btn->normal_image, btn->x, btn->y);
        }
    }
    cairo_destroy(cr);
}

static void
paint_labels(GtkWidget*     wid,
             skin_window_t* wind)
{
    cairo_t* cr = gdk_cairo_create(wid->window);
    skin_label_t* label = wind->priv->labels;
    for (; label != NULL; label = label->priv->next) {
        cairo_set_source_rgba(cr, label->color_r, label->color_g,
                              label->color_b, label->color_a);
        if (label->layout == NULL) {
            label->layout = pango_cairo_create_layout(cr);
            pango_layout_set_font_description(
                label->layout, pango_font_description_from_string(label->font));
        }
        pango_layout_set_text(label->layout, label->text, -1);
        cairo_move_to(cr, label->x, label->y);
        pango_cairo_layout_path(cr, label->layout);
        cairo_fill(cr);
    }
    cairo_destroy(cr);
}

static gboolean
window_on_expose(GtkWidget*      wid,
                 GdkEventExpose* evt,
                 skin_window_t*  wind)
{
    paint_background_with_mask(wid, wind);
    paint_labels(wid, wind);
    paint_buttons(wid, wind);
    return TRUE;
}

static gboolean
window_on_configure(GtkWidget*         wid,
                    GdkEventConfigure* evt,
                    skin_window_t*     wind)
{
    gtk_widget_queue_draw(wid);
    return FALSE;
}

static skin_button_t*
find_button(skin_window_t* wind, int pos_x, int pos_y)
{
    skin_button_t* btn = wind->priv->btns;
    for (; btn != NULL; btn = btn->priv->next) {
        int x = btn->x, y = btn->y, width = btn->width, height = btn->height;
        if (pos_x > x && pos_y > y && pos_x < x + width && pos_y < y + height) {
            return btn;
        }
    }
    return NULL;
}

static gboolean
set_expose_env(skin_window_t* wind,
               skin_button_t* highlight,
               skin_button_t* down)
{
    gboolean ret = FALSE;
    if (wind->priv->highlight_btn != highlight) {
        ret |= TRUE;
        wind->priv->highlight_btn = highlight;
    }
    if (wind->priv->pressdown_btn != down) {
        ret |= TRUE;
        wind->priv->pressdown_btn = down;
    }
    return ret;
}

typedef gboolean (*motion_cb_t) (GtkWidget*, GdkEventMotion*, void*);
typedef gboolean (*mouse_cb_t) (GtkWidget*, GdkEventButton*, void*);

static gboolean
window_on_motion(GtkWidget*      wid,
                 GdkEventMotion* evt,
                 skin_window_t*  wind)
{
    skin_button_t* btn = find_button(wind, evt->x, evt->y);
    gboolean need_redraw = FALSE;
    if (btn) {
        if ((evt->state & GDK_BUTTON1_MASK) != 0
            || (evt->state & GDK_BUTTON2_MASK) != 0
            || (evt->state & GDK_BUTTON3_MASK) != 0) {
            need_redraw = set_expose_env(wind, NULL, btn);
        } else {
            need_redraw = set_expose_env(wind, btn, NULL);
        }
    } else {
        /* move it if drag */
        if (wind->priv->enable_drag_to_move && wind->priv->drag_begin) {
            gtk_window_move(GTK_WINDOW(wind->widget),
                            evt->x_root - wind->priv->drag_init_x,
                            evt->y_root - wind->priv->drag_init_y);
        }
        need_redraw = set_expose_env(wind, NULL, NULL);
        motion_cb_t cb = (motion_cb_t) wind->priv->motion_cb;
        if (cb) {
            cb(wid, evt, wind->priv->motion_cb_data);
        }
    }
    if (need_redraw) {
        gtk_widget_queue_draw(wid);
    }
    return TRUE;
}

static gboolean
window_on_press_or_release(GtkWidget*      wid,
                           GdkEventButton* evt,
                           skin_window_t*  wind,
                           gboolean        press)
{
    skin_button_t* btn = find_button(wind, evt->x, evt->y);
    gboolean need_redraw = FALSE;
    skin_button_t* highlight_btn = NULL;
    skin_button_t* pressdown_btn = NULL;
    mouse_cb_t btncb = NULL;
    mouse_cb_t wincb = NULL;
    
    if (press) {
        wincb = (mouse_cb_t) wind->priv->press_cb;
        pressdown_btn = btn;
        if (!btn) {
            /* update drag init positions */
            wind->priv->drag_init_x = evt->x;
            wind->priv->drag_init_y = evt->y;
            wind->priv->drag_begin = wind->priv->enable_drag_to_move;
        } else {
            btncb = (mouse_cb_t) btn->priv->press_cb;
        }
    } else {
        wincb = (mouse_cb_t) wind->priv->release_cb;
        highlight_btn = btn;
        if (!btn) {
            wind->priv->drag_begin = FALSE;
        } else {
            btncb = (mouse_cb_t) btn->priv->release_cb;
        }
    }
    
    if (btn) {
        need_redraw = set_expose_env(wind, highlight_btn, pressdown_btn);
        if (btncb) {
            btncb(wid, evt, btn->priv->press_cb_data);
        }
    } else {
        if (wincb) {
            wincb(wid, evt, wind->priv->press_cb_data);
        }
    }
    if (need_redraw) {
        gtk_widget_queue_draw(wid);
    }
    return TRUE;
}

static gboolean
window_on_press(GtkWidget*      wid,
                GdkEventButton* evt,
                skin_window_t*  wind)
{
    return window_on_press_or_release(wid, evt, wind, TRUE);
}

static gboolean
window_on_release(GtkWidget* wid,
                  GdkEventButton* evt,
                  skin_window_t* wind)
{
    return window_on_press_or_release(wid, evt, wind, FALSE);
}

skin_window_t*
skin_window_new(GtkWindow* widget,
                GdkPixbuf* background_image,
                int        margin_top,
                int        margin_left,
                int        margin_bottom,
                int        margin_right,
                int        alpha_mask_threshold)
{
    skin_window_t* wind = malloc(sizeof(skin_window_t));
    wind->widget = GTK_WIDGET(widget);
    wind->background_image = background_image;
    wind->margin_top = margin_top;
    wind->margin_left = margin_left;
    wind->margin_bottom = margin_bottom;
    wind->margin_right = margin_right;
    wind->alpha_mask_threshold = alpha_mask_threshold;
    wind->priv = malloc(sizeof(skin_window_priv_t));
    memset(wind->priv, 0, sizeof(skin_window_priv_t));
    gtk_window_set_default_size(widget,
                                gdk_pixbuf_get_width(background_image),
                                gdk_pixbuf_get_height(background_image));
    
    /* set rgba */
    GdkScreen* screen = gdk_screen_get_default();
    GdkColormap* cmap = gdk_screen_get_rgba_colormap(screen);
    if (cmap) {
        gtk_widget_set_colormap(wind->widget, cmap);
    } else {
        fprintf(stderr, "Cannot set rgba colormap!\n");
    }
    
    /* signal expose */
    gtk_widget_set_events(wind->widget,
                          GDK_EXPOSURE_MASK | GDK_POINTER_MOTION_MASK
                          | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
    gtk_widget_realize(wind->widget);
    
    g_signal_connect(wind->widget, "expose-event",
                     G_CALLBACK(window_on_expose), wind);
    g_signal_connect(wind->widget, "configure-event",
                     G_CALLBACK(window_on_configure), wind);
    g_signal_connect(wind->widget, "motion-notify-event",
                     G_CALLBACK(window_on_motion), wind);
    g_signal_connect(wind->widget, "button-press-event",
                     G_CALLBACK(window_on_press), wind);
    g_signal_connect(wind->widget, "button-release-event",
                     G_CALLBACK(window_on_release), wind);
    return wind;
}

void
skin_window_destroy(skin_window_t* wind)
{
    gtk_widget_destroy(wind->widget);
    free(wind->priv);
    free(wind);
}

void
skin_window_add_button(skin_window_t* wind,
                       skin_button_t* btn,
                       int            x,
                       int            y)
{
    /* append to the buttons list */
    btn->priv->next = wind->priv->btns;
    wind->priv->btns = btn;

    btn->priv->parent = wind;
    btn->x = x;
    btn->y = y;
}

void
skin_window_set_drag_to_move(skin_window_t* wind,
                             gboolean       drag_to_move)
{
    wind->priv->enable_drag_to_move = drag_to_move;
}

skin_button_t*
skin_button_new(GdkPixbuf* normal_image,
                GdkPixbuf* highlight_image,
                GdkPixbuf* pressdown_image)
{
    skin_button_t* btn = malloc(sizeof(skin_button_t));
    btn->normal_image = normal_image;
    btn->highlight_image = highlight_image;
    btn->pressdown_image = pressdown_image;
    btn->width = gdk_pixbuf_get_width(normal_image);
    btn->height = gdk_pixbuf_get_height(normal_image);
    btn->priv = malloc(sizeof(skin_button_priv_t));
    memset(btn->priv, 0, sizeof(skin_button_priv_t));
    return btn;
}

void
skin_button_destroy(skin_button_t* btn)
{
    free(btn->priv);
    free(btn);
}

void
skin_button_set_image(skin_button_t* btn,
                      GdkPixbuf* normal_image,
                      GdkPixbuf* highlight_image,
                      GdkPixbuf* pressdown_image)
{
    /* since we're redraw it, we'd better check if we really changed
       the pixbuf. This can save a lot of time.
    */
    gboolean need_set = (btn->normal_image != normal_image) ||
        (btn->highlight_image != highlight_image) ||
        (btn->pressdown_image != pressdown_image);
    
    if (!need_set)
        return;
    
    btn->normal_image = normal_image;
    btn->highlight_image = highlight_image;
    btn->pressdown_image = pressdown_image;
    if (btn->priv->parent) {
        gtk_widget_queue_draw(btn->priv->parent->widget);
    }
}

skin_label_t*
skin_label_new(char*  font,
               char*  text,
               double color_r,
               double color_g,
               double color_b,
               double color_a)
{
    skin_label_t* label = malloc(sizeof(skin_label_t));
    label->color_r = color_r;
    label->color_g = color_g;
    label->color_b = color_b;
    label->color_a = color_a;
    label->font = strdup(font);
    if (text) {
        label->text = strdup(text);
    } else {
        label->text = NULL;
    }
    label->layout = NULL;
    label->priv = malloc(sizeof(skin_label_priv_t));
    memset(label->priv, 0, sizeof(skin_label_priv_t));
    
    return label;
}

void
skin_label_destroy(skin_label_t* label)
{
    free(label->text);
    free(label->font);
    free(label->priv);
    if (label->layout) {
        g_object_unref(label->layout);
    }
    free(label);
}

void
skin_window_add_label(skin_window_t* wind,
                      skin_label_t*  label,
                      int            x,
                      int            y)
{
    label->x = x;
    label->y = y;
    label->priv->wind = wind;
    label->priv->next = wind->priv->labels;
    wind->priv->labels = label;

    cairo_t* cr = gdk_cairo_create(wind->widget->window);
    label->layout = pango_cairo_create_layout(cr);
    pango_layout_set_font_description(
        label->layout, pango_font_description_from_string(label->font));
    cairo_destroy(cr);
}

void
skin_label_set_text(skin_label_t* label, const char* text)
{
    free(label->text);
    if (text) {
        label->text = strdup(text);
        pango_layout_set_text(label->layout, label->text, -1);
    } else {
        label->text = NULL;
    }
    if (label->layout && label->priv->wind) {
        gtk_widget_queue_draw(label->priv->wind->widget);
    }
}

#define SET_CB_IMPL(name, event)                                        \
    void skin_##name##_set_##event##_cb(skin_##name##_t* wid,           \
                                        GCallback        cb,            \
                                        void*            data)          \
    { wid->priv->event##_cb = cb; wid->priv->event##_cb_data = data; }  \


SET_CB_IMPL(window, press);
SET_CB_IMPL(window, release);
SET_CB_IMPL(window, motion);
SET_CB_IMPL(button, press);
SET_CB_IMPL(button, release);

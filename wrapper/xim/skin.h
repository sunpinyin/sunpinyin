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

#ifndef _SKIN_H_
#define _SKIN_H_

#include <gtk/gtk.h>
#include <cairo.h>
#include "common.h"

__BEGIN_DECLS

typedef struct _skin_button_priv_t skin_button_priv_t;
typedef struct _skin_button_t skin_button_t;

typedef struct _skin_label_priv_t skin_label_priv_t;
typedef struct _skin_label_t skin_label_t;

typedef struct _skin_window_priv_t skin_window_priv_t;
typedef struct _skin_window_t skin_window_t;

struct _skin_button_t
{
    int x, y;
    int width, height;

    GdkPixbuf* normal_image;
    GdkPixbuf* highlight_image;
    GdkPixbuf* pressdown_image;

    skin_button_priv_t* priv;
};

struct _skin_label_t
{
    int x, y;
    double color_r, color_g, color_b, color_a;
    char* font;
    char* text;

    PangoLayout* layout;
    
    skin_label_priv_t* priv;
};

struct _skin_window_t
{
    GtkWidget* widget;
    GdkPixbuf* background_image;
    int        margin_top;
    int        margin_left;
    int        margin_bottom;
    int        margin_right;
    int        alpha_mask_threshold;

    skin_window_priv_t* priv;
    
};

skin_window_t*   skin_window_new(GtkWindow* window,
                                 GdkPixbuf* background_image,
                                 int        margin_top,
                                 int        margin_left,
                                 int        margin_bottom,
                                 int        margin_right,
                                 int        alpha_mask_threshold);

void             skin_window_destroy(skin_window_t* wind);

void             skin_window_set_drag_to_move(skin_window_t* wind,
                                              gboolean       drag_to_move);

void             skin_window_add_button(skin_window_t* wind,
                                        skin_button_t* btn,
                                        int            x,
                                        int            y);

void             skin_window_add_label(skin_window_t* wind,
                                       skin_label_t*  label,
                                       int            x,
                                       int            y);

void             skin_window_set_motion_cb(skin_window_t*  wind,
                                           GCallback       cb,
                                           void*           data);

void             skin_window_set_press_cb(skin_window_t*   wind,
                                          GCallback        cb,
                                          void*            data);

void             skin_window_set_release_cb(skin_window_t* wind,
                                            GCallback      cb,
                                            void*          data);

skin_button_t*   skin_button_new(GdkPixbuf* normal_image,
                                 GdkPixbuf* highlight_image,
                                 GdkPixbuf* down_image);

void             skin_button_destroy(skin_button_t* btn);

void             skin_button_set_image(skin_button_t* btn,
                                       GdkPixbuf*     normal_image,
                                       GdkPixbuf*     highlight_image,
                                       GdkPixbuf*     pressdown_image);

void             skin_button_set_press_cb(skin_button_t*   wind,
                                          GCallback        cb,
                                          void*            data);

void             skin_button_set_release_cb(skin_button_t* wind,
                                            GCallback      cb,
                                            void*          data);

skin_label_t*    skin_label_new(char*  font,
                                char*  text,
                                double color_r,
                                double color_g,
                                double color_b,
                                double color_a);

void             skin_label_destroy(skin_label_t* label);

void             skin_label_set_text(skin_label_t* label, const char* text);

__END_DECLS

#endif /* _SKIN_H_ */

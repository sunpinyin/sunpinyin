/* -*- mode: c++ -*- */
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

/**
 * @file a ui layer for sunpinyin, this provides a gtk implementation
 * for sunpinyin wrapper SSWindowHandler
 * @author Mike
 */

#ifndef _SUNPINYIN_UI_GTK_H_
#define _SUNPINYIN_UI_GTK_H_

#include "imi_view.h"
#include "imi_uiobjects.h"
#include "imi_winHandler.h"
#include "imi_options.h"

#include "xmisc.h"
#include "settings.h"

#include <gtk/gtk.h>

class GtkProvider {
    GtkWidget* main_wnd_;
    GtkWidget* preedit_area_;
    GtkWidget* candidate_area_;

    bool status_;
    bool pause_;
public:
    GtkProvider() {
        main_wnd_ = gtk_window_new(GTK_WINDOW_POPUP);
        GtkWidget* box = gtk_vbox_new(FALSE, 1);
        gtk_container_add(GTK_CONTAINER(main_wnd_), box);

        preedit_area_ = gtk_label_new("");
        candidate_area_ = gtk_label_new("");

        gtk_misc_set_alignment(GTK_MISC(preedit_area_), 0, 0.5);
        gtk_misc_set_alignment(GTK_MISC(candidate_area_), 0, 0.5);

        gtk_box_pack_start(GTK_BOX(box), preedit_area_,
                           FALSE, FALSE, 1);
        gtk_box_pack_start(GTK_BOX(box), candidate_area_,
                           FALSE, FALSE, 1);
        gtk_widget_show_all(box);
        gtk_widget_realize(main_wnd_);
        
        // get screen size
        get_screen_size(&screen_width_, &screen_height_);
        LOG("screen size %d %d", screen_width_, screen_height_);
        width_ = 0;
        height_ = 0;
    }

    void reload_ui() {
        GdkColor color;
        varchar value;
        double opa = 1.0;
        
        settings_get(PREEDIT_COLOR, value);
        gdk_color_parse(value, &color);
        gtk_widget_modify_bg(main_wnd_, GTK_STATE_NORMAL, &color);

        settings_get(PREEDIT_FONT, value);
        gtk_widget_modify_font(candidate_area_,
                               pango_font_description_from_string(value));
        
        settings_get(PREEDIT_FONT_COLOR, value);
        gdk_color_parse(value, &color);
        gtk_widget_modify_fg(candidate_area_, GTK_STATE_NORMAL, &color);
        gtk_widget_modify_fg(preedit_area_, GTK_STATE_NORMAL, &color);

        settings_get(PREEDIT_OPACITY, &opa);
        GdkScreen* screen = gtk_widget_get_screen(main_wnd_);
        if (opa < 1.0) {
            GdkColormap* cmap = gdk_screen_get_rgba_colormap(screen);
            if (cmap) {
                gtk_widget_set_colormap(main_wnd_, cmap);
                gtk_window_set_opacity(GTK_WINDOW(main_wnd_), opa);
            }
        } else {
            GdkColormap* cmap = gdk_screen_get_rgb_colormap(screen);
            gtk_window_set_opacity(GTK_WINDOW(main_wnd_), 1.0);
            gtk_widget_set_colormap(main_wnd_, cmap);
        }
    }
    
    void move(int x, int y) {
        x_ = x;
        y_ = y;
        adjust_position();
    }

    void update_preedit_ui(const IPreeditString* ppd,
                           const char* utf_str) {
        gtk_label_set_text(GTK_LABEL(preedit_area_), utf_str);
    
        if (ppd->size() == 0) {
            status_ = false;
        } else if (!status_) {
            status_ = true;
        }
    }

    void update_candidates_ui(const ICandidateList* pcl,
                              const char* utf_str) {
        gtk_label_set(GTK_LABEL(candidate_area_), utf_str);

        PangoLayout* lay = gtk_label_get_layout(GTK_LABEL(candidate_area_));
        int wid = -1;
        pango_layout_get_pixel_size(lay, &wid, NULL);
        gtk_window_resize(GTK_WINDOW(main_wnd_), wid + 1, 1);
        gtk_window_get_size(GTK_WINDOW(main_wnd_), &width_, &height_);

        // add some offsets
        width_ += 10;

        adjust_position();
        if (status_)
            gtk_widget_show(main_wnd_);
        else
            gtk_widget_hide(main_wnd_);
    }

    bool status() {
        return status_;
    }

    void pause() {
        if (status_) {
            gtk_widget_hide(main_wnd_);
            status_ = false;
            pause_ = true;
        }
    }

    void go_on() {
        if (!status_ && pause_) {
            gtk_widget_show(main_wnd_);
            status_ = true;
            pause_ = false;
        }
    }
private:

    int x_, y_;
    int width_, height_;
    int screen_width_, screen_height_;
    
    int in_range(int x, int min, int max) {
        if (x < min)
            x = min;
        if (x > max)
            x = max;
        return x;
    }
    
    void adjust_position() {
        x_ = in_range(x_, 0, screen_width_ - width_);
        y_ = in_range(y_, 0, screen_height_ - height_);

        gtk_window_move(GTK_WINDOW(main_wnd_), x_, y_);
    }
};

#endif /* _SUNPINYIN_UI_GTK_H_ */

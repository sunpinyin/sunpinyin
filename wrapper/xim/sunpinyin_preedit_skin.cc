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

#include "ui.h"
#include "settings.h"
#include "sunpinyin_preedit_ui.h"
#include "skin.h"

SkinPreeditUI::SkinPreeditUI(std::string name) throw()
    : PreeditUI(name)
{
    info_ = ui_skin_new(name.c_str());
    if (!info_) {
        throw SkinLoaderException();
    }
    main_wnd_ = skin_window_new(GTK_WINDOW(ui_create_window()),
                                info_->preedit_background,
                                info_->top, info_->left,
                                info_->bottom, info_->right,
                                1);
    preedit_label_ = skin_label_new(info_->preedit_label.font, NULL,
                                    info_->preedit_label.color_r,
                                    info_->preedit_label.color_g,
                                    info_->preedit_label.color_b,
                                    info_->preedit_label.color_a);

    candidate_label_ = skin_label_new(info_->candidate_label.font, NULL,
                                      info_->candidate_label.color_r,
                                      info_->candidate_label.color_g,
                                      info_->candidate_label.color_b,
                                      info_->candidate_label.color_a);
    
    skin_window_add_label(main_wnd_, preedit_label_,
                          info_->preedit_label.x, info_->preedit_label.y);
    skin_window_add_label(main_wnd_, candidate_label_,
                          info_->candidate_label.x, info_->candidate_label.y);
    
    preedit_str_ = NULL;
}

SkinPreeditUI::~SkinPreeditUI()
{
    free(preedit_str_);
    
    ui_skin_destroy(info_);
    skin_label_destroy(preedit_label_);
    skin_label_destroy(candidate_label_);
    skin_window_destroy(main_wnd_);
}

void
SkinPreeditUI::show()
{
    gtk_widget_show(main_wnd_->widget);
    adjust_size();
}

void
SkinPreeditUI::hide()
{
    gtk_widget_hide(main_wnd_->widget);
}

void
SkinPreeditUI::move(int x, int y)
{
    internal_move(x - info_->offset_x, y - info_->offset_y);
}

void SkinPreeditUI::internal_move(int x, int y)
{
    int width, height;
    gtk_window_get_size(GTK_WINDOW(main_wnd_->widget), &width, &height);
    adjust_position(&x, &y, width, height);
    gtk_window_move(GTK_WINDOW(main_wnd_->widget), x, y);
}

void
SkinPreeditUI::reload()
{
    // Nothing?
}

void
SkinPreeditUI::update_preedit_string(const char* utf_str)
{
    free(preedit_str_);
    preedit_str_ = strdup(utf_str);
}

#define BUFSIZE (4096*2)

void
SkinPreeditUI::update_candidates_string(const char* utf_str)
{
    skin_label_set_text(preedit_label_, preedit_str_);
    skin_label_set_text(candidate_label_, utf_str);

    adjust_size();
}

void
SkinPreeditUI::adjust_size()
{
    if (candidate_label_->layout) {
        int x = 0, y = 0;
        int width = 0, height = 0;
        int hmargin, vmargin;
        hmargin = info_->candidate_label.x + info_->right;
        vmargin = info_->candidate_label.y + info_->bottom;
        pango_layout_get_pixel_size(candidate_label_->layout, &width, &height);
        gtk_window_resize(GTK_WINDOW(main_wnd_->widget), width + hmargin,
                          height + vmargin);
        gtk_window_get_position(GTK_WINDOW(main_wnd_->widget), &x, &y);
        internal_move(x, y);
    }
}

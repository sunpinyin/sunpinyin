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

#ifndef _UI_H_
#define _UI_H_

#include "common.h"
#include "settings.h"
#include <gtk/gtk.h>

__BEGIN_DECLS

typedef struct _skin_button_info_t
{
    int        x, y;
    GdkPixbuf* normal1;
    GdkPixbuf* highlight1;
    GdkPixbuf* pressdown1;
    GdkPixbuf* normal2;
    GdkPixbuf* highlight2;
    GdkPixbuf* pressdown2;
} skin_button_info_t;

typedef struct _skin_label_info_t
{
    int        x, y;
    char       font[256];
    double     color_r, color_g, color_b, color_a;
} skin_label_info_t;

typedef struct _skin_info_t
{
    skin_button_info_t eng_btn, full_btn, punc_btn;
    GdkPixbuf*         icbar_background;
    skin_label_info_t  preedit_label;
    skin_label_info_t  candidate_label;
    GdkPixbuf*         preedit_background;
    int                top, left, bottom, right; /* margins of preedit */
    int                offset_x, offset_y; /* offset of preedit */
} skin_info_t;

void          ui_tray_init       (void);
void          ui_tray_refresh    (void);
GtkWidget*    ui_create_window   (void);
skin_info_t*  ui_skin_new        (const char* name);
void          ui_skin_destroy    (skin_info_t* info);

__END_DECLS

#endif /* _UI_H_ */

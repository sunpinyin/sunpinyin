/*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
 * 
 * Copyright (c) 2007 Sun Microsystems, Inc. All Rights Reserved.
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

#ifndef SUNPINYIN_IMI_GTK_WIN_H
#define SUNPINYIN_IMI_GTK_WIN_H

#include "portability.h"

#include <X11/Xlib.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <iconv.h>

#include "imi_winHandler.h"

class CGTKWinHandler : public CIMIWinHandler
{
public:
    CGTKWinHandler(CIMIView* pv);

    bool createWindows(void);

    /* Inherited methods implementation */
    /*@{*/
    virtual ~CGTKWinHandler();

    /** Update window's preedit area using a GTK widget. */
    virtual void updatePreedit(const IPreeditString* ppd);

    /** Update window's candidates area using a GTK widget. */
    virtual void updateCandidates(const ICandidateList* pcl);

    /** Update status of current session using a GTK buttons. */
    virtual void  updateStatus(int key, int value);
    /*@}*/

protected:
    void switchCN(bool cn);
    void switchFullPunc(bool full);
    void switchFullSimbol(bool full);

protected:
    /*@{*/
    static const char *hanzi_image_file_name;
    static const char *eng_image_file_name;
    static const char *cnpunc_image_file_name;
    static const char *enpunc_image_file_name;
    static const char *fullwidth_image_file_name;
    static const char *halfwidth_image_file_name;
    /*@}*/

    /*@{*/
    static GtkWidget *hanzi_image;
    static GtkWidget *eng_image;
    static GtkWidget *cnpunc_image;
    static GtkWidget *enpunc_image;
    static GtkWidget *fullwidth_image;
    static GtkWidget *halfwidth_image;
    /*@}*/

protected:
    iconv_t             m_iconv;

    /*@{*/
    CIMIView           *mp_view;

    /** Main window */
    GtkWidget          *m_pWin;

    /** Candidate window */
    GtkWidget          *m_CandidataArea;

    /** Candidate window */
    GtkWidget          *m_PreeditArea;

    /** Button for CN/EN state*/
    GtkWidget          *m_pLangButton;

    /** Button for FULL/HALF Punc state */
    GtkWidget          *m_pPuncButton;

    /** Button for FULL/HALF Simbol state */
    GtkWidget          *m_pSimbButton;
    /*@}*/

private:
    /** load the images representing CN/EN, FULL/HALF PUNC|Simbol Simbol STATE*/
    void
    load_images();

private:
    char m_buf[512];
};

#endif

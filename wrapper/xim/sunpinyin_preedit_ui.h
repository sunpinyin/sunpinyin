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

#ifndef _SUNPINYIN_PREEDIT_H_
#define _SUNPINYIN_PREEDIT_H_

#include <string>
#include <cstring>
#include <exception>
#include <gtk/gtk.h>
#include "xmisc.h"
#include "skin.h"
#include "ui.h"

class PreeditUI
{
    std::string name_;
public:    
    PreeditUI(std::string name) throw() : name_(name) {}
    virtual ~PreeditUI() {}
    
    std::string name() { return name_; }
    
    virtual void show() = 0;
    virtual void hide() = 0;
    virtual void move(int x, int y) = 0;
    virtual void reload() = 0;
    virtual void update_preedit_string(const char* utf_str) = 0;
    virtual void update_candidates_string(const char* utf_str) = 0;
};

class GtkPreeditUI : public PreeditUI
{
public:
    GtkPreeditUI();
    virtual ~GtkPreeditUI();
protected:
    virtual void show();
    virtual void hide();
    virtual void move(int x, int y);
    virtual void reload();
    virtual void update_preedit_string(const char* utf_str);
    virtual void update_candidates_string(const char* utf_str);
private:
    GtkWidget* main_wnd_;
    GtkWidget* preedit_area_;
    GtkWidget* candidate_area_;
};

class SkinLoaderException: public std::exception
{
public:
    const char* what() const throw() { return "Cannot load skin!"; }
};

class SkinPreeditUI : public PreeditUI
{
public:
    SkinPreeditUI(std::string name) throw();
    virtual ~SkinPreeditUI();
protected:
    virtual void show();
    virtual void hide();
    virtual void move(int x, int y);
    virtual void reload();
    virtual void update_preedit_string(const char* utf_str);
    virtual void update_candidates_string(const char* utf_str);
private:
    void adjust_size();
    void internal_move(int x, int y);
private:
    skin_window_t* main_wnd_;
    skin_label_t*  preedit_label_;
    skin_label_t*  candidate_label_;
    skin_info_t*   info_;

    char* preedit_str_;
};

inline PreeditUI* create_preedit_ui(std::string name)
{
    if (name == "classic") {
        return new GtkPreeditUI();
    } else {
        try {
            return new SkinPreeditUI(name);
        } catch (const SkinLoaderException& ex) {
            fprintf(stderr, "%s\n", ex.what());
            exit(-1);
        }
    }
}

#endif /* _SUNPINYIN_PREEDIT_H_ */

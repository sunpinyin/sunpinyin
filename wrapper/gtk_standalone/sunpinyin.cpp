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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <locale.h>
#include <iconv.h>
#include <unistd.h>

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <X11/Xlib.h>

#include "imi_options.h"
#include "imi_gtkwin.h"

using namespace std;

int main(int argc, char* argv[])
{
    #ifdef DEBUG
        printf("sizef 'long double' %d, size 'of double' %d, size of 'float' %d \n", sizeof(long double), sizeof(double), sizeof(float));

        double test_array[] = { 3.0, 2.0, 1.5, 1.0, 0.5, 0.25, 0.0, -0.0, -0.25, -0.5, -1.0, -2.0, -3.0 };
        char valbuf[256];

        for (int i=0, sz=sizeof(test_array)/sizeof(double); i < sz; ++i) {
            TLongExpFloat lef(test_array[i]);
            lef.toString(valbuf);
            printf("value %lf ==> %s\n", test_array[i], valbuf);
        }
        fflush(stdout);
    #endif

    setlocale(LC_ALL, "zh_CN.UTF-8");
    int opt;
    char py_scheme = 'q';
    bool do_auto_correction = false;

    string_pairs auto_correction_pairs;
    auto_correction_pairs.push_back(make_pair("ign", "ing"));
    auto_correction_pairs.push_back(make_pair("img", "ing"));
    auto_correction_pairs.push_back(make_pair("uei", "ui"));
    auto_correction_pairs.push_back(make_pair("uen", "un"));
    auto_correction_pairs.push_back(make_pair("iou", "iu"));

    string_pairs fuzzy_pairs;
    fuzzy_pairs.push_back(make_pair("s", "sh"));
    fuzzy_pairs.push_back(make_pair("z", "zh"));
    fuzzy_pairs.push_back(make_pair("c", "ch"));

    while ((opt = getopt(argc, argv, "p:c")) != -1) {
        switch (opt) {
        case 'p':
            py_scheme = *optarg;
            break;
        case 'c':
            do_auto_correction = true;
            break;
        }
    }

    gtk_init(&argc, &argv);
    CSunpinyinSessionFactory& factory = CSunpinyinSessionFactory::getFactory ();
    switch (py_scheme) {
    case 'q':
        factory.setPinyinScheme (CSunpinyinSessionFactory::QUANPIN);
        break;
    case 's':
        factory.setPinyinScheme (CSunpinyinSessionFactory::SHUANGPIN);
        break;
    default:
        factory.setPinyinScheme (CSunpinyinSessionFactory::SHUANGPIN);
        break;
    }
    if (py_scheme == 'q' && do_auto_correction) {
        AQuanpinSchemePolicy::instance().setAutoCorrecting(true);
        AQuanpinSchemePolicy::instance().setAutoCorrectionPairs(auto_correction_pairs);
    }

    AQuanpinSchemePolicy::instance().setFuzzyForwarding(false, true);
    AQuanpinSchemePolicy::instance().setFuzzyPinyinPairs(fuzzy_pairs, false);
    AQuanpinSchemePolicy::instance().setFuzzySegmentation(true);

    //AShuangpinSchemePolicy::instance().setShuangpinType(ZIGUANG);
    CIMIView *pv = factory.createSession ();

    CGTKWinHandler *pwh = new CGTKWinHandler(pv);
    pv->attachWinHandler(pwh);

    pwh->createWindows();
    gtk_main();

    factory.destroySession (pv);
    delete pwh;

    return 0;
}

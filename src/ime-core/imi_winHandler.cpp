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

#include "imi_winHandler.h"
#include "imi_view.h"

void
CIMIWinHandler::commit(const TWCHAR* wstr)
{
    if (wstr) print_wide(wstr);
    fflush(stdout);
}

void
CIMIWinHandler::updatePreedit(const IPreeditString* ppd)
{
    if (ppd) {
        print_wide(ppd->string());
        printf("\n");
        fflush(stdout);
    }
}

void
CIMIWinHandler::updateCandidates(const ICandidateList* pcl)
{
    for (int i = 0, sz = pcl->size(); i < sz; ++i) {
        const TWCHAR* pcand = pcl->candiString(i);
        if (pcand != NULL) {
            printf("%c. ", '1' + i);
            print_wide(pcand);
            printf("\n");
        }
    }
    fflush(stdout);
}

void
CIMIWinHandler::throwBackKey(unsigned keycode,
                             unsigned keyvalue,
                             unsigned modifier)
{
    if (keyvalue > 0x0 && keyvalue < 0x7f) {
        printf("%c", keyvalue);
        fflush(stdout);
    }
}

void
CIMIWinHandler::updateStatus(int key, int value)
{
    switch (key) {
    case STATUS_ID_CN:
        printf("CN status is "); break;
    case STATUS_ID_FULLPUNC:
        printf("Full Punc is "); break;
    case STATUS_ID_FULLSYMBOL:
        printf("Full Simbol is "); break;
    default:
        printf("Unknown Status id %d is ", key);
        break;
    }

    printf("%d\n", value);
    fflush(stdout);
}

// -*- indent-tabs-mode: nil -*- vim:et:ts=4

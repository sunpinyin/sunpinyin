/*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
 *
 * Copyright (c) 2007, 2009 Kov Chai <tchaikov@gmail.com>
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
#include <string>
#include <algorithm>
#include <ime-core/imi_uiobjects.h>

#include "sunpinyin_engine.h"
#include "imi_ibus_win.h"



CIBusWinHandler::CIBusWinHandler(SunPinyinEngine* engine)
  : CIMIWinHandler(), m_engine(engine)
{
    //
}

CIBusWinHandler::~CIBusWinHandler()
{
}

void
CIBusWinHandler::commit(const TWCHAR* wstr)
{
    if (wstr) {
        std::wstring str;
        copy(wstr, wstr+WCSLEN(wstr)+1, back_inserter(str));
        m_engine->commit_string(str);
    }
}

void
CIBusWinHandler::updatePreedit(const IPreeditString* ppd)
{
    if (ppd)
        m_engine->update_preedit_string(*ppd);
}

void
CIBusWinHandler::updateCandidates(const ICandidateList* pcl)
{
    if (pcl)
        m_engine->update_candidates(*pcl);
}

void
CIBusWinHandler::throwBackKey(unsigned keycode, unsigned keyvalue, unsigned modifier)
{
    if (keyvalue > 0x0 && keyvalue < 0x7f) {
        printf("%c", keyvalue);
        fflush(stdout);
    }
}

void
CIBusWinHandler::updateStatus(int key, int value)
{
    switch (key) {
    case STATUS_ID_CN:
        m_engine->update_status_property(value != 0);
        break;
    case STATUS_ID_FULLPUNC:
        m_engine->update_punct_property(value != 0);
        break;
    case STATUS_ID_FULLSYMBOL:
        m_engine->update_letter_property(value != 0);
        break;
    default:
        break;
    }
}

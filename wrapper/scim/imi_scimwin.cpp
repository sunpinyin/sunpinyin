/*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
 *
 * Copyright (c) 2007 Kov Chai <tchaikov@gmail.com>
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
#define Uses_STL_AUTOPTR
#define Uses_STL_FUNCTIONAL
#define Uses_STL_VECTOR
#define Uses_STL_IOSTREAM
#define Uses_STL_FSTREAM
#define Uses_STL_ALGORITHM
#define Uses_STL_MAP
#define Uses_STL_UTILITY
#define Uses_STL_IOMANIP
#define Uses_C_STDIO
#define Uses_SCIM_UTILITY
#define Uses_SCIM_IMENGINE
#define Uses_SCIM_ICONV
#define Uses_SCIM_CONFIG_BASE
#define Uses_SCIM_CONFIG_PATH
#define Uses_SCIM_LOOKUP_TABLE
#define Uses_SCIM_DEBUG

#include "portability.h"

#include <imi_scimwin.h>
#include <imi_winHandler.h>
#include <imi_uiobjects.h>

#include <scim.h>
#include "sunpinyin_utils.h"
#include "sunpinyin_lookup_table.h"
#include "sunpinyin_imengine.h"

using namespace scim;

CScimWinHandler::CScimWinHandler(SunPyInstance* ime, SunLookupTable* lookup_table)
    : CIMIWinHandler(), m_ime(ime), m_lookup_table(lookup_table)
{
    //
}

CScimWinHandler::~CScimWinHandler()
{
}

void
CScimWinHandler::commit(const TWCHAR* wstr)
{
    if (wstr) {
        SCIM_DEBUG_IMENGINE (3) << "commit(" << wstr << ")\n";
        m_ime->commit_string(wstr_to_widestr(wstr));
    }
}

void
CScimWinHandler::updatePreedit(const IPreeditString* ppd)
{
    if (ppd)
        m_ime->redraw_preedit_string(ppd);
}

void
CScimWinHandler::updateCandidates(const ICandidateList* pcl)
{
    if (pcl)
        m_ime->redraw_lookup_table(pcl);
}

void
CScimWinHandler::throwBackKey(unsigned keycode, unsigned keyvalue, unsigned modifier)
{
    if (keyvalue > 0x0 && keyvalue < 0x7f) {
        printf("%c", keyvalue);
        fflush(stdout);
    }
}

void
CScimWinHandler::updateStatus(int key, int value)
{
    switch (key) {
    case STATUS_ID_CN:
        m_ime->refresh_status_property(value != 0);
        break;
    case STATUS_ID_FULLPUNC:
        m_ime->refresh_fullpunc_property(value != 0);
        break;
    case STATUS_ID_FULLSYMBOL:
        m_ime->refresh_fullsymbol_property(value != 0);
        break;
    default:
        SCIM_DEBUG_IMENGINE (2) << "Should not happen: updateStatus(" << key << ", " << value << ")\n";
        break;
    }
}

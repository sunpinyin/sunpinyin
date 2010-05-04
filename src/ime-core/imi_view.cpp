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

#include "imi_view.h"
#include "imi_view_classic.h"

#pragma setlocale("zh_CN.UTF-8")

CHotkeyProfile::CHotkeyProfile()
    : m_punctSwitchKey(IM_VK_PERIOD, 0, IM_CTRL_MASK),
      m_symbolSwitchKey(IM_VK_SPACE, 0, IM_SHIFT_MASK),
      m_candiDeleteKey(0, 0, IM_CTRL_MASK),
      m_prevKey(0)
{
    addModeSwitchKey(CKeyEvent(IM_VK_SHIFT_L, 0, IM_ALT_MASK));
}


CIMIView::CIMIView()
    : m_pIC(NULL), m_pWinHandler(NULL), m_pPySegmentor(NULL), m_pHotkeyProfile(NULL),
      m_candiWindowSize(10), m_bCN(true), m_bFullPunct(true), m_bFullSymbol(false)
{
}

void
CIMIView::setStatusAttrValue(int key, int value)
{
    switch(key) {
    case CIMIWinHandler::STATUS_ID_CN:
        m_bCN = (value != 0);
        if (m_pWinHandler)
            m_pWinHandler->updateStatus( key, value );
        break;
    case CIMIWinHandler::STATUS_ID_FULLPUNC:
        m_bFullPunct = (value != 0);
        if (m_pWinHandler)
            m_pWinHandler->updateStatus( key, value );
        if (m_pIC)
            m_pIC->setFullPunctForwarding (m_bFullPunct);
        break;
    case CIMIWinHandler::STATUS_ID_FULLSYMBOL:
        m_bFullSymbol = (value != 0);
        if (m_pWinHandler)
            m_pWinHandler->updateStatus( key, value );
        if (m_pIC)
            m_pIC->setFullSymbolForwarding (m_bFullSymbol);
        break;
    }
}

int
CIMIView::getStatusAttrValue(int key)
{
    switch(key) {
    case CIMIWinHandler::STATUS_ID_CN:
        return (m_bCN)?1:0;
    case CIMIWinHandler::STATUS_ID_FULLPUNC:
        return (m_bFullPunct)?1:0;
    case CIMIWinHandler::STATUS_ID_FULLSYMBOL:
        return (m_bFullSymbol)?1:0;
    }
    return 0;
}

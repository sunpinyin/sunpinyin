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

#ifdef ENABLE_PLUGINS
#include "imi_plugin.h"
#endif

// #pragma setlocale("zh_CN.UTF-8")

CHotkeyProfile::CHotkeyProfile()
    : m_punctSwitchKey(IM_VK_PERIOD, 0, IM_CTRL_MASK),
      m_symbolSwitchKey(IM_VK_SPACE, 0, IM_SHIFT_MASK),
      m_candiDeleteKey(0, 0, IM_CTRL_MASK),
      m_prevKey(0)
{
    addModeSwitchKey(CKeyEvent(IM_VK_SHIFT_L, 0, IM_ALT_MASK));
}


CIMIView::CIMIView()
    : m_pIC(NULL), m_pWinHandler(NULL), m_pPySegmentor(NULL),
      m_pHotkeyProfile(NULL),
      m_candiWindowSize(10), m_bCN(true), m_bFullPunct(true),
      m_bFullSymbol(false), m_backspaceCancel(true), m_smartPunct(true)
{
#ifdef ENABLE_PLUGINS
    // load all needed plugins
    AIMIPluginManager::instance().initializePlugins();
#endif
}

void
CIMIView::setStatusAttrValue(int key, int value)
{
    switch (key) {
    case CIMIWinHandler::STATUS_ID_CN:
        m_bCN = (value != 0);
        if (m_pWinHandler)
            m_pWinHandler->updateStatus(key, value);
        break;
    case CIMIWinHandler::STATUS_ID_FULLPUNC:
        m_bFullPunct = (value != 0);
        if (m_pWinHandler)
            m_pWinHandler->updateStatus(key, value);
        if (m_pIC)
            m_pIC->setFullPunctForwarding(m_bFullPunct);
        break;
    case CIMIWinHandler::STATUS_ID_FULLSYMBOL:
        m_bFullSymbol = (value != 0);
        if (m_pWinHandler)
            m_pWinHandler->updateStatus(key, value);
        if (m_pIC)
            m_pIC->setFullSymbolForwarding(m_bFullSymbol);
        break;
    }
}

int
CIMIView::getStatusAttrValue(int key)
{
    switch (key) {
    case CIMIWinHandler::STATUS_ID_CN:
        return (m_bCN) ? 1 : 0;
    case CIMIWinHandler::STATUS_ID_FULLPUNC:
        return (m_bFullPunct) ? 1 : 0;
    case CIMIWinHandler::STATUS_ID_FULLSYMBOL:
        return (m_bFullSymbol) ? 1 : 0;
    }
    return 0;
}

void
CIMIView::handlerUpdatePreedit(const IPreeditString* ppd)
{
    if (m_pWinHandler == NULL || ppd == NULL) {
        return;
    }
    // nothing to do here for plugins
    m_pWinHandler->updatePreedit(ppd);
}

// only build these with plugins support
#ifdef ENABLE_PLUGINS

void
CIMIView::_pluginProvideCandidates(wstring preedit, ICandidateList* pcl)
{
    CIMIPluginManager& manager = AIMIPluginManager::instance();
    if (preedit.size() == 0) {
        return;
    }

    for (size_t i = 0; i < manager.getPluginSize(); i++) {
        CIMIPlugin* plugin = manager.getPlugin(i);
        int wait_time = -1;
        TPluginCandidates candidates = plugin->provide_candidates(preedit,
                                                                  &wait_time);
        if (wait_time != 0) {
            manager.markWaitTime(wait_time);
        }

        for (size_t j = 0; j < candidates.size(); j++) {
            const TPluginCandidateItem& item = candidates[j];
            pcl->insertCandidateNoDedup(item.m_candidate,
                                        ICandidateList::PLUGIN_TAIL,
                                        item.m_rank);
        }
    }
}

void
CIMIView::_pluginTranslateCandidate(ICandidateList* pcl)
{
    CIMIPluginManager& manager = AIMIPluginManager::instance();
    ICandidateList::CCandiStrings& css = pcl->getCandiStrings();
    if (css.size() == 0) {
        return;
    }

    for (size_t i = 0; i < css.size(); i++) {
        for (size_t j = 0; j < manager.getPluginSize(); j++) {
            CIMIPlugin* plugin = manager.getPlugin(j);
            int wait_time = -1;
            wstring result = plugin->translate_candidate(css[i], &wait_time);
            if (wait_time != 0) {
                manager.markWaitTime(wait_time);
            } else {
                css[i] = result;
            }
        }
    }
}

#endif // ENABLE_PLUGINS

void
CIMIView::handlerUpdateCandidates(IPreeditString* ppd,
                                  ICandidateList* pcl)
{
    if (m_pWinHandler == NULL || pcl == NULL) {
        return;
    }
#ifdef ENABLE_PLUGINS
    _pluginProvideCandidates(ppd->getString(), pcl);
    pcl->shrinkList();
    _pluginTranslateCandidate(pcl);

    m_pWinHandler->updateCandidates(pcl);
    CIMIPluginManager& manager = AIMIPluginManager::instance();
    m_pWinHandler->enableDeferedUpdate(this, manager.getWaitTime());
    manager.resetWaitTime();
#else
    pcl->shrinkList();
    m_pWinHandler->updateCandidates(pcl);
#endif
}

void
CIMIView::handlerCommit(const wstring& wstr)
{
    if (m_pWinHandler == NULL) {
        return;
    }
#ifdef ENABLE_PLUGINS
    wstring commit_result = wstr;
    CIMIPluginManager& manager = AIMIPluginManager::instance();
    // re-run filter again
    for (size_t i = 0; i < manager.getPluginSize(); i++) {
        CIMIPlugin* plugin = manager.getPlugin(i);
        int wait_time = -1;
        wstring result = plugin->translate_candidate(commit_result, &wait_time);
        if (wait_time != 0) {
            continue;
        }
        commit_result = result;
    }
    m_pWinHandler->commit(commit_result.c_str());
    m_pWinHandler->disableDeferedUpdate();
#else
    m_pWinHandler->commit(wstr.c_str());
#endif
}

// -*- indent-tabs-mode: nil -*- vim:et:ts=4

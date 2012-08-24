// -*- mode: c++ -*-
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

#ifndef _SUNPINYIN_IMI_PLUGIN_H
#define _SUNPINYIN_IMI_PLUGIN_H

#include <string>
#include <vector>

#include "portability.h"
#include "utils.h"

typedef wstring TPluginPreedit;
typedef wstring TPluginCandidate;

struct TPluginCandidateItem
{
    size_t m_rank;
    TPluginCandidate m_candidate;
    TPluginCandidateItem(size_t rank, TPluginCandidate candidate)
        : m_rank(rank), m_candidate(candidate) {}
};

typedef std::vector<TPluginCandidateItem> TPluginCandidates;

enum TPluginTypeEnum
{
    CIMI_PLUGIN_PYTHON,
    CIMI_PLUGIN_UNKNOWN
};

class CIMIPlugin
{
private:
    TPluginTypeEnum m_pluginType;
protected:
    CIMIPlugin(TPluginTypeEnum pluginType);
public:
    virtual ~CIMIPlugin();
    virtual TPluginCandidates provide_candidates(const TPluginPreedit& str,
                                                 int* waitTime) = 0;
    virtual TPluginCandidate  translate_candidate(const TPluginCandidate& candi,
                                                  int* waitTime) = 0;

    virtual std::string getName() = 0;
    virtual std::string getAuthor() = 0;
    virtual std::string getDescription() = 0;

    TPluginTypeEnum getPluginType() const { return m_pluginType; }
};

class CIMIPluginManager
{
public:
    CIMIPluginManager();
    virtual ~CIMIPluginManager();

    void            initializePlugins();

    TPluginTypeEnum detectPluginType(std::string filename);
    CIMIPlugin*     loadPlugin(std::string filename);
    CIMIPlugin*     createPlugin(std::string filename,
                                 TPluginTypeEnum pluginType);
    size_t          getPluginSize() const { return m_plugins.size(); }
    CIMIPlugin*     getPlugin(size_t i) { return m_plugins[i]; }

    bool        hasLastError() const { return m_hasError; }
    std::string getLastError() const { return m_lastError; }
    void        setLastError(std::string desc);
    void        clearLastError();

    void        markWaitTime(int waitTime);
    int         getWaitTime() const { return m_waitTime; }
    void        resetWaitTime() { m_waitTime = 0; }
private:
    std::vector<CIMIPlugin*> m_plugins;

    bool        m_hasError;
    std::string m_lastError;
    int         m_waitTime;
};

typedef SingletonHolder<CIMIPluginManager> AIMIPluginManager;

#endif

// -*- indent-tabs-mode: nil -*- vim:et:ts=4

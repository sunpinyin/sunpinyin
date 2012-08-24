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

#include "imi_uiobjects.h"

#include <algorithm>

#define MAX_CANDI_COUNT_PER_WINDOW      16
#define MAX_STRING_LEN_CANDI_PREEDIT    256

IPreeditString::~IPreeditString()
{
}

CPreEditString::CPreEditString()
    : m_caret(0), m_candi_start(0), m_charTypes()
{
    m_charTypes.reserve(MAX_STRING_LEN_CANDI_PREEDIT);
}

CPreEditString::~CPreEditString()
{
}

int
CPreEditString::size() const
{
    return m_wstr.size();
}

const TWCHAR*
CPreEditString::string() const
{
    return m_wstr.c_str();
}

int
CPreEditString::charTypeSize() const
{
    return std::min(m_charTypes.size(), m_wstr.size());
}

int
CPreEditString::charTypeAt(int idx) const
{
    if (idx < 0 || idx >= (int) m_charTypes.size()) {
        return DONTCARE_CHAR;
    }
    return m_charTypes[idx];
}

int
CPreEditString::caret() const
{
    return m_caret;
}

int
CPreEditString::candi_start() const
{
    return m_candi_start;
}

void
CPreEditString::clear()
{
    m_caret = 0;
    m_wstr.clear();
    m_charTypes.clear();
}

wstring&
CPreEditString::getString()
{
    return m_wstr;
}

IPreeditString::CCharTypeVec &
CPreEditString::getCharTypeVec()
{
    return m_charTypes;
}

void
CPreEditString::setCaret(int caret)
{
    m_caret = caret;
}

void
CPreEditString::setCandiStart(int candi_start)
{
    m_candi_start = candi_start;
}

ICandidateList::~ICandidateList()
{
}

CCandidateList::CCandidateList()
    : m_total(0), m_first(0), m_size(0)
{
}

CCandidateList::~CCandidateList()
{
}

int
CCandidateList::size() const
{
    return m_candiStrings.size();
}

int
CCandidateList::total() const
{
    return m_total;
}

int
CCandidateList::first() const
{
    return m_first;
}

void
CCandidateList::setTotal(int total)
{
    m_total = total;
}

void
CCandidateList::setFirst(int first)
{
    m_first = first;
}

int
CCandidateList::candiType(unsigned item) const
{
    return (item >= m_candiTypes.size()) ? (NORMAL_WORD) : (m_candiTypes[item]);
}

int
CCandidateList::candiSize(unsigned item) const
{
    return (item >= m_candiStrings.size()) ? (0) : (m_candiStrings[item].size());
}

const TWCHAR*
CCandidateList::candiString(unsigned item) const
{
    return (item >=
            m_candiStrings.size()) ? (NULL) : (m_candiStrings[item].c_str());
}

int
CCandidateList::candiCharTypeSizeAt(unsigned item) const
{
    if (item >= m_candiCharTypeVecs.size())
        return 0;
    const CCharTypeVec & ctv = m_candiCharTypeVecs[item];
    return std::min((int)ctv.size(), candiSize(item));
}

int
CCandidateList::candiCharTypeAt(unsigned item, unsigned idx) const
{
    int sz = candiCharTypeSizeAt(item);
    if ((int) idx >= sz) {
        return DONTCARE_CHAR;
    }
    return m_candiCharTypeVecs[item][idx];
}

void
CCandidateList::clear()
{
    m_size = m_first = m_total = 0;
    m_candiStrings.clear();
    m_candiTypes.clear();
    m_candiCharTypeVecs.clear();
    m_candiUserIndex.clear();
    m_candiStringsIndex.clear();
}

void
CCandidateList::setSize(int count)
{
    m_size = count;
}

ICandidateList::CCandiStrings &
CCandidateList::getCandiStrings()
{
    return m_candiStrings;
}

ICandidateList::CCandiTypeVec &
CCandidateList::getCandiTypeVec()
{
    return m_candiTypes;
}

ICandidateList::CCharTypeVecs &
CCandidateList::getCharTypeVecs()
{
    return m_candiCharTypeVecs;
}

void
CCandidateList::pushBackCandidate(wstring wstr, int type, int userIdx)
{
    if (m_candiStringsIndex.find(wstr) == m_candiStringsIndex.end()) {
        m_candiStringsIndex.insert(std::make_pair(wstr, m_candiStrings.size()));
        m_candiStrings.push_back(wstr);
        m_candiTypes.push_back(type);
        m_candiUserIndex.push_back(userIdx);
    }
}

void
CCandidateList::insertCandidate(wstring wstr, int type, int rank, int userIdx)
{
    if (rank > (int) m_candiStrings.size()) {
        rank = m_candiStrings.size();
    }
    if (m_candiStringsIndex.find(wstr) == m_candiStringsIndex.end()) {
        m_candiStringsIndex.insert(std::make_pair(wstr, m_candiStrings.size()));
        insertCandidateNoDedup(wstr, type, rank, userIdx);
    } else {
        int idx = m_candiStringsIndex[wstr];
        if (rank >= idx) {
            return;
        }
        m_candiStringsIndex[wstr] = rank;
        m_candiStrings.erase(m_candiStrings.begin() + idx);
        m_candiTypes.erase(m_candiTypes.begin() + idx);
        m_candiUserIndex.erase(m_candiUserIndex.begin() + idx);
        m_candiStrings.insert(m_candiStrings.begin() + rank, wstr);
        m_candiTypes.insert(m_candiTypes.begin() + rank, type);
        m_candiUserIndex.insert(m_candiUserIndex.begin() + rank, userIdx);
    }
}

void
CCandidateList::insertCandidateNoDedup(wstring wstr, int type, int rank,
                                       int userIdx)
{
    m_candiStrings.insert(m_candiStrings.begin() + rank, wstr);
    m_candiTypes.insert(m_candiTypes.begin() + rank, type);
    m_candiUserIndex.insert(m_candiUserIndex.begin() + rank, userIdx);
}

void
CCandidateList::shrinkList()
{
    if ((int) m_candiStrings.size() > m_first) {
        m_candiStrings.erase(m_candiStrings.begin(),
                             m_candiStrings.begin() + m_first);
        m_candiTypes.erase(m_candiTypes.begin(),
                           m_candiTypes.begin() + m_first);
        m_candiUserIndex.erase(m_candiUserIndex.begin(),
                               m_candiUserIndex.begin() + m_first);
    }

    if ((int) m_candiStrings.size() > m_size) {
        m_candiStrings.erase(m_candiStrings.begin() + m_size,
                             m_candiStrings.end());
        m_candiTypes.erase(m_candiTypes.begin() + m_size,
                           m_candiTypes.end());
        m_candiUserIndex.erase(m_candiUserIndex.begin() + m_size,
                               m_candiUserIndex.end());
    }
}

// -*- indent-tabs-mode: nil -*- vim:et:ts=4

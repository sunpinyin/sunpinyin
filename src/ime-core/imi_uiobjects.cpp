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
    : m_caret(0), m_candi_start(0), m_wstr(), m_charTypes()
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
    return (idx < 0 || idx >= m_charTypes.size())?(DONTCARE_CHAR):(m_charTypes[idx]);
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
    : m_candiStrings(), m_candiTypes(), m_candiCharTypeVecs(),
      m_first(0), m_total(0)
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
    return (item >= m_candiTypes.size())?(NORMAL_WORD):(m_candiTypes[item]);
}

int
CCandidateList::candiSize(unsigned item) const
{
    return (item >= m_candiStrings.size())?(0):(m_candiStrings[item].size());
}

const TWCHAR*
CCandidateList::candiString(unsigned item) const
{
    return (item >= m_candiStrings.size())?(NULL):(m_candiStrings[item].c_str());
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
    return (idx >= sz)?(DONTCARE_CHAR):(m_candiCharTypeVecs[item][idx]);
}

void
CCandidateList::clear()
{
    m_first = m_total = 0;
    m_candiStrings.clear();
    m_candiTypes.clear();
    m_candiCharTypeVecs.clear();
}

void
CCandidateList::reserve(int count)
{
    m_candiStrings.reserve(count);
    m_candiTypes.reserve(count);
    m_candiCharTypeVecs.reserve(count);
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

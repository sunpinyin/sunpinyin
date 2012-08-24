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

#include "imi_funcobjs.h"

void
CGetFullSymbolOp::initSymbolMap()
{
    m_symbolMap.clear();

    TWCHAR cwstr[256];
    static const char half_symbol[] = "azAZ";
    static const char full_symbol[] = "ａｚＡＺ";

    char * src = (char*)full_symbol;
    memset(&cwstr[0], 0, sizeof(cwstr));
    MBSTOWCS(&cwstr[0], src, (sizeof(cwstr) / sizeof(TWCHAR)) - 1);

    TWCHAR wch = cwstr[0];
    for (char h = half_symbol[0], t = half_symbol[1]; h <= t; ++h, ++wch)
        m_symbolMap[h] = wch;

    wch = cwstr[2];
    for (char h = half_symbol[2], t = half_symbol[3]; h <= t; ++h, ++wch)
        m_symbolMap[h] = wch;
}

const wstring&
CGetFullSymbolOp::operator ()(unsigned ch)
{
    static wstring ret;

    if (m_symbolMap.empty())
        initSymbolMap();

    CSymbolMap::iterator it1 = m_symbolMap.find(ch);
    if (it1 != m_symbolMap.end())
        return it1->second;

    return ret;
}

CGetFullPunctOp::CGetFullPunctOp()
{
}

void
CGetFullPunctOp::initPunctMap(const string_pairs& punc_map)
{
    TWCHAR cwstr[256];

    m_punctMap.clear();
    m_punctClosingSts.clear();

    string_pairs::const_iterator it = punc_map.begin();
    string_pairs::const_iterator ite = punc_map.end();

    for (; it != ite; ++it) {
        const char *k = it->first.c_str();
        const char *v = it->second.c_str();

        if (!v) continue;

        memset(cwstr, 0, sizeof(cwstr));
        MBSTOWCS(&cwstr[0], v, (sizeof(cwstr) / sizeof(TWCHAR)) - 1);

        unsigned key = *k;
        if (m_punctMap.find(*k) != m_punctMap.end()) {
            m_punctClosingSts.insert(std::make_pair(key, false));
            key |= 0x80000000;
        }

        m_punctMap[key] = wstring(cwstr);
    }
}

const wstring&
CGetFullPunctOp::operator ()(unsigned ch)
{
    static wstring ret;
    CPunctClosingStates::iterator it = m_punctClosingSts.find(ch);
    if (it != m_punctClosingSts.end()) {
        if (it->second) ch |= 0x80000000;
        it->second = !it->second;
    }

    CPunctMap::iterator it1 = m_punctMap.find(ch);
    if (it1 != m_punctMap.end())
        return it1->second;

    return ret;
}

// -*- indent-tabs-mode: nil -*- vim:et:ts=4

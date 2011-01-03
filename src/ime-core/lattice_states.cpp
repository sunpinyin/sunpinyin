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

#include "pinyin_data.h"
#include "lattice_states.h"
#include <algorithm>

const CPinyinTrie::TWordIdInfo*
TLexiconState::getWords (unsigned &num)
{
    num = 0;

    if (!m_words.empty ()) {
        num = m_words.size();
        return &m_words[0];
    }

    if (m_bPinyin && m_pPYNode) {
        num = m_pPYNode->m_nWordId;
        return m_pPYNode->getWordIdPtr();
    }

    return NULL;
}

void
TLexiconState::print (std::string prefix) const
{
    printf ("%s", prefix.c_str());
    printf ("from frame[%d] ", m_start);

    if (m_bPinyin) {
        printf ("%sdict ", m_pPYNode?"sys":"usr");
        if (!m_syls.empty()) {
            printf ("pinyin: ");
            CSyllables::const_iterator it  = m_syls.begin();
            for (; it != m_syls.end(); ++it)
                printf ("%x:%x:%x ", it->initial, it->final, it->tone);
        }

        printf ("seg_ranges: (");
        for (std::vector<unsigned>::const_iterator it = m_seg_path.begin(); it != m_seg_path.end(); ++it)
            printf ("%d ", *it);
        printf (")");

    } else {
        printf ("word id ");
        printf ("%d", m_words.front().m_id);
    }

    printf ("\n");
}

void
TLatticeState::print(std::string prefix) const
{
    printf("%s", prefix.c_str());
    char valbuf[256];
    m_score.toString(valbuf);
    printf("<State(%d:%d), from word %d, score %s>\n", m_slmState.getLevel(), m_slmState.getIdx(), m_backTraceWordId, valbuf);
}

const unsigned CLatticeStates::beam_width;

bool
CTopLatticeStates::push(const TLatticeState& state)
{
    bool ret = true;
    if (size() >= m_threshold) {
        if (m_heap[0] < state) return false;
        std::pop_heap(m_heap.begin(), m_heap.end());
        m_heap.pop_back();
        ret = false;
    }
    m_heap.push_back(state);
    std::push_heap(m_heap.begin(), m_heap.end());
    return ret;
}

void
CTopLatticeStates::pop()
{
    std::pop_heap(m_heap.begin(), m_heap.end());
    m_heap.pop_back();
}

void
CLatticeStates::clear()
{
    m_stateMap.clear();
    m_size = 0;
}

std::vector<TLatticeState>
CLatticeStates::getSortedResult()
{
    std::vector<TLatticeState> res;
    for (CLatticeStates::iterator it = begin(); it != end(); ++it) {
        res.push_back(*it);
    }
    std::sort(res.begin(), res.end());
    return res;
}

void
CLatticeStates::add(const TLatticeState& state)
{
    bool inserted = false;
    state_map::iterator it = m_stateMap.find(state.m_slmState);
    if (it == m_stateMap.end()) {
        CTopLatticeStates topstates(m_nMaxBest);
        inserted = topstates.push(state);
        m_stateMap.insert(std::make_pair(state.m_slmState, topstates));
    } else {
        inserted = it->second.push(state);
    }
    if (inserted) m_size++;
    if (m_size > beam_width) {
        TSentenceScore max_score(-1.0 * INT_MAX);
        // find the largest top states
        std::map<CSlmState, CTopLatticeStates>::iterator it, mark;
        for (it = m_stateMap.begin(); it != m_stateMap.end(); ++it) {
            TSentenceScore this_score = it->second.top().m_score;
            if (max_score < this_score) {
                max_score = this_score;
                mark = it;
            }
        }
        // pop one node from it, and if it's empty, remove it from map
        mark->second.pop();
        if (mark->second.size() == 0) {
            m_stateMap.erase(mark);
        }
        m_size--;
    }
}

CLatticeStates::iterator
CLatticeStates::begin()
{
    CLatticeStates::iterator it;
    it.main_it = m_stateMap.begin();
    it.main_end = m_stateMap.end();
    it.child_it = it.main_it->second.begin();
    return it;
}

CLatticeStates::iterator
CLatticeStates::end()
{
    CLatticeStates::iterator it;
    it.main_end = it.main_it = m_stateMap.end();
    return it;
}

void
CLatticeStates::iterator::operator++()
{
    ++child_it;
    if (child_it == main_it->second.end()) {
        ++main_it;
        if (main_it != main_end)
            child_it = main_it->second.begin();
    }
}

bool
CLatticeStates::iterator::operator!=(const CLatticeStates::iterator& rhs)
{
    if (main_it == main_end || rhs.main_it == rhs.main_end) {
        return main_it != rhs.main_it;
    } else {
        return main_it != rhs.main_it && child_it != rhs.child_it;
    }
}

TLatticeState&
CLatticeStates::iterator::operator*()
{
    return child_it.operator*();
}

TLatticeState*
CLatticeStates::iterator::operator->()
{
    return child_it.operator->();
}

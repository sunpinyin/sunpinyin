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
TLexiconState::getWords(unsigned &num)
{
    num = 0;

    if (!m_words.empty()) {
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
TLexiconState::print(std::string prefix) const
{
    printf("%s", prefix.c_str());
    printf("from frame[%d] ", m_start);

    if (m_bPinyin) {
        printf("%sdict ", m_pPYNode ? "sys" : "usr");
        if (!m_syls.empty()) {
            printf("pinyin: ");
            CSyllables::const_iterator it = m_syls.begin();
            for (; it != m_syls.end(); ++it)
                printf("%x:%x:%x ", it->initial, it->final, it->tone);
        }

        printf("seg_ranges: (");
        for (std::vector<unsigned>::const_iterator it = m_seg_path.begin();
             it != m_seg_path.end();
             ++it)
            printf("%d ", *it);
        printf(")");
    } else {
        printf("word id ");
        printf("%d", m_words.front().m_id);
    }

    printf("\n");
}

void
TLatticeState::print(std::string prefix) const
{
    printf("%s", prefix.c_str());
    char valbuf[256];
    m_score.toString(valbuf);
    printf("<State(%d:%d), from word %d, score %s>\n", m_slmState.getLevel(),
           m_slmState.getIdx(), m_backTraceWordId, valbuf);
}

const unsigned CLatticeStates::beam_width = 48;
const TSentenceScore CLatticeStates::filter_ratio_l1 = TSentenceScore(0.12);
const TSentenceScore CLatticeStates::filter_ratio_l2 = TSentenceScore(0.02);
const TSentenceScore CLatticeStates::filter_threshold_exp =
    TSentenceScore(-40, -1.0);

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
    m_heapIdx.clear();
    m_scoreHeap.clear();
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

std::vector<TLatticeState>
CLatticeStates::getFilteredResult()
{
    std::vector<TLatticeState> sorted = getSortedResult();
    std::vector<TLatticeState> res;
    if (sorted.size() == 0) {
        return sorted;
    }
    res.push_back(sorted[0]);
    TSentenceScore max_score = sorted[0].m_score;
    for (size_t i = 1; i < sorted.size(); i++) {
        TSentenceScore current_score = sorted[i].m_score;
        if (filter_threshold_exp < current_score
            && current_score / max_score < filter_ratio_l1) {
            break;
        }
        if (current_score / max_score < filter_ratio_l2) {
            break;
        }
        res.push_back(sorted[i]);
    }
    return res;
}

void
CLatticeStates::add(const TLatticeState& state)
{
    CSlmState slmState = state.m_slmState;
    state_map::iterator it = m_stateMap.find(slmState);
    bool inserted = false;

    if (it == m_stateMap.end()) {
        CTopLatticeStates topstates(m_maxBest);
        inserted = topstates.push(state);
        m_stateMap.insert(std::make_pair(slmState, topstates));
        _pushScoreHeap(state.m_score, slmState);
    } else {
        inserted = it->second.push(state);
        slmState = it->second.top().m_slmState;
        _adjustDown(m_heapIdx[slmState]);
    }
    if (inserted) m_size++;

    if (m_size > beam_width) {
        slmState = m_scoreHeap[0].second;
        it = m_stateMap.find(slmState);

        // pop one node from it, and if it's empty, remove it from map and heap
        it->second.pop();
        if (it->second.size() == 0) {
            m_stateMap.erase(it);
            _popScoreHeap();
        } else {
            m_scoreHeap[0].first = it->second.top().m_score;
            _adjustDown(0);
        }
        m_size--;
    }
}

void
CLatticeStates::_pushScoreHeap(TSentenceScore score, CSlmState slmState)
{
    m_scoreHeap.push_back(std::make_pair(score, slmState));
    _adjustUp(m_scoreHeap.size() - 1);
}

void
CLatticeStates::_popScoreHeap()
{
    m_heapIdx.erase(m_scoreHeap[0].second);
    m_scoreHeap[0] = m_scoreHeap[m_scoreHeap.size() - 1];
    m_scoreHeap.pop_back();
    if (m_scoreHeap.size() > 0) {
        _refreshHeapIdx(0);
        _adjustDown(0);
    }
}

void
CLatticeStates::_refreshHeapIdx(int heapIdx)
{
    m_heapIdx[m_scoreHeap[heapIdx].second] = heapIdx;
}

void
CLatticeStates::_adjustUp(int node)
{
    int parent = (node - 1) / 2;
    while (parent >= 0) {
        if (m_scoreHeap[parent].first < m_scoreHeap[node].first) {
            std::swap(m_scoreHeap[parent], m_scoreHeap[node]);
            _refreshHeapIdx(parent);
            node = parent;
            parent = (node - 1) / 2;
        } else {
            _refreshHeapIdx(node);
            return;
        }
    }
}

void
CLatticeStates::_adjustDown(int node)
{
    int left = node * 2 + 1;
    int right = node * 2 + 2;
    while (left < (int) m_scoreHeap.size()) {
        int child = -1;
        if (m_scoreHeap[node].first < m_scoreHeap[left].first) {
            child = left;
        } else if (right < (int) m_scoreHeap.size()
                   && m_scoreHeap[node].first < m_scoreHeap[right].first) {
            child = right;
        } else {
            _refreshHeapIdx(node);
            return;
        }
        std::swap(m_scoreHeap[node], m_scoreHeap[child]);
        _refreshHeapIdx(child);
        node = child;
        left = node * 2 + 1;
        right = node * 2 + 2;
    }
}

CLatticeStates::iterator
CLatticeStates::begin()
{
    CLatticeStates::iterator it;
    it.m_mainIt = m_stateMap.begin();
    it.m_mainEnd = m_stateMap.end();
    it.m_childIt = it.m_mainIt->second.begin();
    return it;
}

CLatticeStates::iterator
CLatticeStates::end()
{
    CLatticeStates::iterator it;
    it.m_mainEnd = it.m_mainIt = m_stateMap.end();
    return it;
}

void
CLatticeStates::iterator::operator++()
{
    ++m_childIt;
    if (m_childIt == m_mainIt->second.end()) {
        ++m_mainIt;
        if (m_mainIt != m_mainEnd)
            m_childIt = m_mainIt->second.begin();
    }
}

bool
CLatticeStates::iterator::operator!=(const CLatticeStates::iterator& rhs)
{
    if (m_mainIt == m_mainEnd || rhs.m_mainIt == rhs.m_mainEnd) {
        return m_mainIt != rhs.m_mainIt;
    } else {
        return m_mainIt != rhs.m_mainIt && m_childIt != rhs.m_childIt;
    }
}

TLatticeState&
CLatticeStates::iterator::operator*()
{
    return m_childIt.operator*();
}

TLatticeState*
CLatticeStates::iterator::operator->()
{
    return m_childIt.operator->();
}

// -*- indent-tabs-mode: nil -*- vim:et:ts=4

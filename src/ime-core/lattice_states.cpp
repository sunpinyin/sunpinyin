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

/**
 * clear all existing states, and assure the vector's size.
 * Of cause the reserve space action only useful right after construction,
 * but just let it be.
 */
void
CLatticeStates::clear()
{
    m_vec.clear();
    m_heap.clear();
    m_vecIdxInHeap.clear();
    m_map.clear();

    m_vec.reserve(beam_width);
    m_vecIdxInHeap.reserve(beam_width);
    m_heap.reserve(beam_width);
}

/**
 * Following conditions may be encountered:
 *    - node's StateKey is alredy in the map
 *        -# But its score is larger than the original one: \n
 *                   ===> do nothing.
 *        -# its score is less than the original one: \n
 *                   ===> replace original node with new one, ironDown it
 *    - node is a new node
 *        -# Vector is full
 *            - and node's score is >= the largest score of all existing node:\n
 *                   ===> do nothing.
 *            - node score < the largest score of all existing node:\n
 *                   ===> Relpace the largest node in the vector with new node,
 *                   ironDown it (0 on the heap)
 *        -# Vector is not full: \n
 *             ===> push_back the new node in vector, build heap node in the
 *             heap tail. bubbleUp it.
 */
void
CLatticeStates::push_back(const TLatticeState& node)
{
    std::map<CSlmState, int>::iterator itMap;

    itMap = m_map.find(node.m_slmState);
    if (itMap != m_map.end()) {
        TLatticeState& oldNode = m_vec[itMap->second];
        if (node.m_score < oldNode.m_score) {
            oldNode = node;
            ironDown(m_vecIdxInHeap[itMap->second]);
        }
    } else {
        if (m_vec.size() >= beam_width) {
            TLatticeState& oldNode = m_vec[m_heap[0]];
            if (node.m_score < oldNode.m_score) {
                itMap = m_map.find(oldNode.m_slmState);
                m_map.erase(itMap);
                m_map[node.m_slmState] = m_heap[0];
                oldNode = node;
                ironDown(0);
            }
        } else {
            m_map[node.m_slmState] = m_vec.size();
            m_vecIdxInHeap.push_back(m_vec.size());
            m_vec.push_back(node);
            m_heap.push_back(m_heap.size());
            bubbleUp(m_heap.size()-1);
        }
    }
}

/**
 * The index of heap's corresponding node like a bubble,
 * it should go up in the heap until it goes to the top
 * or meet lighter bubble.
 */
void
CLatticeStates::bubbleUp(int idxInHeap)
{
    while (idxInHeap > 0) {
        int idxNode = m_heap[idxInHeap];
        int parentInHeap = (idxInHeap-1) / 2;
        int parentNode = m_heap[parentInHeap];

        // if child's score > parent's, try to swap them
        if (m_vec[parentNode].m_score < m_vec[idxNode].m_score) {
            m_vecIdxInHeap[idxNode] = parentInHeap;
            m_vecIdxInHeap[parentNode]  = idxInHeap;
            m_heap[idxInHeap] = parentNode;
            m_heap[parentInHeap] = idxNode;
            idxInHeap = parentInHeap;
        } else {
            break;
        }
    }
}

/**
 * The index of heap's corresponding node like a iron,
 * it should go down in the heap, until to the bottom, or
 * meet heavier bubble.
 */
void
CLatticeStates::ironDown(int idxInHeap)
{
    int sz = m_heap.size(), lcInHeap, rcInHeap;
    while ((lcInHeap = 2*idxInHeap + 1) < sz) {
        int idxNode = m_heap[idxInHeap];
        int lcNode = m_heap[lcInHeap];

        // make the rcInHeap become the child idx in heap whose node's
        // score is the largest of the two child node
        rcInHeap = lcInHeap + 1;
        if (rcInHeap >= sz ||
            m_vec[m_heap[rcInHeap]].m_score <= m_vec[lcNode].m_score)
            rcInHeap = lcInHeap;
        int rcNode = m_heap[rcInHeap];

        // if parent score not the largest, try to swap it with better child
        if (m_vec[idxNode].m_score < m_vec[rcNode].m_score) {
            m_vecIdxInHeap[idxNode] = rcInHeap;
            m_vecIdxInHeap[rcNode]  = idxInHeap;
            m_heap[idxInHeap] = rcNode;
            m_heap[rcInHeap] = idxNode;
            idxInHeap = rcInHeap;
        } else {
            break;
        }
    }
}

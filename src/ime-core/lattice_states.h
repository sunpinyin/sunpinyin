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

#ifndef SUNPY_LATTICE_STATES_H
#define SUNPY_LATTICE_STATES_H

#include <vector>
#include <map>
#include "portability.h"
#include "imi_data.h"
#include "pinyin/pinyin_seg.h"

typedef TLongExpFloat TSentenceScore;

/**
 * CSlmState represent the history. In real implementation, it's a
 * node pointer to a state in the language model. But to save the
 * language model size, the state node in language model do not
 * thread the back-off pointer. Now, we just use the Word Id for
 * the node in the language model. Later we should abstract the
 * StateNode from language model implemetation to replace this
 * definition.
 */
typedef CThreadSlm::TState          CSlmState;

/**
 * A WordKey could represent a word. Define this use the unsigned int
 * directly. Because in the future, we may adopt word class, such as
 * Digital Word Class.
 */
typedef unsigned                    CWordId;

/**
 * This class is used to record lexicon state (pinyin trie nodes)
 * just before a bone. From the bone, it could see when arriving
 * it, how many valid Pinyin Trie Node still could be used to search
 * more words further, and what bone is its starting bone.
 */
struct TLexiconState {
    typedef std::vector<CPinyinTrie::TWordIdInfo> TWordIdInfoVec;

    const CPinyinTrie::TNode   *m_pPYNode;
    TWordIdInfoVec              m_words;
    CSyllables                  m_syls;         // accumulated syllables, may contain fuzzy syllables
    std::vector<unsigned>       m_seg_path;     // accumulated segments,  may contain fuzzy segments
    unsigned                    m_start                 :16;
    unsigned                    m_num_of_inner_fuzzies  :15;
    bool                        m_bPinyin               :1;

    TLexiconState (unsigned start, const CPinyinTrie::TNode *pnode, CSyllables& syls, std::vector<unsigned>& seg_path):
        m_start(start), m_pPYNode(pnode), m_syls(syls), m_seg_path(seg_path), m_bPinyin(true), m_num_of_inner_fuzzies(0) {}

    TLexiconState (unsigned start, TWordIdInfoVec &words, CSyllables &syls, std::vector<unsigned>& seg_path):
        m_start(start), m_pPYNode(NULL), m_words(words), m_syls(syls), m_seg_path(seg_path), m_bPinyin(true), m_num_of_inner_fuzzies(0) {}

    TLexiconState (unsigned start, unsigned wid):
        m_start(start), m_pPYNode(NULL), m_bPinyin(false)
        {
            m_words.push_back(wid);
            m_seg_path.push_back (start);
            m_seg_path.push_back (start+1);
        }

    const CPinyinTrie::TWordIdInfo *getWords (unsigned &num);
    void print (std::string prefix) const;
};

/**
 * A list of Lexicon State. Every state may from different
 * starting position. Later, when Fuzzy PinYin are added,
 * more than one state may comes from one starting bone.
 */
typedef std::vector<TLexiconState>    CLexiconStates;


/**
 * The basic static unit used in the lattice searching
 */
struct TLatticeState {
    TSentenceScore      m_score;
    unsigned            m_frIdx;
    TLexiconState      *m_pLexiconState;
    TLatticeState      *m_pBackTraceNode;
    CSlmState           m_slmState;
    CWordId             m_backTraceWordId;
    
    TLatticeState(double score = -1.0,
                  unsigned frIdx=0,
                  TLexiconState* lxstPtr = NULL,
                  TLatticeState* btNodePtr = NULL,
                  CSlmState sk= CSlmState(),
                  CWordId wk = CWordId())
        : m_score(score), m_frIdx(frIdx), m_pBackTraceNode(btNodePtr),
          m_pLexiconState(lxstPtr), m_slmState(sk), m_backTraceWordId(wk) {}

    /** for debug printing... */
    void
    print(std::string prefix) const;
};

typedef std::vector<TLatticeState>  CLatticeStateVec;

/**
 * All lattice node on a lattice frame. This class provide beam pruning
 * while push_back, which means at most the best MAX states are reserved,
 * ie, weak state will may be discard while new better state are inserted,
 * and the number MAX is arrived.
 */
class CLatticeStates {
private:
    static const unsigned beam_width = 32;

public:
    /** just use the CLatticeStateVec's iterator */
    typedef CLatticeStateVec::iterator        iterator;

    /** just use the CLatticeStateVec's iterator */
    typedef CLatticeStateVec::const_iterator  const_iterator;

    typedef CLatticeStateVec::reference       reference;
    typedef CLatticeStateVec::const_reference const_reference;
    typedef CLatticeStateVec::size_type       size_type;

public:
    void
    clear();

    void
    push_back(const TLatticeState& node);

    //@{
    /** return the first iterator of m_vec. */
    size_t
    size()
        { return m_vec.size(); }

    iterator
    begin()
        { return m_vec.begin(); }

    /** return the first iterator of m_vec. */
    const_iterator
    begin() const
        { return m_vec.begin(); }
    //@}


    //@{
    /** return the last iterator of m_vec. */
    iterator
    end()
        { return m_vec.end(); }

    /** return the last iterator of m_vec. */
    const_iterator
    end() const
        { return m_vec.end(); }
    //@}

    reference
    operator[] (size_type index)
        {return m_vec[index];}

    const_reference
    operator[] (size_type index) const
        {return m_vec[index];}

protected:
    void
    bubbleUp(int idxInHeap);

    void
    ironDown(int idxInHeap);

protected:
    std::vector<TLatticeState>      m_vec;
    std::vector<int>                m_vecIdxInHeap;
    std::map<CSlmState, int>        m_map;
    std::vector<int>                m_heap;
};

#endif

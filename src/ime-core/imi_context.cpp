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

#include <assert.h>
#include <algorithm>
#include "imi_defines.h"
#include "imi_context.h"

TCandiRank::TCandiRank(bool user, bool best, unsigned len,
                       bool fromLattice, TSentenceScore score)
{
    anony.m_user = (user) ? 0 : 1;
    anony.m_best = (best) ? 0 : 1;
    anony.m_len = (len > 31) ? (0) : (31 - len);
    anony.m_lattice = (fromLattice) ? 0 : 1;

    double ds = -score.log2();

    //make it 24-bit
    if (ds > 32767.0)
        ds = 32767.0;
    else if (ds < -32768.0)
        ds = -32768.0;
    unsigned cost = unsigned((ds + 32768.0) * 256.0);
    anony.m_cost = cost;
}

TCandiRank::TCandiRank(bool user, bool best, unsigned len,
                       bool fromLattice, unsigned rank)
{
    anony.m_user = (user) ? 0 : 1;
    anony.m_best = (best) ? 0 : 1;
    anony.m_len = (len > 31) ? (0) : (31 - len);
    anony.m_lattice = (fromLattice) ? 0 : 1;
    anony.m_cost = rank;
}

void
CLatticeFrame::print(std::string prefix)
{
    if (m_bwType & BESTWORD) printf("B");
    if (m_bwType & USER_SELECTED) printf("U");
    printf("\n");

    prefix += "    ";
    printf("  Lexicon States:\n");
    for_each(m_lexiconStates.begin(), m_lexiconStates.end(),
             bind2nd(mem_fun_ref(&TLexiconState::print), prefix));

    printf("  Lattice States:\n");
    for_each(m_latticeStates.begin(), m_latticeStates.end(),
             bind2nd(mem_fun_ref(&TLatticeState::print), prefix));
    printf("\n");
}

void
CIMIContext::printLattice()
{
    std::string prefix;

    for (size_t i = 0; i <= m_tailIdx; ++i) {
        if (m_lattice[i].m_type == CLatticeFrame::UNUSED)
            continue;

        printf("Lattice Frame [%lu]:", i);
        m_lattice[i].print(prefix);
    }
}

CIMIContext::CIMIContext()
    : m_tailIdx(1), m_nBest(0), m_maxBest(1), m_maxTailCandidateNum(0),
      m_pModel(NULL), m_pPinyinTrie(NULL), m_pUserDict(NULL), m_pHistory(NULL),
      m_historyPower(5), m_csLevel(0), m_bFullSymbolForwarding(false),
      m_bOmitPunct(false), m_pGetFullSymbolOp(NULL),
      m_bFullPunctForwarding(true), m_pGetFullPunctOp(NULL),
      m_pPySegmentor(NULL), m_bNonCompleteSyllable(true),
      m_bDynaCandiOrder(true), m_candiStarts(0), m_candiEnds(0)
{
    m_lattice.resize(MAX_LATTICE_LENGTH);
    m_lattice[0].m_latticeStates.add(TLatticeState(-1.0, 0));
    setMaxBest(m_maxBest);
}

void
CIMIContext::setCoreData(CIMIData *pCoreData)
{
    m_pModel = pCoreData->getSlm();
    m_pPinyinTrie = pCoreData->getPinyinTrie();
}

void
CIMIContext::clear()
{
    _clearFrom(1);
    _clearPaths();
    m_tailIdx = 1;
    m_candiStarts = m_candiEnds = 0;
}

void
CIMIContext::_clearFrom(unsigned idx)
{
    for (size_t i = idx; i < m_tailIdx + 1; i++)
        m_lattice[i].clear();
}

bool
CIMIContext::buildLattice(IPySegmentor *segmentor, bool doSearch)
{
    m_pPySegmentor = segmentor;
    return _buildLattice(segmentor->getSegments(),
                         segmentor->updatedFrom() + 1, doSearch);
}

bool
CIMIContext::_buildLattice(IPySegmentor::TSegmentVec &segments,
                           unsigned rebuildFrom,
                           bool doSearch)
{
    _clearFrom(rebuildFrom);

    IPySegmentor::TSegmentVec::const_iterator it = segments.begin();
    IPySegmentor::TSegmentVec::const_iterator ite = segments.end();

    unsigned i, j = 0;
    for (; it != ite; ++it) {
        i = it->m_start;
        j = i + it->m_len;

        if (i < rebuildFrom - 1)
            continue;

        if (j >= m_lattice.capacity() - 1)
            break;

        if (it->m_type == IPySegmentor::SYLLABLE)
            _forwardSyllables(i, j, *it);
        else if (it->m_type == IPySegmentor::SYLLABLE_SEP)
            _forwardSyllableSep(i, j);
        else
            _forwardString(i, j, it->m_syllables);
        m_bOmitPunct = false;
    }

    _forwardTail(j, j + 1);
    m_tailIdx = j + 1;

    return doSearch && searchFrom(rebuildFrom);
}

void
CIMIContext::_forwardSyllables(unsigned i,
                               unsigned j,
                               const IPySegmentor::TSegment& seg)
{
    std::vector<unsigned>::const_iterator it = seg.m_syllables.begin();
    std::vector<unsigned>::const_iterator ite = seg.m_syllables.end();

    for (; it != ite; ++it)
        _forwardSingleSyllable(i, j, *it, seg);

    it = seg.m_fuzzy_syllables.begin();
    ite = seg.m_fuzzy_syllables.end();

    for (; it != ite; ++it)
        _forwardSingleSyllable(i, j, *it, seg, true);
}


void
CIMIContext::_forwardString(unsigned i,
                            unsigned j,
                            const std::vector<unsigned>& strbuf)
{
    if (strbuf.size() == 1) {
        unsigned ch = strbuf[0];
        if (ispunct(ch)) {
            _forwardPunctChar(i, j, ch);
        } else {
            _forwardOrdinaryChar(i, j, ch);
        }
    } else{
        CLatticeFrame &fr = m_lattice[j];
        fr.m_wstr.assign(strbuf.begin(), strbuf.end());
        fr.m_lexiconStates.push_back(TLexiconState(i, 0));
    }
}

void
CIMIContext::_forwardSingleSyllable(unsigned i,
                                    unsigned j,
                                    TSyllable syllable,
                                    const IPySegmentor::TSegment& seg,
                                    bool fuzzy)
{
    const CPinyinTrie::TNode * pn = NULL;

    CLatticeFrame &fr = m_lattice[j];
    fr.m_type = CLatticeFrame::SYLLABLE;

    CLexiconStates::iterator it = m_lattice[i].m_lexiconStates.begin();
    CLexiconStates::iterator ite = m_lattice[i].m_lexiconStates.end();
    for (; it != ite; ++it) {
        TLexiconState &lxst = *it;
        bool added_from_sysdict = false;

        if (lxst.m_pPYNode) {
            // try to match a word from lattice i to lattice j
            // and if match, we'll count it as a new lexicon on lattice j
            pn = m_pPinyinTrie->transfer(lxst.m_pPYNode, syllable);
            if (pn) {
                added_from_sysdict = true;
                TLexiconState new_lxst = TLexiconState(lxst.m_start,
                                                       pn,
                                                       lxst.m_syls,
                                                       lxst.m_seg_path,
                                                       fuzzy);
                new_lxst.m_syls.push_back(syllable);
                new_lxst.m_num_of_inner_fuzzies = lxst.m_num_of_inner_fuzzies +
                                                  (seg.m_inner_fuzzy ? 1 : 0);
                new_lxst.m_seg_path.push_back(seg.m_start + seg.m_len);
                fr.m_lexiconStates.push_back(new_lxst);
            }
        }

        if (m_pUserDict && lxst.m_syls.size() < MAX_USRDEF_WORD_LEN) {
            // try to match a word from user dict
            CSyllables syls = lxst.m_syls;
            syls.push_back(syllable);
            std::vector<CPinyinTrie::TWordIdInfo> words;
            m_pUserDict->getWords(syls, words);
            if (!words.empty() || !added_from_sysdict) {
                // even if the words is empty we'll add a fake lexicon
                // here. This helps _saveUserDict detect new words.
                TLexiconState new_lxst = TLexiconState(lxst.m_start,
                                                       words,
                                                       lxst.m_syls,
                                                       lxst.m_seg_path,
                                                       fuzzy);
                new_lxst.m_syls.push_back(syllable);
                new_lxst.m_num_of_inner_fuzzies = lxst.m_num_of_inner_fuzzies +
                                                  (seg.m_inner_fuzzy ? 1 : 0);
                new_lxst.m_seg_path.push_back(seg.m_start + seg.m_len);
                fr.m_lexiconStates.push_back(new_lxst);
            }
        }
    }

    // last, create a lexicon for single character with only one syllable
    pn = m_pPinyinTrie->transfer(syllable);
    if (pn) {
        CSyllables syls;
        syls.push_back(syllable);
        std::vector<unsigned> seg_path;
        seg_path.push_back(seg.m_start);
        seg_path.push_back(seg.m_start + seg.m_len);
        TLexiconState new_lxst = TLexiconState(i, pn, syls, seg_path, fuzzy);
        new_lxst.m_num_of_inner_fuzzies = seg.m_inner_fuzzy ? 1 : 0;
        fr.m_lexiconStates.push_back(new_lxst);
    }
}

void
CIMIContext::_forwardSyllableSep(unsigned i, unsigned j)
{
    CLatticeFrame &fr = m_lattice[j];
    fr.m_type = CLatticeFrame::SYLLABLE | CLatticeFrame::SYLLABLE_SEP;
    fr.m_lexiconStates = m_lattice[i].m_lexiconStates;

    CLexiconStates::iterator it = fr.m_lexiconStates.begin();
    CLexiconStates::iterator ite = fr.m_lexiconStates.end();
    for (; it != ite; ++it) {
        it->m_seg_path.back() = j;
    }
}

void
CIMIContext::_forwardPunctChar(unsigned i, unsigned j, unsigned ch)
{
    CLatticeFrame &fr = m_lattice[j];

    wstring wstr;
    unsigned wid = 0;

    if (m_pGetFullPunctOp) {
        if (m_bFullPunctForwarding && !m_bOmitPunct) {
            wstr = (*m_pGetFullPunctOp)(ch);
            wid = m_pPinyinTrie->getSymbolId(wstr);
        }
    }

    fr.m_type = CLatticeFrame::PUNC;

    if (!wstr.empty())
        fr.m_wstr = wstr;
    else
        fr.m_wstr.push_back(ch);

    fr.m_lexiconStates.push_back(TLexiconState(i, wid));
}

void
CIMIContext::_forwardOrdinaryChar(unsigned i, unsigned j, unsigned ch)
{
    CLatticeFrame &fr = m_lattice[j];

    wstring wstr;
    unsigned wid = 0;

    if (m_pGetFullSymbolOp) {
        wstr = (*m_pGetFullSymbolOp)(ch);
        wid = m_pPinyinTrie->getSymbolId(wstr);

        if (!m_bFullSymbolForwarding)
            wstr.clear();
    }

    fr.m_type = wid ? CLatticeFrame::SYMBOL : CLatticeFrame::ASCII;

    if (!wstr.empty())
        fr.m_wstr = wstr;
    else
        fr.m_wstr.push_back(ch);

    fr.m_lexiconStates.push_back(TLexiconState(i, wid));
}

void
CIMIContext::_forwardTail(unsigned i, unsigned j)
{
    CLatticeFrame &fr = m_lattice[j];
    fr.m_type = CLatticeFrame::TAIL;

    fr.m_lexiconStates.push_back(TLexiconState(i, ENDING_WORD_ID));
}

static double exp2_tbl[32] = {
    exp2(-0), exp2(-1), exp2(-2), exp2(-3), exp2(-4), exp2(-5), exp2(-6), exp2(-7),
    exp2(-8), exp2(-9), exp2(-10), exp2(-11), exp2(-12), exp2(-13), exp2(-14),
    exp2(-15), exp2(-16), exp2(-17), exp2(-18), exp2(-19), exp2(-20), exp2(-21),
    exp2(-22), exp2(-23), exp2(-24), exp2(-25), exp2(-26), exp2(-27), exp2(-28),
    exp2(-29), exp2(-30), exp2(-31)
};

bool
CIMIContext::searchFrom(unsigned idx)
{
    bool affectCandidates = (idx <= m_candiEnds);

    for (; idx <= m_tailIdx; ++idx) {
        CLatticeFrame &fr = m_lattice[idx];

        if (fr.m_type == CLatticeFrame::UNUSED)
            continue;

        fr.m_latticeStates.clear();

        /* user selected word might be cut in next step */
        if (fr.m_bwType & CLatticeFrame::USER_SELECTED) {
            _transferBetween(fr.m_selWord.m_start, idx,
                             fr.m_selWord.m_pLexiconState,
                             fr.m_selWord.m_wordId);
        }

        CLexiconStates::iterator it = fr.m_lexiconStates.begin();
        CLexiconStates::iterator ite = fr.m_lexiconStates.end();
        for (; it != ite; ++it) {
            unsigned word_num = 0;
            TLexiconState &lxst = *it;
            const CPinyinTrie::TWordIdInfo *words = lxst.getWords(word_num);

            if (!word_num)
                continue;

            if (lxst.m_start == m_candiStarts && idx > m_candiEnds)
                affectCandidates = true;

            // only selected the word with higher unigram probablities, and
            // narrow the search deepth and lower the initial score for fuzzy
            // syllables
            int maxsz = it->m_bFuzzy ? MAX_LEXICON_TRIES /
                        2 : MAX_LEXICON_TRIES;

            double ic = it->m_bFuzzy ? 0.5 : 1.0;

            int sz = (int) word_num < maxsz ? (int) word_num : maxsz;
            int i = 0, count = 0;

            while (count < sz && i < sz && (words[i].m_bSeen || count < 2)) {
                if (m_csLevel >= words[i].m_csLevel) {
                    // printf("cost %d\n", words[i].m_cost);
                    _transferBetween(lxst.m_start, idx, &lxst, words[i].m_id,
                                     ic * exp2_tbl[words[i].m_cost]);
                    ++count;
                }
                i++;
            }

            /* try extra words in history cache */
            if (m_pHistory) {
                while (i < (int) word_num) {
                    if (m_csLevel >= words[i].m_csLevel
                        && m_pHistory->seenBefore(words[i].m_id)) {
                        // printf("history cost %d\n", words[i].m_cost);
                        _transferBetween(lxst.m_start, idx, &lxst,
                                         words[i].m_id,
                                         ic * exp2_tbl[words[i].m_cost]);
                    }
                    i++;
                }
            }
        }
    }

    _clearPaths();
    m_path.clear();
    m_segPath.clear();
    m_nBest = 0;

    std::vector<TLatticeState> tail_states =
        m_lattice[m_tailIdx].m_latticeStates.getFilteredResult();

#ifdef DEBUG
    for (int i = 0; i < tail_states.size(); i++) {
        std::string score;
        tail_states[i].m_score.toString(score);
        printf("score[%d]: %s\n", i, score.c_str());
    }
#endif

    for (size_t i = 0; i < m_maxBest; i++) {
        TPath path, segpath;
        if (_backTracePaths(tail_states, m_nBest, path, segpath)) {
            m_path.push_back(path);
            m_segPath.push_back(segpath);
            m_nBest++;
        }
    }

    if (m_pPySegmentor && m_nBest > 0 && !m_segPath[0].empty())
        m_pPySegmentor->notify_best_segpath(m_segPath[0]);

    return affectCandidates;
}

void
CIMIContext::_transferBetween(unsigned start, unsigned end,
                              TLexiconState* plxst, unsigned wid,
                              double ic)
{
    CLatticeFrame &start_fr = m_lattice[start];
    CLatticeFrame &end_fr = m_lattice[end];

    TLatticeState node(-1.0, end, plxst);
    TSentenceScore efic(ic);

    if ((end_fr.m_bwType & CLatticeFrame::USER_SELECTED)
        && end_fr.m_selWord.m_wordId == wid)
        efic = TSentenceScore(30000, 1.0);

    static double s_history_distribution[] = {
        0.0, 0.05, 0.10, 0.15, 0.20, 0.25, 0.30, 0.35, 0.40, 0.45, 0.50
    };

    double weight_h = s_history_distribution[m_historyPower];
    double weight_s = 1.0 - weight_h;

    CLatticeStates::iterator it = start_fr.m_latticeStates.begin();
    CLatticeStates::iterator ite = start_fr.m_latticeStates.end();

    for (; it != ite; ++it) {
        // for 1-length lattice states, replace ending_word_id (comma)
        // with none_word_id (recognized by CThreadSlm)
    unsigned _wid = wid;
        if (wid == ENDING_WORD_ID && it->m_pBackTraceNode && it->m_pBackTraceNode->m_frIdx == 0)
            _wid = NONE_WORD_ID;

        node.m_pBackTraceNode = &(*it);
        node.m_backTraceWordId = wid;

        double ts = m_pModel->transfer(it->m_slmState, _wid, node.m_slmState);
        m_pModel->historify(node.m_slmState);

        // backward to pseudo root, so wid is probably a user word,
        // save the wid in idx field, so that later we could get it via
        // CThreadSlm::lastWordId, to calculate p_{cache} correctly.
        if (node.m_slmState.getLevel() == 0
            && m_pHistory && m_pHistory->seenBefore(wid))
            node.m_slmState.setIdx(wid);  // an pseudo unigram node state

        if (m_pHistory) {
            unsigned history[2] = { m_pModel->lastWordId(it->m_slmState), _wid };
            double hpr = m_pHistory->pr(history, history + 2);
            ts = weight_s * ts + weight_h * hpr;
        }

        node.m_score = it->m_score * efic * TSentenceScore(ts);
        // std::string buf;
        // node.m_score.toString(buf);
        // printf("node score %s ts=%lf ", buf.c_str(), ts);
        // it->m_score.toString(buf);
        // printf("%s ic=%lf\n", buf.c_str(), ic);
        end_fr.m_latticeStates.add(node);
    }
}

bool
CIMIContext::_backTracePaths(const std::vector<TLatticeState>& tail_states,
                             int rank, TPath& path, TPath& segmentPath)
{
    path.clear();
    segmentPath.clear();

    if (rank >= (int) tail_states.size()) {
        // rank out of bounds, only return the segment path
        return false;
    }

    const TLatticeState *bs = &(tail_states[rank]);

    while (bs->m_pBackTraceNode) {
        unsigned start = bs->m_pBackTraceNode->m_frIdx;
        unsigned end = bs->m_frIdx;
        CLatticeFrame & end_fr = m_lattice[end];

        if (!(end_fr.m_bwType & CLatticeFrame::USER_SELECTED)) {
            const TWCHAR* cwstr = NULL;
            if (end_fr.m_wstr.empty()) {
                cwstr = _getWstr(bs->m_backTraceWordId);
            } else {
                cwstr = end_fr.m_wstr.c_str();
            }

            CCandidate candi(start, end, bs->m_pLexiconState, cwstr,
                             bs->m_backTraceWordId);

            end_fr.m_bwType |= CLatticeFrame::BESTWORD;
            end_fr.m_bestWords[rank] = candi;
            if (rank == 0) {
                end_fr.m_selWord = candi; // select the first by default.
            }
        }

        if (bs->m_pBackTraceNode->m_pLexiconState) {
            std::vector<unsigned> seg_path =
                bs->m_pBackTraceNode->m_pLexiconState->m_seg_path;
            std::vector<unsigned>::reverse_iterator it = seg_path.rbegin();

            for (; it != seg_path.rend(); ++it) {
                if (segmentPath.empty() || segmentPath.back() != *it)
                    segmentPath.push_back(*it);
            }
        }

        path.push_back(end);
        bs = bs->m_pBackTraceNode;
    }

    std::reverse(path.begin(), path.end());
    std::reverse(segmentPath.begin(), segmentPath.end());

#ifdef DEBUG
    std::vector<unsigned>::iterator it;

    printf("trace lattice path[%d]: ", rank);
    for (it = path.begin(); it != path.end(); ++it)
        printf("%d ", *it);
    printf("\n");

    printf("trace segments path[%d]: ", rank);
    for (it = segmentPath.begin(); it != segmentPath.end(); ++it)
        printf("%d ", *it);
    printf("\n");
#endif

    return true;
}

void
CIMIContext::_clearPaths()
{
    m_path.clear();
    m_segPath.clear();
}

std::vector<CCandidates>
CIMIContext::getBestSentenceTails(int rank, unsigned start, unsigned end)
{
    std::vector<CCandidates> result;
    if (rank < 0) {
        return result;
    }

    CCandidates sentence;
    unsigned word_num = getBestSentence(sentence, rank, start, end);
    unsigned tail_word_num = word_num;

    while (tail_word_num > 1) {
        unsigned dec = tail_word_num / (m_maxTailCandidateNum + 1) + 1;
        tail_word_num -= std::min(dec, tail_word_num);
        if (tail_word_num <= 1) {
            break;
        }
        CCandidates tail(sentence.begin(), sentence.begin() + tail_word_num);
        result.push_back(tail);
    }
    return result;
}

unsigned
CIMIContext::getBestSentence(CCandidates& result, int rank,
                             unsigned start, unsigned end)
{
    // -1 means selected sentence
    if (rank < -1 || rank >= (int) m_nBest)
        return 0;

    result.clear();

    if (end == UINT_MAX)
        end = m_tailIdx - 1;

    while (end > start && m_lattice[end].m_bwType == CLatticeFrame::NO_BESTWORD)
        end--;

    unsigned i = end, nWordConverted = 0;
    while (i > start) {
        CLatticeFrame& fr = m_lattice[i];
        if (rank < 0) {
            result.insert(result.begin(), fr.m_selWord);
            i = fr.m_selWord.m_start;
        } else {
            result.insert(result.begin(), fr.m_bestWords[rank]);
            i = fr.m_bestWords[rank].m_start;
        }
        nWordConverted++;
    }
    return nWordConverted;
}

unsigned
CIMIContext::getBestSentence(wstring& result, int rank,
                             unsigned start, unsigned end)
{
    CCandidates sentence;
    unsigned nWordConverted = getBestSentence(sentence, rank, start, end);
    result.clear();
    for (size_t i = 0; i < sentence.size(); i++) {
        result += sentence[i].m_cwstr;
    }
    return nWordConverted;
}

unsigned
CIMIContext::getBestSentence(std::vector<unsigned>& result, int rank,
                             unsigned start, unsigned end)
{
    CCandidates sentence;
    unsigned nWordConverted = getBestSentence(sentence, rank, start, end);
    result.clear();
    for (size_t i = 0; i < sentence.size(); i++) {
        result.push_back(sentence[i].m_wordId);
    }
    return nWordConverted;
}


unsigned
CIMIContext::getSelectedSentence(wstring& result,
                                 unsigned start, unsigned end)
{
    return getBestSentence(result, -1, start, end);
}


unsigned
CIMIContext::getSelectedSentence(std::vector<unsigned>& result,
                                 unsigned start, unsigned end)
{
    return getBestSentence(result, -1, start, end);
}

struct TCandiPair {
    CCandidate m_candi;
    TCandiRank m_Rank;

    TCandiPair() : m_candi(), m_Rank()
    {
    }
};

struct TCandiPairPtr {
    TCandiPair*                     m_Ptr;

    TCandiPairPtr(TCandiPair* p = NULL) : m_Ptr(p)
    {
    }

    bool
    operator<(const TCandiPairPtr& b) const
    {
        return m_Ptr->m_Rank < b.m_Ptr->m_Rank;
    }
};

const TWCHAR *
CIMIContext::_getWstr(unsigned wid)
{
    if (wid < m_pPinyinTrie->getWordCount())
        return (*m_pPinyinTrie)[wid];
    else if (m_pUserDict)
        return (*m_pUserDict)[wid];
    else
        return NULL;
}

void
CIMIContext::getCandidates(unsigned frIdx, CCandidates& result)
{
    TCandiPair cp;
    static std::map<wstring, TCandiPair> candidates_map;
    std::map<wstring, TCandiPair>::iterator candidates_it;

    candidates_map.clear();
    result.clear();

    std::vector<unsigned> st;
    getSelectedSentence(st, frIdx);

    cp.m_candi.m_start = m_candiStarts = frIdx++;

    for (; frIdx < m_tailIdx; ++frIdx) {
        if (m_lattice[frIdx + 1].isSyllableSepFrame())
            continue;

        CLatticeFrame &fr = m_lattice[frIdx];
        if (!fr.isSyllableFrame())
            continue;

        cp.m_candi.m_end = frIdx;
        if (fr.m_bwType != CLatticeFrame::NO_BESTWORD) {
            for (size_t i = 0; i < m_nBest; i++) {
                if (fr.m_bestWords.find(i) == fr.m_bestWords.end())
                    continue;
                CCandidate candi = fr.m_bestWords[i];
                if (candi.m_start != m_candiStarts)
                    continue;
                if (candi.m_pLexiconState == NULL)
                    continue;

                TLexiconState & lxst = *(candi.m_pLexiconState);
                int len = lxst.m_syls.size() - lxst.m_num_of_inner_fuzzies;
                if (len == 0) len = 1;

                cp.m_candi = candi;
                cp.m_Rank =
                    TCandiRank(fr.m_bwType & CLatticeFrame::USER_SELECTED,
                               fr.m_bwType & CLatticeFrame::BESTWORD,
                               len, false, 0);
                candidates_map[candi.m_cwstr] = cp;
            }
        }

        bool found = false;
        CLexiconStates::iterator it = fr.m_lexiconStates.begin();
        CLexiconStates::iterator ite = fr.m_lexiconStates.end();
        for (; it != ite; ++it) {
            TLexiconState & lxst = *it;

            if (lxst.m_start != m_candiStarts)
                continue;

            int len = lxst.m_syls.size() - lxst.m_num_of_inner_fuzzies;
            if (0 == len) len = 1;

            found = true;
            unsigned word_num;
            const CPinyinTrie::TWordIdInfo *words = lxst.getWords(word_num);

            for (unsigned i = 0; i < word_num; ++i) {
                if (m_csLevel < words[i].m_csLevel)
                    continue;

                cp.m_candi.m_wordId = words[i].m_id;
                cp.m_candi.m_cwstr = _getWstr(cp.m_candi.m_wordId);
                cp.m_candi.m_pLexiconState = &lxst;
                if (!cp.m_candi.m_cwstr)
                    continue;

                //sorting according to the order in PinYinTire
                cp.m_Rank =
                    TCandiRank(false,
                               !st.empty() && st.front() == cp.m_candi.m_wordId,
                               len, false, i);
                candidates_it = candidates_map.find(cp.m_candi.m_cwstr);
                if (candidates_it == candidates_map.end()
                    || cp.m_Rank < candidates_it->second.m_Rank
                    || cp.m_candi.m_wordId > INI_USRDEF_WID) {
                    candidates_map[cp.m_candi.m_cwstr] = cp;
                    // print_wide(cp.m_candi.m_cwstr);
                    // printf(" ");
                }
            }
            // puts("");
        }

        if (!found) continue;  // FIXME: need better solution later

        if (m_bDynaCandiOrder) {
            CLatticeStates::iterator it = fr.m_latticeStates.begin();
            CLatticeStates::iterator ite = fr.m_latticeStates.end();
            // printf("adjusting ");
            for (; it != ite; ++it) {
                TLatticeState & ltst = *it;

                if (ltst.m_pBackTraceNode->m_frIdx != m_candiStarts)
                    continue;

                cp.m_candi.m_wordId = ltst.m_backTraceWordId;
                cp.m_candi.m_cwstr = _getWstr(cp.m_candi.m_wordId);
                cp.m_candi.m_pLexiconState = ltst.m_pLexiconState;
                if (!cp.m_candi.m_cwstr)
                    continue;

                int len = cp.m_candi.m_pLexiconState->m_syls.size() -
                          cp.m_candi.m_pLexiconState->m_num_of_inner_fuzzies;
                if (0 == len) len = 1;
                cp.m_Rank = TCandiRank(false,
                                       !st.empty() && st.front() ==
                                       cp.m_candi.m_wordId,
                                       len, true, ltst.m_score /
                                       ltst.m_pBackTraceNode->m_score);
                candidates_it = candidates_map.find(cp.m_candi.m_cwstr);
                if (candidates_it == candidates_map.end()
                    || cp.m_Rank < candidates_it->second.m_Rank
                    || cp.m_candi.m_wordId > INI_USRDEF_WID) {
                    // print_wide(cp.m_candi.m_cwstr);
                    // std::string buf;
                    // ltst.m_score.toString(buf);
                    // printf("len:%d %s", len, buf.c_str());
                    // ltst.m_pBackTraceNode->m_score.toString(buf);
                    // printf("%s ", buf.c_str());
                    candidates_map[cp.m_candi.m_cwstr] = cp;
                }
            }
            // puts("");
        }

        m_candiEnds = frIdx;
    }

    std::vector<TCandiPairPtr> vec;

    vec.reserve(candidates_map.size());
    for (candidates_it = candidates_map.begin();
         candidates_it != candidates_map.end(); ++candidates_it) {
        vec.push_back(TCandiPairPtr(&(candidates_it->second)));
    }

    std::sort(vec.begin(), vec.end());
    for (size_t i = 0; i < vec.size(); i++) {
        // print_wide(vec[i].m_Ptr->m_candi.m_cwstr);
        // printf(" ");
        result.push_back(vec[i].m_Ptr->m_candi);
    }
    // puts("");
}

unsigned
CIMIContext::cancelSelection(unsigned frIdx, bool doSearch)
{
    unsigned ret = frIdx;

    CLatticeFrame &fr = m_lattice[frIdx];
    while (fr.m_bwType & CLatticeFrame::IGNORED) {
        --frIdx;
        fr = m_lattice[frIdx];
    }

    if (fr.m_bwType &
        (CLatticeFrame::USER_SELECTED | CLatticeFrame::BESTWORD)) {
        ret = fr.m_selWord.m_start;
        fr.m_bwType = CLatticeFrame::NO_BESTWORD;
        if (doSearch) searchFrom(frIdx);
    }

    return ret;
}

void
CIMIContext::makeSelection(CCandidate &candi, bool doSearch)
{
    CLatticeFrame &fr = m_lattice[candi.m_end];
    fr.m_bwType = fr.m_bwType | CLatticeFrame::USER_SELECTED;
    fr.m_selWord = candi;
    // make best sentence word consistent as well
    for (size_t i = 0; i < m_nBest; i++) {
        fr.m_bestWords[i] = candi;
    }

    if (doSearch) searchFrom(candi.m_end);
}

void
CIMIContext::selectSentence(int idx)
{
    unsigned i = m_tailIdx - 1;
    while (i > 0 && m_lattice[i].m_bwType == CLatticeFrame::NO_BESTWORD)
        i--;

    while (i > 0) {
        CLatticeFrame &fr = m_lattice[i];
        fr.m_selWord = fr.m_bestWords[idx];
        i = fr.m_selWord.m_start;
    }
}

void
CIMIContext::memorize()
{
    _saveUserDict();
    _saveHistoryCache();
}

void
CIMIContext::_saveUserDict()
{
    if (!m_pUserDict)
        return;

    CSyllables syls;
    bool has_user_selected = false;
    unsigned i = m_tailIdx - 1;
    unsigned e_pos = 0;

    while (i > 0 && m_lattice[i].m_bwType == CLatticeFrame::NO_BESTWORD)
        i--;

    while (i > 0) {
        CLatticeFrame &fr = m_lattice[i];
        if (!fr.isSyllableFrame()) {
            i = fr.m_selWord.m_start;
            break;
        }

        TLexiconState* state = fr.m_selWord.m_pLexiconState;
        if (!state) {
            i = fr.m_selWord.m_start;
            continue;
        }

        if (syls.size() + state->m_syls.size() > MAX_USRDEF_WORD_LEN) {
            i = fr.m_selWord.m_start;
            break;
        }

    if (!e_pos) e_pos = i;

        has_user_selected |= (fr.m_bwType & CLatticeFrame::USER_SELECTED);
        std::copy(state->m_syls.begin(), state->m_syls.end(), inserter(syls, syls.begin()));
        i = fr.m_selWord.m_start;
    }

    if (has_user_selected && syls.size() > 1) {
        wstring phrase;
        getSelectedSentence (phrase, 0, e_pos);
        m_pUserDict->addWord (syls, phrase);
    }
}

void
CIMIContext::_saveHistoryCache()
{
    if (!m_pHistory)
        return;

    std::vector<unsigned> result;
    unsigned i = m_tailIdx - 1;
    while (i > 0 && m_lattice[i].m_bwType == CLatticeFrame::NO_BESTWORD)
        i--;

    while (i > 0) {
        CLatticeFrame &fr = m_lattice[i];
        if (fr.isSyllableFrame()) {
            result.insert(result.begin(), fr.m_selWord.m_wordId);
        } else {
            result.insert(result.begin(), 0);
        }
        i = fr.m_selWord.m_start;
    }

    if (!result.empty()) {
        m_pHistory->memorize(&(result[0]), &(result[0]) + result.size());
        m_pHistory->saveToFile();
    }
}

void
CIMIContext::deleteCandidate(CCandidate &candi)
{
    unsigned wid = candi.m_wordId;
    deleteCandidateByWID(wid);
}

void
CIMIContext::deleteCandidateByWID(unsigned wid)
{
    if (wid > INI_USRDEF_WID) {
        m_pHistory->forget(wid);
        m_pUserDict->removeWord(wid);
        _buildLattice(m_pPySegmentor->getSegments());
    }
}

void
CIMIContext::removeFromHistoryCache(std::vector<unsigned>& wids)
{
    if (!m_pHistory)
        return;

    m_pHistory->forget(&(wids[0]), &(wids[0]) + wids.size());
    buildLattice(m_pPySegmentor);
}

// -*- indent-tabs-mode: nil -*- vim:et:ts=4

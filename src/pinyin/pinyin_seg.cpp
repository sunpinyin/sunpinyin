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

#include <cassert>
#include <functional>
#include <algorithm>
#include "pinyin_seg.h"
#include "quanpin_trie.h"

const char *
CGetCorrectionPairOp::operator ()(std::string& pystr, unsigned& matched_len)
{
    CCorrectionPairVec::iterator it = m_correctionPairs.begin();
    CCorrectionPairVec::iterator ite = m_correctionPairs.end();

    for (; it != ite; ++it) {
        std::string& k = it->first;
        std::string& v = it->second;
        unsigned l = k.size();

        if (pystr.size() >= l && !pystr.compare(pystr.size() - l, l, k)) {
            matched_len = l;
            return v.c_str();
        }
    }

    return NULL;
}

void
CGetFuzzySegmentsOp::_initMaps()
{
    unsigned num_of_fuzzy_finals;
    const unsigned * fuzzy_final_map = CPinyinData::getInnerFuzzyFinalMap(
        num_of_fuzzy_finals);

    for (size_t i = 0; i < num_of_fuzzy_finals; ++i) {
        unsigned f = *(fuzzy_final_map++);
        unsigned _f = *(fuzzy_final_map++);
        unsigned l = *(fuzzy_final_map++);

        m_fuzzyFinalMap.insert(std::make_pair(f, std::make_pair(_f, l)));
    }

    const unsigned *fuzzy_pre_syls, *fuzzy_pro_syls;
    CPinyinData::getFuzzyPreProSyllables(&fuzzy_pre_syls, &fuzzy_pro_syls);

    while (*fuzzy_pre_syls) {
        unsigned s = *(fuzzy_pre_syls++);
        char c = *(fuzzy_pre_syls++);
        unsigned _s = *(fuzzy_pre_syls++);
        m_fuzzyPreMap.insert(std::make_pair(s, std::make_pair(c, _s)));
    }

    while (*fuzzy_pro_syls) {
        unsigned s = *(fuzzy_pro_syls++);
        char c = *(fuzzy_pro_syls++);
        unsigned _s = *(fuzzy_pro_syls++);
        m_fuzzyProMap.insert(std::make_pair(s, std::make_pair(c, _s)));
    }
}

unsigned
CGetFuzzySegmentsOp::_invalidateSegments(IPySegmentor::TSegmentVec& fuzzy_segs,
                                         IPySegmentor::TSegment& seg)
{
    unsigned invalidatedFrom = UINT_MAX;

    IPySegmentor::TSegmentVec::reverse_iterator it = fuzzy_segs.rbegin();
    IPySegmentor::TSegmentVec::reverse_iterator ite = fuzzy_segs.rend();

    for (; it != ite; it += 2) {
        IPySegmentor::TSegment& seg1 = *(it + 1);
        IPySegmentor::TSegment& seg2 = *it;

        unsigned r = seg2.m_start + seg2.m_len;
        if (r <= seg.m_start)
            break;

        invalidatedFrom = seg1.m_start;
    }

    fuzzy_segs.erase(it.base(), fuzzy_segs.end());

    return invalidatedFrom;
}

unsigned
CGetFuzzySegmentsOp::operator ()(IPySegmentor::TSegmentVec& segs,
                                 IPySegmentor::TSegmentVec& fuzzy_segs,
                                 wstring& input)
{
    IPySegmentor::TSegment&  seg = segs.back();
    unsigned invalidatedFrom = _invalidateSegments(fuzzy_segs, seg);

    unsigned updatedFrom = UINT_MAX;
    TSyllable syl = (TSyllable)seg.m_syllables[0];

    if (m_bInnerFuzzyEnabled) { // xian -> xian, xi'an
        CInnerFuzzyFinalMap::iterator it = m_fuzzyFinalMap.find(syl.final);

        if (it != m_fuzzyFinalMap.end()) {
            unsigned an_syl = it->second.first;
            unsigned an_len = it->second.second;

            unsigned xi_len = seg.m_len - an_len;
            wstring wstr = input.substr(seg.m_start, xi_len);

#ifndef _RW_STD_STL
            std::string xi_str(wstr.begin(), wstr.end());
#else
            std::string xi_str;
            for (wstring::iterator it = wstr.begin(); it != wstr.end(); ++it)
                xi_str.push_back(*it);
#endif

            unsigned xi_syl = CPinyinData::encodeSyllable(xi_str.c_str());

            if (0 == xi_syl)
                goto RETURN;

            IPySegmentor::TSegment xi = segs.back();
            xi.m_len = xi_len;
            xi.m_syllables[0] = xi_syl;

            IPySegmentor::TSegment an = segs.back();
            an.m_len = an_len;
            an.m_start += xi_len;
            an.m_syllables[0] = an_syl;
            an.m_inner_fuzzy = true;

            fuzzy_segs.push_back(xi);
            fuzzy_segs.push_back(an);

            updatedFrom = xi.m_start;
            goto RETURN;
        }
    }

    if (segs.size() >= 2) { // fangan -> fang'an, fan'gan
        IPySegmentor::TSegment& pre_seg = *(segs.end() - 2);

        CFuzzySyllableMap::iterator pre_it = m_fuzzyPreMap.find(
            pre_seg.m_syllables[0]);
        CFuzzySyllableMap::iterator it = m_fuzzyProMap.find(syl);

        if (pre_it != m_fuzzyPreMap.end() && it != m_fuzzyProMap.end() &&
            pre_it->second.first == it->second.first) {
            IPySegmentor::TSegment fang = segs[segs.size() - 2];
            fang.m_len++;
            fang.m_syllables[0] = pre_it->second.second;

            IPySegmentor::TSegment an = segs.back();
            an.m_start++;
            an.m_len--;
            an.m_syllables[0] = it->second.second;

            fuzzy_segs.push_back(fang);
            fuzzy_segs.push_back(an);

            updatedFrom = fang.m_start;
            goto RETURN;
        }
    }

RETURN:;

    return std::min(updatedFrom, invalidatedFrom);
}


CQuanpinSegmentor::CQuanpinSegmentor ()
    : m_pGetFuzzySyllablesOp(NULL),
      m_pGetCorrectionPairOp(NULL),
      m_pGetFuzzySegmentsOp(NULL),
      m_pytrie(base, check, value, sizeof(base) / sizeof(*base)),
      m_updatedFrom(0)
{
    m_segs.reserve(32);
}

bool
CQuanpinSegmentor::load(const char * pyTrieFileName)
{
    return m_pytrie.load(pyTrieFileName);
}

#ifdef DEBUG
void
print_pystr(const std::string pystr)
{
    for (const char* c = pystr.c_str();
         c != pystr.c_str() + pystr.length();
         ++c) {
        printf("%c", *c & 0x7f);
    }
    printf("<\n");
}
#endif

unsigned
CQuanpinSegmentor::push(unsigned ch)
{
    m_inputBuf.push_back(ch);

    if (m_pGetCorrectionPairOp && m_pGetCorrectionPairOp->isEnabled()) {
        m_pystr.push_back(ch);
        unsigned l = 0;
        const char * v = (*m_pGetCorrectionPairOp)(m_pystr, l);

        if (v) {
            unsigned orig_size = m_segs.size();
            _clear(m_pystr.size() - l);
            m_updatedFrom = _updateWith(v);

            if (m_segs.size() >= orig_size) {
                // does not get better segmentation, revert to original
                _clear(m_pystr.size() - strlen(v));
                std::string new_pystr;
                std::copy(m_inputBuf.end() - l, m_inputBuf.end(),
                          back_inserter(new_pystr));
                m_updatedFrom = _updateWith(new_pystr);
            } else {
                if (l != strlen(v)) {
                    // e.g. uen -> un
                    m_segs.back().m_len += l - strlen(v);
                    m_pystr.resize(m_inputBuf.length());
                }
                std::copy(m_inputBuf.end() - l, m_inputBuf.end(),
                          m_pystr.end() - l);
            }
            return m_updatedFrom;
        }

        m_pystr.resize(m_pystr.size() - 1);
    }

    return m_updatedFrom = _push(ch);
}

unsigned
CQuanpinSegmentor::pop()
{
    if (m_pystr.empty())
        return m_updatedFrom = 0;

    unsigned size = m_inputBuf.size();
    m_inputBuf.resize(size - 1);
    m_pystr.resize(size - 1);

    unsigned l = m_segs.back().m_len;
    m_segs.pop_back();

    if (l == 1)
        return m_updatedFrom = size - 1;

    std::string new_pystr = m_pystr.substr(size - l);
    m_pystr.resize(size - l);

    m_updatedFrom = _updateWith(new_pystr);

    return m_updatedFrom;
}

unsigned
CQuanpinSegmentor::insertAt(unsigned idx, unsigned ch)
{
    unsigned i, j;
    _locateSegment(idx, i, j);

    m_inputBuf.insert(idx, 1, ch);
    m_pystr.insert(idx, 1, ch);

    std::string new_pystr = m_pystr.substr(i);
    m_pystr.resize(i);
    m_segs.erase(m_segs.begin() + j, m_segs.end());

    m_updatedFrom = _updateWith(new_pystr);

    return m_updatedFrom;
}

unsigned
CQuanpinSegmentor::deleteAt(unsigned idx, bool backward)
{
    unsigned i, j;
    if (!backward) idx += 1;
    _locateSegment(idx, i, j);

    m_inputBuf.erase(idx, 1);
    m_pystr.erase(idx, 1);

    std::string new_pystr = m_pystr.substr(i);
    m_pystr.resize(i);
    m_segs.erase(m_segs.begin() + j, m_segs.end());

    m_updatedFrom = _updateWith(new_pystr);

    return m_updatedFrom;
}

unsigned
CQuanpinSegmentor::clear(unsigned from)
{
    m_inputBuf.resize(from);
    return _clear(from);
}

unsigned
CQuanpinSegmentor::_clear(unsigned from)
{
    unsigned i, j;
    _locateSegment(from, i, j);


    std::string new_pystr = m_pystr.substr(i, from - i);
    m_pystr.resize(i);
    m_segs.erase(m_segs.begin() + j, m_segs.end());

    m_updatedFrom = _updateWith(new_pystr, from);

    return m_updatedFrom;
}

void
CQuanpinSegmentor::_locateSegment(unsigned idx,
                                  unsigned &strIdx,
                                  unsigned &segIdx)
{
    strIdx = segIdx = 0;

    TSegmentVec::iterator it = m_segs.begin();
    TSegmentVec::iterator ite = m_segs.end();

    for (; it != ite; ++it) {
        if (strIdx + (*it).m_len > idx)
            break;

        strIdx += (*it).m_len;
        segIdx += 1;
    }
}

unsigned
CQuanpinSegmentor::_push(unsigned ch)
{
    unsigned l, ret;
    m_pystr.push_back(ch);
    int v = m_pytrie.match_longest(m_pystr.rbegin(), m_pystr.rend(), l);

    if (l == 0) { // not a valid syllable character, e.g., \', i, u, or A-Z
        IPySegmentor::ESegmentType seg_type;
        if (ch == '\'' && m_inputBuf.size() > 1)
            seg_type = IPySegmentor::SYLLABLE_SEP;
        else if (islower(ch))
            seg_type = IPySegmentor::INVALID;
        else
            seg_type = IPySegmentor::STRING;

        ret = m_pystr.size() - 1;
        m_segs.push_back(TSegment(ch, ret, 1, seg_type));
    } else if (l == 1) { // possible a new segment
        int last_idx = m_pystr.size() - 2;
        if (last_idx >= 0 && (m_pystr[last_idx] & 0x80)) {
            // check if the last syllable character's highest bitmask is set
            // e.g., feN, so [feN] + g -> [feng]
            m_pystr[last_idx] &= 0x7f;
            unsigned l;
            int v = m_pytrie.match_longest(m_pystr.rbegin(), m_pystr.rend(), l);

            TSegment &last_seg = m_segs.back();
            if (l == (unsigned) last_seg.m_len + 1) {
                last_seg.m_len += 1;
                last_seg.m_syllables[0] = v;
                ret = m_pystr.size() - l;
                goto RETURN;
            }

            // in case not extensible, change highest bitmask back
            m_pystr[last_idx] |= 0x80;
        }

        // push the new 1-length segment
        ret = m_pystr.size() - 1;
        m_segs.push_back(TSegment(v, ret, 1));
    } else if (l == (unsigned) m_segs.back().m_len + 1) {
        // current segment is extensible, e.g., [xia] + n -> [xian]
        TSegment &last_seg = m_segs.back();
        last_seg.m_len += 1;
        last_seg.m_syllables[0] = v;
        ret = m_pystr.size() - l;
    } else {  // other cases
        TSegment &last_seg = m_segs.back();
        int i = 0, isum = last_seg.m_len + 1, lsum = l;
        TSegmentVec new_segs(1, TSegment(v, m_pystr.size() - l, l));

        // e.g., [zh] [o] [n] + g -> [zhonG],
        if (isum < lsum) {
            unsigned end_idx = m_pystr.size() - 1;
            m_pystr[end_idx] |= 0x80;
        }

        while (isum != lsum) {
            if (lsum < isum) { // e.g., [die] + r -> [di] [er]
                v = m_pytrie.match_longest(
                    m_pystr.rbegin() + lsum, m_pystr.rend(), l);
                TSegment &last_seg = new_segs.back();
                new_segs.push_back(TSegment(v, last_seg.m_start - l, l));
                _addFuzzySyllables(new_segs.back());
                lsum += l;
            } else {
                i += 1;
                isum += (m_segs.rbegin() + i)->m_len;
            }
        }

        m_segs.erase(m_segs.end() - (i + 1), m_segs.end());
        std::copy(new_segs.rbegin(), new_segs.rend(), back_inserter(m_segs));
        ret = m_pystr.size() - lsum;
    }

RETURN:;

    if (m_pGetFuzzySegmentsOp && m_pGetFuzzySegmentsOp->isEnabled())
        ret =
            std::min(ret,
                     (*m_pGetFuzzySegmentsOp)(m_segs, m_fuzzy_segs, m_inputBuf));

    if (m_pGetFuzzySyllablesOp && m_pGetFuzzySyllablesOp->isEnabled()) {
        if (m_segs.back().m_type == SYLLABLE)
            _addFuzzySyllables(m_segs.back());

        if (m_fuzzy_segs.size()) {
            _addFuzzySyllables(*(m_fuzzy_segs.end() - 1));
            _addFuzzySyllables(*(m_fuzzy_segs.end() - 2));
        }
    }

    return ret;
}

void
CQuanpinSegmentor::_addFuzzySyllables(TSegment& seg)
{
    assert(seg.m_type == SYLLABLE);

    seg.m_fuzzy_syllables.clear();

    CSyllables fuzzy_set = (*m_pGetFuzzySyllablesOp)(seg.m_syllables.front());
    CSyllables::const_iterator it = fuzzy_set.begin();
    CSyllables::const_iterator ite = fuzzy_set.end();

    for (; it != ite; ++it)
        seg.m_fuzzy_syllables.push_back(*it);
}

unsigned
CQuanpinSegmentor::_updateWith(const std::string& new_pystr, unsigned from)
{
    unsigned minUpdatedFrom = from;
    std::string::const_iterator it = new_pystr.begin();
    for (; it != new_pystr.end(); ++it) {
        unsigned updatedFrom = _push(*it & 0x7f);

        if (updatedFrom < minUpdatedFrom) minUpdatedFrom = updatedFrom;
    }
    return minUpdatedFrom;
}

// -*- indent-tabs-mode: nil -*- vim:et:ts=4

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

#ifndef SUNPY_PINYIN_SEG_H
#define SUNPY_PINYIN_SEG_H

#include "portability.h"
#include "segmentor.h"
#include "pinyin_data.h"
#include "datrie.h"
#include "ime-core/utils.h"

#include <algorithm>
#include <climits>

class CGetCorrectionPairOp : private CNonCopyable
{
public:
    typedef std::pair<std::string, std::string> CCorrectionPair;
    typedef std::vector<CCorrectionPair> CCorrectionPairVec;

    CGetCorrectionPairOp () : m_bEnabled(false) { m_correctionPairs.reserve(8); }

    void setEnable(bool value = true) { m_bEnabled = value; }
    bool isEnabled() { return m_bEnabled; }

    void setCorrectionPairs(const string_pairs& pairs)
    { m_correctionPairs = pairs; }

    const char * operator ()(std::string& pystr, unsigned& matched_len);

private:
    CCorrectionPairVec m_correctionPairs;
    bool m_bEnabled;
};

class CGetFuzzySegmentsOp : private CNonCopyable
{
public:
    typedef std::map<unsigned,
                     std::pair<unsigned, unsigned> > CInnerFuzzyFinalMap;
    typedef std::map<unsigned, std::pair<char, unsigned> > CFuzzySyllableMap;

    CGetFuzzySegmentsOp () : m_bEnabled(false),
                             m_bInnerFuzzyEnabled(false) { _initMaps(); }
    unsigned operator ()(IPySegmentor::TSegmentVec&,
                         IPySegmentor::TSegmentVec&,
                         wstring&);

    void setEnable(bool value = true) { m_bEnabled = value; }
    void setInnerFuzzyEnable(bool value = true) { m_bInnerFuzzyEnabled = value; }
    bool isEnabled() { return m_bEnabled; }

private:
    void        _initMaps();
    unsigned    _invalidateSegments(IPySegmentor::TSegmentVec&,
                                    IPySegmentor::TSegment&);

    bool m_bEnabled;
    bool m_bInnerFuzzyEnabled;
    CInnerFuzzyFinalMap m_fuzzyFinalMap;
    CFuzzySyllableMap m_fuzzyPreMap;
    CFuzzySyllableMap m_fuzzyProMap;
};

class CQuanpinSegmentor : public IPySegmentor
{
public:
    CQuanpinSegmentor ();

    virtual TSegmentVec& getSegments(bool req_aux_segs){
        if (req_aux_segs && m_pGetFuzzySegmentsOp &&
            m_pGetFuzzySegmentsOp->isEnabled()) {
            m_merged_segs.clear();
            std::merge(m_segs.begin(), m_segs.end(),
                       m_fuzzy_segs.begin(), m_fuzzy_segs.end(),
                       back_inserter(m_merged_segs));
            return m_merged_segs;
        }

        return m_segs;
    }

    virtual const wstring& getInputBuffer() { return m_inputBuf; }

    virtual const char* getSylSeps() { return "'"; }

    virtual unsigned push(unsigned ch);
    virtual unsigned pop();
    virtual unsigned insertAt(unsigned idx, unsigned ch);
    virtual unsigned deleteAt(unsigned idx, bool backward = true);
    virtual unsigned clear(unsigned from = 0);

    virtual unsigned updatedFrom() { return m_updatedFrom; }

    bool load(const char * pyTrieFileName);

    void setGetFuzzySyllablesOp(CGetFuzzySyllablesOp<CPinyinData> *op) {
        m_pGetFuzzySyllablesOp = op; }
    void setGetCorrectionPairOp(CGetCorrectionPairOp *op) {
        m_pGetCorrectionPairOp = op; }
    void setGetFuzzySegmentsOp(CGetFuzzySegmentsOp  *op) {
        m_pGetFuzzySegmentsOp = op; }

private:
    inline unsigned _push(unsigned ch);
    inline unsigned _clear(unsigned from);
    inline void _addFuzzySyllables(TSegment &seg);
    inline unsigned _updateWith(const std::string& new_pystr,
                                unsigned from = UINT_MAX);
    inline void _locateSegment(unsigned idx, unsigned &strIdx, unsigned &segIdx);

    CGetFuzzySyllablesOp<CPinyinData>  *m_pGetFuzzySyllablesOp;
    CGetCorrectionPairOp               *m_pGetCorrectionPairOp;
    CGetFuzzySegmentsOp                *m_pGetFuzzySegmentsOp;

    CDATrie<short>                      m_pytrie;
    std::string m_pystr;
    wstring m_inputBuf;
    TSegmentVec m_segs;
    TSegmentVec m_fuzzy_segs;
    TSegmentVec m_merged_segs;

    unsigned m_updatedFrom;
};

#endif /* SUNPY_PINYIN_SEG_H */

// -*- indent-tabs-mode: nil -*- vim:et:ts=4

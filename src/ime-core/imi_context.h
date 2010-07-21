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

#ifndef SUNPY_IMI_CONTEXT_H
#define SUNPY_IMI_CONTEXT_H

#include "portability.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#if defined(DEBUG) && defined (HAVE_ASSET_H)
#include <assert.h>
#endif

#include <climits>
#include <map>
#include <vector>

#include "pinyin/pinyin_seg.h"
#include "imi_data.h"
#include "ic_history.h"
#include "userdict.h"
#include "lattice_states.h"
#include "imi_funcobjs.h"

/**
* TSentenceScore is only used for whole sentence score,
* the score from language model still using double.
*/
typedef TLongExpFloat TSentenceScore;

class CLatticeFrame;
class CCandidate;
class CIMIContext;

typedef std::vector<CLatticeFrame>  CLattice;
typedef std::vector<CCandidate>     CCandidates;
typedef CCandidates::iterator       CCandidatesIter;

union TCandiRank {
public:
    bool
    operator< (const TCandiRank& b) const
        { return m_all < b.m_all; };

    TCandiRank() : m_all(0) { }

    TCandiRank(bool user, bool best, unsigned int len,
               bool fromLattice, TSentenceScore score);

    TCandiRank(bool user, bool best, unsigned int len,
              bool fromLattice, unsigned score);

protected:
    unsigned  int               m_all;
    #if !defined(WORDS_BIGENDIAN)
    struct TAnony {
        unsigned                m_cost   : 24;
        unsigned                m_lattice: 1;
        unsigned                m_best   : 1;
        unsigned                m_len    : 5;
        unsigned                m_user   : 1;
    } anony;
    #else
    struct TAnony {
        unsigned                m_user   : 1;
        unsigned                m_len    : 5;
        unsigned                m_best   : 1;
        unsigned                m_lattice: 1;
        unsigned                m_cost   : 24;
    } anony;
    #endif
}; // TCandiRank

/**
 * CCandidate represent basic information about a single candidate.
 * Its start bone and finishing bone. It's content string. and its
 * word id.
 */
class CCandidate {
    friend class CIMIContext;
public:
    unsigned            m_start;
    unsigned            m_end;
    const TWCHAR       *m_cwstr;

public:
    /** Give out the constructor for convinience */
    CCandidate(unsigned start=0, unsigned end=0, TLexiconState* pLxst = NULL, const TWCHAR* s = NULL, unsigned int wid=0)
        : m_start(start), m_end(end), m_pLexiconState(pLxst), m_cwstr(s), m_wordId(wid) {}

protected:
    unsigned int        m_wordId;
    TLexiconState*      m_pLexiconState;
}; // of CCandidate

class CLatticeFrame {
    friend class CIMIContext;
public:
    enum TYPE {
        UNUSED                  = 0x0000,      // unused frame
        TAIL                    = 0x0001,      // tail frame

        CATE_SYLLABLE           = 0x0100,
        SYLLABLE                = 0x0101,      // pinyin
        SYLLABLE_SEP            = 0x0102,      // pinyin
        INCOMPLETE_SYLLABLE     = 0x0104,      // incomplete syllable string

        CATE_OTHER              = 0x0200,
        ASCII                   = 0x0201,      // english string
        PUNC                    = 0x0202,      // punctuation
        SYMBOL                  = 0x0204,      // other symbol
        DIGITAL                 = 0x0208,      // not implemeted here
    }; // TYPE

    enum BESTWORD_TYPE {
        NO_BESTWORD             = 1 << 0,
        BESTWORD                = 1 << 1,
        USER_SELECTED           = 1 << 2,
        IGNORED                 = 1 << 3,
    }; // BESTWORD_TYPE

    unsigned    m_type;
    unsigned    m_bwType;
    wstring     m_wstr;

    CLatticeFrame () : m_type (UNUSED), m_bwType (NO_BESTWORD) {}

    bool isUnusedFrame () const 
        {return m_type == 0;}

    bool isSyllableFrame () const
        {return (m_type & CATE_SYLLABLE);}

    bool isSyllableSepFrame () const
        {return ((m_type & SYLLABLE_SEP) > CATE_SYLLABLE);}

    bool isTailFrame () const
        {return (m_type == TAIL);}

    void clear () 
    {
        m_type = UNUSED;
        m_bwType = NO_BESTWORD;
        m_lexiconStates.clear();
        m_latticeStates.clear();
        m_wstr.clear ();
    }

    void print (std::string prefix);

protected:
    CCandidate                  m_bestWord;
    CLexiconStates              m_lexiconStates;
    CLatticeStates              m_latticeStates;
}; // CLatticeFrame

class CIMIContext {
public:
     CIMIContext ();
    ~CIMIContext () {clear();}

    void clear ();

    void setCoreData (CIMIData *pCoreData);
    void setUserDict (CUserDict *pUserDict) {m_pUserDict = pUserDict;}

    void setHistoryMemory (CICHistory *phm) {m_pHistory = phm;}
    CICHistory * getHistoryMemory () {return m_pHistory;}
    
    void setHistoryPower (unsigned power)
        {m_historyPower = power <=10? power: 3;}

    int getHistoryPower ()
        {return m_historyPower;}

    void setFullSymbolForwarding (bool value=true) {m_bFullSymbolForwarding = value;}
    bool getFullSymbolForwarding () {return m_bFullSymbolForwarding;}
    void setGetFullSymbolOp (CGetFullSymbolOp *op) {m_pGetFullSymbolOp = op;}
    CGetFullSymbolOp& fullSymbolOp () const {return *m_pGetFullSymbolOp;}

    void setFullPunctForwarding (bool value=true) {m_bFullPunctForwarding = value;}
    bool getFullPunctForwarding () {return m_bFullPunctForwarding;}
    void setGetFullPunctOp (CGetFullPunctOp *op) {m_pGetFullPunctOp = op;}
    CGetFullPunctOp& fullPuncOp () const {return *m_pGetFullPunctOp;}

    void setNonCompleteSyllable(bool value=true) {m_bNonCompleteSyllable = value;}
    bool getNonCompleteSyllable() {return m_bNonCompleteSyllable;}
    
    void setCharsetLevel (unsigned l) {m_csLevel = l;}
    unsigned getCharsetLevel () {return m_csLevel;}
    
    void setDynamicCandidateOrder (bool value=true) {m_bDynaCandiOrder = value;}
    bool getDynaCandiOrder () {return m_bDynaCandiOrder;}

    CLattice& getLattice () {return m_lattice;}
    bool buildLattice (IPySegmentor *segmentor, bool doSearch=true);
    bool isEmpty () {return m_tailIdx <= 1;}
    unsigned getLastFrIdx () {return m_tailIdx-1;}

    // omit next punctuation if the very next symbol is an punctuation
    void omitNextPunct() { m_bOmitPunct = true; }

    bool searchFrom (unsigned from=1);
    std::vector<unsigned>& getBestPath () {return m_bestPath;}
    std::vector<unsigned>& getBestSegPath () 
        {
            // CIMIContext would fail to backTrace the bestPathes when there are no latticeStates
            // on frame e.g., 'yiden' in Quanpin mode, in this case, return the original segs
            if (m_bestSegPath.empty() && m_pPySegmentor) 
            {
                // only require the primary segments without the auxiliary ones
                IPySegmentor::TSegmentVec& segments = m_pPySegmentor->getSegments (false);
                IPySegmentor::TSegmentVec::const_iterator it  = segments.begin ();
                IPySegmentor::TSegmentVec::const_iterator ite = segments.end ();
                m_bestSegPath.push_back (0);
                for (; it != ite; ++it)
                    m_bestSegPath.push_back (it->m_start + it->m_len);
            }
                
            return m_bestSegPath;
        }

    unsigned getBestSentence (wstring& result, unsigned start=0, unsigned end=UINT_MAX);
    unsigned getBestSentence (std::vector<unsigned>& result, unsigned start=0, unsigned end=UINT_MAX);

    void getCandidates (unsigned frIdx, CCandidates& result);
    unsigned cancelSelection (unsigned frIdx, bool doSearch=true);
    void makeSelection (CCandidate &candi, bool doSearch=true);
    void deleteCandidate (CCandidate &candi);

    void memorize ();
    void removeFromHistoryCache (std::vector<unsigned>& wids);
    void printLattice ();

protected:
    void _clearFrom (unsigned from);

    inline bool _buildLattice (IPySegmentor::TSegmentVec &segments, unsigned rebuildFrom=1, bool doSearch=true);
    inline void _forwardSyllables (unsigned i, unsigned j, const IPySegmentor::TSegment& seg);
    inline void _forwardSingleSyllable (unsigned i, unsigned j, TSyllable syllable, const IPySegmentor::TSegment& seg);
    inline void _forwardSyllableSep (unsigned i, unsigned j);
    inline void _forwardString (unsigned i, unsigned j, const std::vector<unsigned>& strbuf);
    inline void _forwardPunctChar (unsigned i, unsigned j, unsigned ch);
    inline void _forwardOrdinaryChar (unsigned i, unsigned j, unsigned ch);
    inline void _forwardTail (unsigned i, unsigned j);

    inline void _transferBetween (unsigned start, unsigned end, TLexiconState* plxst, unsigned wid, double ic=1.0);
    inline void _backTraceBestPaths ();
    inline void _clearBestPaths ();

    inline const TWCHAR *_getWstr (unsigned wid);

    inline void _saveUserDict ();
    inline void _saveHistoryCache ();

protected:
    CLattice                    m_lattice;
    unsigned                    m_tailIdx;
    std::vector<unsigned>       m_bestPath;
    std::vector<unsigned>       m_bestSegPath;

    CThreadSlm                 *m_pModel;
    CPinyinTrie                *m_pPinyinTrie;
    CUserDict                  *m_pUserDict;
    CICHistory                 *m_pHistory;
    unsigned                    m_historyPower;
    
    unsigned                    m_csLevel;

    bool                        m_bFullSymbolForwarding;
    bool                        m_bOmitPunct;
    CGetFullSymbolOp           *m_pGetFullSymbolOp;

    bool                        m_bFullPunctForwarding;
    CGetFullPunctOp            *m_pGetFullPunctOp;

    IPySegmentor               *m_pPySegmentor;
    
    bool                        m_bNonCompleteSyllable;
    bool                        m_bDynaCandiOrder;

    unsigned                    m_candiStarts;
    unsigned                    m_candiEnds;

}; // CIMIContext

#endif

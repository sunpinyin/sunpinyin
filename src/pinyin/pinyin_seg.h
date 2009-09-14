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
#include "syllable.h"
#include "pinyin_data.h"
#include "datrie.h"
#include "utils.h"

#include <vector>

struct IPySegmentor
{
    enum ESegmentType 
        {SYLLABLE, SYLLABLE_SEP, INVALID, STRING};

    struct TSegment {
        TSegment (ESegmentType type=SYLLABLE) : m_type(type) {}
        TSegment (unsigned syllable, unsigned start, unsigned length, ESegmentType type=SYLLABLE) 
            : m_start(start), m_len(length), m_type(type)
            {m_syllables.push_back (syllable);}

        // if segment is a STRING type, m_syllables may contain the string buffer without the '\0'
        // for multiple syllables in one seg, the non-0th elements are treated as fuzzy syllables
        std::vector<unsigned>   m_syllables;
        unsigned                m_start        : 16;
        unsigned                m_len          : 8;
        ESegmentType            m_type         : 8;
    };

    // it requires the segments are sorted by its m_start field
    typedef std::vector<TSegment>  TSegmentVec;

    virtual ~IPySegmentor () {}
    virtual TSegmentVec& getSegments () = 0;
    virtual wstring& getInputBuffer () = 0;
    virtual const char* getSylSeps () = 0;

    virtual unsigned push (unsigned ch) = 0;
    virtual unsigned pop () = 0;
    virtual unsigned insertAt (unsigned idx, unsigned ch) = 0;
    virtual unsigned deleteAt (unsigned idx, bool backward=true) = 0;
    virtual unsigned clear (unsigned from=0) = 0;

    virtual unsigned updatedFrom () = 0;
    virtual void locateSegment (unsigned idx, unsigned &strIdx, unsigned &segIdx) = 0;
};

class CGetFuzzySyllablesOp : private CNonCopyable
{
public: 
    typedef std::multimap<const std::string, std::string> CFuzzyMap;

    CGetFuzzySyllablesOp () : m_bEnabled(false) {}
    void initFuzzyMap (const char **fuzzyPairs, unsigned num);

    void setEnable (bool value=true) {m_bEnabled = value;}
    bool isEnabled () {return m_bEnabled;}

    CSyllables operator () (TSyllable s);

private:
    CFuzzyMap   m_fuzzyMap;
    bool        m_bEnabled;
};

class CGetCorrectionPairOp : private CNonCopyable
{
public:
    typedef std::pair<std::string, std::string> CCorrectionPair;
    typedef std::vector<CCorrectionPair> CCorrectionPairVec;

    CGetCorrectionPairOp () : m_bEnabled(false) {m_correctionPairs.reserve(8);}

    void setEnable (bool value=true) {m_bEnabled = value;}
    bool isEnabled () {return m_bEnabled;}

    void setCorrectionPairs (const char **pairs, unsigned num) 
    {
        m_correctionPairs.clear ();
        for (unsigned i=0; i<num; ++i) {
            const char * k = pairs [i*2];
            const char * v = pairs [i*2+1];

            m_correctionPairs.push_back (std::pair<std::string, std::string> (k, v));
        }
    }

    const char * operator () (std::string& pystr, unsigned& matched_len);
    
private:
    CCorrectionPairVec  m_correctionPairs;
    bool                m_bEnabled;
};

class CQuanpinSegmentor : public IPySegmentor
{
public:
    CQuanpinSegmentor ();

    virtual TSegmentVec& getSegments () {return m_segs;}
    virtual wstring& getInputBuffer () {return m_inputBuf;}
    virtual const char* getSylSeps () {return "'";}

    virtual unsigned push (unsigned ch);
    virtual unsigned pop ();
    virtual unsigned insertAt (unsigned idx, unsigned ch);
    virtual unsigned deleteAt (unsigned idx, bool backward=true);
    virtual unsigned clear (unsigned from=0);

    virtual unsigned updatedFrom () {return m_updatedFrom;}
    virtual void locateSegment (unsigned idx, unsigned &strIdx, unsigned &segIdx);

    bool load(const char * pyTrieFileName);
    
    void setGetFuzzySyllablesOp (CGetFuzzySyllablesOp *op) {m_pGetFuzzySyllablesOp = op;}
    void setGetCorrectionPairOp (CGetCorrectionPairOp *op) {m_pGetCorrectionPairOp = op;}

private:
    inline unsigned _push  (unsigned ch);
    inline unsigned _clear (unsigned from);
    inline void _addFuzzySyllables (TSegment &seg);

    CGetFuzzySyllablesOp       *m_pGetFuzzySyllablesOp;
    CGetCorrectionPairOp       *m_pGetCorrectionPairOp;

    CDATrie<short>              m_pytrie;
    std::string                 m_pystr;
    wstring                     m_inputBuf;
    TSegmentVec                 m_segs;

    unsigned                    m_updatedFrom;
};

#endif /* SUNPY_PINYIN_SEG_H */

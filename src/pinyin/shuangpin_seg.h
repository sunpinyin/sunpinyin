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

#ifndef SUNPY_SHUANGPIN_SEG_H
#define SUNPY_SHUANGPIN_SEG_H

#include "portability.h"
#include "shuangpin_data.h"
#include "segmentor.h"
#include "pinyin_data.h"
#include <vector>

class CShuangpinSegmentor : public IPySegmentor
{
public:
    CShuangpinSegmentor (EShuangpinType shpType);

    virtual TSegmentVec& getSegments(bool) { return m_segs; }
    virtual const wstring& getInputBuffer() { return m_inputBuf; }
    virtual const char* getSylSeps() { return "'"; }

    virtual unsigned push(unsigned ch);
    virtual unsigned pop();
    virtual unsigned insertAt(unsigned idx, unsigned ch);
    virtual unsigned deleteAt(unsigned idx, bool backward = true);
    virtual unsigned clear(unsigned from = 0);

    virtual unsigned updatedFrom() { return m_updatedFrom; }

    void setGetFuzzySyllablesOp(CGetFuzzySyllablesOp<CPinyinData> *op) {
        m_pGetFuzzySyllablesOp = op; }

private:
    unsigned _push(unsigned ch);
    unsigned _clear(unsigned from);
    int _getNumberOfNonAlpha() const;
    int _encode(const char* buf, char ch, bool isComplete);
    void _locateSegment(unsigned idx, unsigned &strIdx, unsigned &segIdx);

    inline void _addFuzzySyllables(TSegment &seg);

    static CShuangpinData s_shpData;
    std::string m_pystr;
    wstring m_inputBuf;
    TSegmentVec m_segs;
    unsigned m_updatedFrom;
    int m_nAlpha;                                     /* number of non-py chars in m_pystr*/
    bool m_hasInvalid;                                /* true if there is invalid py in m_pystr */
    unsigned m_nLastValidPos;

    CGetFuzzySyllablesOp<CPinyinData>  *m_pGetFuzzySyllablesOp;
};

#endif /* SUNPY_SHUANGPIN_SEG_H */

// -*- indent-tabs-mode: nil -*- vim:et:ts=4

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

#include <climits>
#include "shuangpin_seg.h"

CShuangpinData CShuangpinSegmentor::s_shpData;

CShuangpinSegmentor::CShuangpinSegmentor (EShuangpinType shpType) 
    : m_updatedFrom(0), m_nAlpha(0), m_bPreInvalid(false), m_nLastValidPos(0)
{
    m_segs.reserve (32);
    m_pystr.reserve (32);
    if (s_shpData.getShuangpinType() != shpType)
        s_shpData = CShuangpinData(shpType);
}

unsigned CShuangpinSegmentor::push (unsigned ch)
{
    m_inputBuf.push_back (ch);

    return m_updatedFrom = _segmentor (ch);
}

unsigned CShuangpinSegmentor::pop ()
{
    if (m_pystr.empty())
        return m_updatedFrom = 0;

    unsigned size = m_inputBuf.size ();
    
    if (!islower(m_pystr[size-1])) {
        m_nAlpha -= 1;
    }

    m_inputBuf.resize (size - 1);
    m_pystr.resize (size - 1);

    unsigned l = m_segs.back().m_len;
    m_segs.pop_back ();

    if (size==1 || m_segs.back().m_type!=IPySegmentor::INVALID) {
        m_bPreInvalid = false;
    }
    if (l == 1) 
        return m_updatedFrom = size - 1;

    std::string new_pystr = m_pystr.substr (size-l);
    m_pystr.resize (size-l);

    m_updatedFrom = UINT_MAX;
    std::string::const_iterator it = new_pystr.begin();
    for (; it!= new_pystr.end(); ++it) {
        unsigned tmp = _segmentor ((*it) & 0x7f);
        if (tmp < m_updatedFrom) m_updatedFrom = tmp;
    }

    return m_updatedFrom;
}

unsigned CShuangpinSegmentor::insertAt (unsigned idx, unsigned ch)
{
    unsigned i, j;
    locateSegment (idx, i, j);

    m_inputBuf.insert (idx, 1, ch);
    m_pystr.insert (idx, 1, ch);

    std::string new_pystr = m_pystr.substr (i);
    m_pystr.resize (i);
    m_segs.erase (m_segs.begin()+j, m_segs.end());

    if (m_nLastValidPos == idx) {
        m_bPreInvalid = false;
    } else if (m_nLastValidPos + 1 == idx) {
        m_bPreInvalid = false;
        int nSize = m_pystr.size();
        if (islower(m_pystr[nSize-1])) {
            m_nLastValidPos = idx - 1;
            new_pystr.insert((size_t)0, 1, m_pystr[nSize-1]);
            m_pystr.erase(nSize-1, 1);
            m_segs.erase (m_segs.begin()+j-1);
        } 
    
    } else if (m_nLastValidPos + 1 > idx) {
        m_bPreInvalid = false;
        m_nLastValidPos = idx;
    }
    unsigned num = 0;
    unsigned nSize = m_pystr.size();
    for (int n = 0; n<nSize; n++) {
        if (!islower(m_pystr.at(n))) {
            num += 1;
        }
    }
    m_nAlpha = num;

    m_updatedFrom = UINT_MAX;
    std::string::const_iterator it = new_pystr.begin();
    for (; it!= new_pystr.end(); ++it) {
        unsigned tmp = _segmentor ((*it) & 0x7f);
        if (tmp < m_updatedFrom) m_updatedFrom = tmp;
    }

    return m_updatedFrom;
}

unsigned CShuangpinSegmentor::deleteAt (unsigned idx, bool backward)
{
    unsigned i, j;
    if (!backward) idx += 1;
    locateSegment (idx, i, j);

    m_inputBuf.erase (idx, 1);
    m_pystr.erase (idx, 1);

    std::string new_pystr = m_pystr.substr (i);
    m_pystr.resize (i);
    TSegmentVec tmp_segs;
    tmp_segs.assign (m_segs.begin()+j+1, m_segs.end());
    m_segs.erase (m_segs.begin()+j, m_segs.end());

    if (idx==0) {
        m_bPreInvalid = false;
    }
    if (m_nLastValidPos + 1 < idx) {
       //del invalid ch, and do not effect current status. 
        m_pystr.insert(idx, new_pystr);
        m_segs.insert (m_segs.end(), tmp_segs.begin(), tmp_segs.end());
        return m_inputBuf.size() -1;
    } else {
        m_bPreInvalid = false;
        
        unsigned nSize = m_pystr.size();
        unsigned num = 0;
        for (int n = 0; n<nSize; n++) {
            if (!islower(m_pystr.at(n))) {
                num += 1;
            }
        }
        m_nAlpha = num;
    }

    m_updatedFrom = UINT_MAX;
    std::string::const_iterator it = new_pystr.begin();
    for (; it!= new_pystr.end(); ++it) {
        unsigned tmp = _segmentor ((*it) & 0x7f);
        if (tmp < m_updatedFrom) m_updatedFrom = tmp;
    }

    return m_updatedFrom;
}

unsigned CShuangpinSegmentor::clear (unsigned from)
{
    m_inputBuf.resize (from);
    return _clear (from);
}

unsigned CShuangpinSegmentor::_clear (unsigned from)
{
    unsigned i, j;
    locateSegment (from, i, j);


    std::string new_pystr = m_pystr.substr (i, from-i);
    m_pystr.resize (i);
    m_segs.erase (m_segs.begin()+j, m_segs.end());

    m_updatedFrom = from;
    std::string::const_iterator it = new_pystr.begin();
    for (; it!= new_pystr.end(); ++it) {
        unsigned tmp = _segmentor ((*it) & 0x7f);
        if (tmp < m_updatedFrom) m_updatedFrom = tmp;
    }

    return m_updatedFrom;
}

void CShuangpinSegmentor::locateSegment (unsigned idx, unsigned &strIdx, unsigned &segIdx)
{
    strIdx = segIdx = 0;

    TSegmentVec::iterator it  = m_segs.begin();
    TSegmentVec::iterator ite = m_segs.end();

    for (; it != ite; ++it) {
        if (strIdx + (*it).m_len > idx)
           break; 

        strIdx += (*it).m_len;
        segIdx += 1;
    }
}

static char wchar_to_char (unsigned ch)
{
    return ch;
}

unsigned CShuangpinSegmentor::_segmentor (unsigned ch)
{
    CMappedYin  syls;
    unsigned    ret = 0;
    char        buf[4];
    int         len = 0;

    m_pystr.push_back (ch); 
    len = m_pystr.size();
    if (m_bPreInvalid) {
        ret = m_pystr.size () - 1;
        m_segs.push_back (TSegment (ch, ret, 1, IPySegmentor::INVALID));
        m_bPreInvalid = true;
        return ret;
    }

    IPySegmentor::ESegmentType seg_type;
    EShuangpinType    shpType = s_shpData.getShuangpinType();
    bool       bInputCh = false;
    if ((shpType == MS2003 || shpType ==ZIGUANG) && ch==';') {
        bInputCh = true;
    }
    bool  bCompleted = !((len - m_nAlpha)%2) && 
                 ( islower(m_pystr[m_pystr.length()-1]) || bInputCh ) ;

    if (!islower(ch) && !bInputCh) { 
        ret = m_pystr.size() - 1;
        (ch == '\'') ? seg_type = IPySegmentor::SYLLABLE_SEP: seg_type = IPySegmentor::STRING;
        m_segs.push_back (TSegment (ch, ret, 1, seg_type));
        m_nAlpha += 1;
        m_nLastValidPos += 1;
        return  ret;
    } else{

        if (bCompleted) {
            memset(buf, '\0', sizeof(buf));
            sprintf(buf, "%c%c", m_pystr[len-2], ch);
        } else {
            sprintf(buf, "%c", ch);
        }

        syls.reserve(8);
        s_shpData.getMapString(buf, syls);
        if (syls.empty()) {
            if (bCompleted) {
                sprintf(buf, "%c", ch);
                syls.clear();
                s_shpData.getMapString(buf, syls);
                if (!syls.empty()) {
                    CMappedYin::iterator iter = syls.begin();
                    CMappedYin::iterator iter_end = syls.end();
                    TSegment s;

                    s.m_start =  m_pystr.size() - 1;
                    s.m_len = 1;
                    for (; iter!=iter_end; iter++) {
                        s.m_syllables.push_back(s_shpData.encodeSyllable(iter->c_str()));
                    }
                    m_segs.push_back (s);
                    m_nLastValidPos += 1;
                    return m_pystr.size() - 1;
                }
            }  
            seg_type = IPySegmentor::INVALID;
            m_bPreInvalid = true;
            ret = m_pystr.size() - 1;
            m_segs.push_back (TSegment (ch, ret, 1, seg_type));
            
            return ret;
        } else {
            CMappedYin::iterator iter = syls.begin();
            CMappedYin::iterator iter_end = syls.end();

            if (!bCompleted) {
                TSegment s;
                s.m_start =  m_pystr.size() - 1;
                s.m_len = 1;
                for (; iter!=iter_end; iter++) {
                    s.m_syllables.push_back(s_shpData.encodeSyllable(iter->c_str()));
                }
                m_segs.push_back (s);
                m_nLastValidPos += 1;
                return m_pystr.size() - 1;
            } else {
                TSegment &s = m_segs.back();
                s.m_len = 2;
                s.m_start =  m_pystr.size() - 2;
                s.m_syllables.clear();
                for (; iter!=iter_end; iter++) {
                    s.m_syllables.push_back(s_shpData.encodeSyllable(iter->c_str()));
                }
                m_nLastValidPos += 1;
                return m_pystr.size() - 2;
            }
        }
    }
}

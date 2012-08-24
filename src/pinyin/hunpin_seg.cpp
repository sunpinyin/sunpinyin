/*
 *  hunpin_seg.cpp
 *  FIT
 *
 *  Created by cererd.long on 10-10-8.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "hunpin_seg.h"
#include <cassert>
#include <functional>
#include <algorithm>
#include "pinyin_seg.h"
#include "quanpin_trie.h"


CShuangpinData CHunpinSegmentor::s_shpData;

CHunpinSegmentor::CHunpinSegmentor (EShuangpinType shpType)
    : m_pGetFuzzySyllablesOp(NULL),
      m_pytrie(base, check, value, sizeof(base) / sizeof(*base)),
      m_updatedFrom(0)
{
    m_segs.reserve(32);
    s_shpData.setShuangpinType(shpType);
}


int
CHunpinSegmentor::_encode(const char* buf, int ret)
{
    CMappedYin syls;
    syls.reserve(8);
    s_shpData.getMapString(buf, syls);
    if (syls.empty())
        return -1;


    CMappedYin::const_iterator iter = syls.begin();
    CMappedYin::const_iterator iter_end = syls.end();

    m_segs.push_back(TSegment(0, 0, 1, IPySegmentor::SYLLABLE));
    TSegment &s = m_segs.back();

    s.m_len = 2;
    s.m_start = ret;
    s.m_syllables.clear();
    s.m_type = IPySegmentor::SYLLABLE;
    for (; iter != iter_end; iter++) {
        s.m_syllables.push_back(s_shpData.encodeSyllable(iter->c_str()));
    }

    return s.m_start;
}

int
CHunpinSegmentor::_encode(const char* buf)
{
    CMappedYin syls;
    syls.reserve(8);
    s_shpData.getMapString(buf, syls);
    if (syls.empty())
        return -1;

    CMappedYin::const_iterator iter = syls.begin();
    CMappedYin::const_iterator iter_end = syls.end();

    TSegment &s = m_segs.back();
    s.m_len = 2;
    s.m_start = m_pystr.size() - s.m_len;
    s.m_syllables.clear();
    s.m_type = IPySegmentor::SYLLABLE;
    for (; iter != iter_end; iter++) {
        s.m_syllables.push_back(s_shpData.encodeSyllable(iter->c_str()));
    }

    return s.m_start;
}

bool
CHunpinSegmentor::load(const char * pyTrieFileName)
{
    return m_pytrie.load(pyTrieFileName);
}

#if 0
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
CHunpinSegmentor::push(unsigned ch)
{
    m_inputBuf.push_back(ch);

    m_updatedFrom = _push(ch);


    return m_updatedFrom;
}

unsigned
CHunpinSegmentor::pop()
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
CHunpinSegmentor::insertAt(unsigned idx, unsigned ch)
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
CHunpinSegmentor::deleteAt(unsigned idx, bool backward)
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
CHunpinSegmentor::clear(unsigned from)
{
    m_inputBuf.resize(from);
    return _clear(from);
}

unsigned
CHunpinSegmentor::_clear(unsigned from)
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
CHunpinSegmentor::_locateSegment(unsigned idx,
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

// TOTEST
unsigned
CHunpinSegmentor::_push(unsigned ch)
{
    m_pystr.push_back(ch);

    TSegmentVec::iterator ite = m_segs.size() > 0 ? m_segs.end() -
                                1 : m_segs.begin() - 1;
    const unsigned maxStringCount = 6;
    unsigned syllableCount = 0;
    unsigned stringCount = 0;
    for (; ite != m_segs.begin() - 1; ite--) {
        stringCount += (*ite).m_len;
        syllableCount++;
        if (stringCount > maxStringCount) {
            syllableCount--;
            break;
        }
    }

    unsigned strlen = m_pystr.size();
    unsigned ret;

    for (int index = syllableCount; index >= 0; index--) {
        TSegmentVec::iterator it = m_segs.end() - index;
        unsigned tmpl;
        unsigned v;
        if (index != 0) {
            if ((strlen - (*it).m_start) == 2) {
                char buf[4];
                sprintf(buf, "%c%c", m_pystr[(*it).m_start],
                        m_pystr[(*it).m_start + 1]);
                int startFrom = _encode(buf);
                if (startFrom >= 0) break;
            }

            v = m_pytrie.match_longest(m_pystr.rbegin(),
                                       m_pystr.rbegin() + strlen -
                                       (*it).m_start, tmpl);

            if (tmpl == (strlen - (*it).m_start)) {
                TSegmentVec new_segs(1, TSegment(v, (*it).m_start, tmpl));
                m_segs.erase(m_segs.end() - index, m_segs.end());
                std::copy(new_segs.rbegin(), new_segs.rend(),
                          back_inserter(m_segs));

                break;
            }
        } else {
            v = m_pytrie.match_longest(m_pystr.rbegin(),
                                       m_pystr.rbegin() + 1, tmpl);
            if (tmpl == 0) {
                IPySegmentor::ESegmentType seg_type;
                if (ch == '\'' && m_inputBuf.size() > 1) {
                    seg_type = IPySegmentor::SYLLABLE_SEP;
                } else if (islower(ch)) {
                    seg_type = IPySegmentor::INVALID;
                } else {
                    seg_type = IPySegmentor::STRING;
                }
                ret = m_pystr.size() - 1;
                m_segs.push_back(TSegment(ch, ret, 1, seg_type));
            } else {
                ret = m_pystr.size() - 1;
                m_segs.push_back(TSegment(v, ret, 1));
            }
        }
    }

    TSegment &last_seg = m_segs.back();
    if (m_pGetFuzzySyllablesOp && m_pGetFuzzySyllablesOp->isEnabled())
        if (m_segs.back().m_type == SYLLABLE)
            _addFuzzySyllables(last_seg);

    return last_seg.m_start;
}

/*
   //慢算法
   unsigned CHunpinSegmentor::_push (unsigned ch)
   {
        printf("using hunpin_seg");
        //translation for positive match arithmetic
        //m_segs.erase (m_segs.begin(), m_segs.end());
        m_segs.clear();
        m_pystr.push_back (ch);
        unsigned strlen = m_pystr.size();
        unsigned maxlen = 6;
        unsigned _start = 0;

        while (_start < strlen) {
                for (unsigned _tn = (maxlen+_start) <= strlen ? maxlen : (strlen - _start) ; _tn > 0 ; _tn--) {


                        if (_tn == 2) {
                                char buf[4];
                                //printf("\nout,shuang pin output,char1:%c,char2:%c\n",m_pystr[_start],m_pystr[_start+1]);

                                sprintf(buf, "%c%c", m_pystr[_start], m_pystr[_start+1]);
                                int startFrom = _encode(buf,_start);
                                if(startFrom >= 0)  {
                                        //printf("\nin,shuang pin output,char1:%c,char2:%c\n",m_pystr[_start],m_pystr[_start+1]);
                                        _start += 2;
                                        break;
                                }
                        }


                        unsigned tmpl;
                        int v  = m_pytrie.match_longest ((m_pystr.rbegin() + strlen - _start - _tn), (m_pystr.rbegin() + strlen - _start), tmpl);

                        //printf("\n input match len is %d,size is %d,_start is %d ,end is %d ,start char is %c,end char is %c\n",tmpl,strlen,_start,_start + _tn,m_pystr[_start],m_pystr[_start+_tn-1]);

                        if(tmpl == _tn) {
                                m_segs.push_back (TSegment (v, _start, _tn));
                                _start += _tn;
                                break;
                        }
                        else if(_tn == 1 && tmpl == 0) {
                                IPySegmentor::ESegmentType seg_type;
                                if (m_pystr[_start] == '\'' && m_inputBuf.size() > 1)
                                        seg_type = IPySegmentor::SYLLABLE_SEP;
                                else if (islower (m_pystr[_start]))
                                        seg_type = IPySegmentor::INVALID;
                                else
                                        seg_type = IPySegmentor::STRING;

                                m_segs.push_back (TSegment (m_pystr[_start], _start, 1, seg_type));
                                _start += 1;
                                break;
                        }
                }
        }

        TSegment &last_seg = m_segs.back();
        if (m_pGetFuzzySyllablesOp && m_pGetFuzzySyllablesOp->isEnabled())
        if ( m_segs.back().m_type == SYLLABLE)
            _addFuzzySyllables (last_seg);

        return last_seg.m_start;
   }
 */

void
CHunpinSegmentor::_addFuzzySyllables(TSegment& seg)
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
CHunpinSegmentor::_updateWith(const std::string& new_pystr, unsigned from)
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

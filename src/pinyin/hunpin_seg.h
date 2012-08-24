// -*- mode: c++ -*-
/*
 *  hunpin_seg.h
 *  FIT
 *
 *  Created by cererd.long on 10-10-8.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */


#ifndef SUNPY_HUNPIN_SEG_H
#define SUNPY_HUNPIN_SEG_H




#include "datrie.h"
#include "portability.h"
#include "shuangpin_data.h"
#include "segmentor.h"
#include "pinyin_data.h"
#include <vector>
#include <limits.h>

class CHunpinSegmentor : public IPySegmentor
{
public:
    CHunpinSegmentor (EShuangpinType shpType);

    virtual TSegmentVec& getSegments(bool) { return m_segs; }


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




private:
    inline unsigned _push(unsigned ch);
    inline unsigned _clear(unsigned from);
    inline void _addFuzzySyllables(TSegment &seg);
    inline unsigned _updateWith(const std::string& new_pystr,
                                unsigned from = UINT_MAX);
    inline void _locateSegment(unsigned idx, unsigned &strIdx, unsigned &segIdx);
    int _encode(const char* buf, int ret);
    int _encode(const char* buf);


    CGetFuzzySyllablesOp<CPinyinData>  *m_pGetFuzzySyllablesOp;


    CDATrie<short>                      m_pytrie;
    std::string m_pystr;
    wstring m_inputBuf;
    TSegmentVec m_segs;
    TSegmentVec m_fuzzy_segs;
    TSegmentVec m_merged_segs;

    unsigned m_updatedFrom;
    static CShuangpinData s_shpData;
};

#endif /* SUNPY_PINYIN_SEG_H */

// -*- indent-tabs-mode: nil -*- vim:et:ts=4

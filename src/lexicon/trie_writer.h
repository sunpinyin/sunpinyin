// -*- mode: c++ -*-
#ifndef __SUNPINYIN_PYTRIE_WRITER_H__
#define __SUNPINYIN_PYTRIE_WRITER_H__

#include "writer.h"
#include "pytrie.h"

template <>
class OtherEndian<CPinyinTrie::TNode>
{
    struct TNode_BE {
        unsigned m_other      : 5;
        unsigned m_bFullSyllableTransfer : 1;
        unsigned m_csLevel    : 2;
        unsigned m_nTransfer  : 12;
        unsigned m_nWordId    : 12;
    };

    struct TNode_LE {
        unsigned m_nWordId    : 12;
        unsigned m_nTransfer  : 12;
        unsigned m_csLevel    : 2;
        unsigned m_bFullSyllableTransfer : 1;
        unsigned m_other      : 5;
    };
public:
    DEFINE_OTHER_TYPE(TNode);

    static TargetType create(const CPinyinTrie::TNode& from){
        TargetType to;
        to.m_nTransfer = from.m_nTransfer;
        to.m_nWordId = from.m_nWordId;
        to.m_bFullSyllableTransfer = from.m_bFullSyllableTransfer;
        to.m_csLevel = from.m_csLevel;
        // we don't care about m_other though
        to.m_other = from.m_other;
        return to;
    }
};

template<>
class OtherEndian<TSyllable>
{
    struct TSyllable_BE {
        unsigned other    : 12;
        unsigned initial  : 8;
        unsigned final    : 8;
        unsigned tone     : 4;
    };

    struct TSyllable_LE {
        unsigned tone     : 4;
        unsigned final    : 8;
        unsigned initial  : 8;
        unsigned other    : 12;
    };

public:
    DEFINE_OTHER_TYPE(TSyllable);

    static TargetType create(const TSyllable& from){
        TargetType to;
        to.other = from.other;
        to.initial = from.initial;
        to.final = from.final;
        to.tone = from.tone;
        return to;
    }
};

template <>
class OtherEndian<CPinyinTrie::TWordIdInfo>
{
    struct TWordIdInfo_BE {
        unsigned m_bSeen    : 1;
        unsigned m_cost     : 5;
        unsigned m_csLevel  : 2;
        unsigned m_id       : WORD_ID_WIDTH;
    };

    struct TWordIdInfo_LE {
        unsigned m_id       : WORD_ID_WIDTH;
        unsigned m_csLevel  : 2;
        unsigned m_cost     : 5;
        unsigned m_bSeen    : 1;
    };

public:
    DEFINE_OTHER_TYPE(TWordIdInfo);

    static TargetType create(const CPinyinTrie::TWordIdInfo& from){
        TargetType to;
        to.m_id = from.m_id;
        to.m_csLevel = from.m_csLevel;
        to.m_bSeen = from.m_bSeen;
        to.m_cost = from.m_cost;
        return to;
    }
};

template <>
bool revert_write<CPinyinTrie::TTransUnit> (const CPinyinTrie::TTransUnit& t,
                                            FILE *fp);

#endif //__SUNPINYIN_PYTRIE_WRITER_H__

// -*- indent-tabs-mode: nil -*- vim:et:ts=4

#ifndef __SUNPINYIN_PYTRIE_H__
#define __SUNPINYIN_PYTRIE_H__

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "../portability.h"
#include "pinyin/syllable.h"
#include <map>

#define WORD_ID_WIDTH       24

class CPinyinTrie {
public:
    friend class CPinyinTrieMaker;

    struct TTransUnit {
        TSyllable       m_Syllable;
        unsigned        m_Offset;
    };

    struct TWordIdInfo {
    #ifdef WORDS_BIGENDIAN
        unsigned        m_bSeen    : 1;
        unsigned        m_cost     : 5;
        unsigned        m_csLevel  : 2;
        unsigned        m_id       : WORD_ID_WIDTH;
    #else
        unsigned        m_id       : WORD_ID_WIDTH;
        unsigned        m_csLevel  : 2;
        unsigned        m_cost     : 5;
        unsigned        m_bSeen    : 1;
    #endif

        TWordIdInfo() { memset(this, 0, sizeof(TWordIdInfo)); }

        TWordIdInfo(unsigned id, unsigned len=0, unsigned seen=0, unsigned cost = 0, unsigned cslvl = 0)
            : m_id(id), m_csLevel(cslvl), m_cost(cost), m_bSeen(seen) { }

        operator unsigned int() const { return m_id; }
    };

    struct TNode {
    #ifdef WORDS_BIGENDIAN
        unsigned        m_other      : 5;
        unsigned        m_bFullSyllableTransfer: 1;
        unsigned        m_csLevel    : 2;
        unsigned        m_nTransfer  : 12;
        unsigned        m_nWordId    : 12;
    #else
        unsigned        m_nWordId    : 12;
        unsigned        m_nTransfer  : 12;
        unsigned        m_csLevel    : 2;
        unsigned        m_bFullSyllableTransfer: 1;
        unsigned        m_other      : 5;
    #endif

        static unsigned int
        size_for(unsigned int nTransfer, unsigned int nWordId)
            { return sizeof(TNode) + sizeof(TTransUnit)*nTransfer + 
                     sizeof(TWordIdInfo)*nWordId; }

        TNode() 
            { *((unsigned*)this) = 0; }

        bool
        hasPinyinChild(void) const
            { return (m_nTransfer > 1);}

        const TTransUnit*
        getTrans() const
            { return (TTransUnit*)(this+1); }

        const TWordIdInfo*
        getWordIdPtr() const
            { return (TWordIdInfo*)(((char*)(this+1))+sizeof(TTransUnit)*m_nTransfer); }

        unsigned int
        transfer(unsigned s) const 
            {
                unsigned int b = 0, e = m_nTransfer;
                const TTransUnit* ptrans = getTrans();
                while (b < e) {
                    int m = b + (e-b)/2;
                    if (ptrans[m].m_Syllable == s)
                        return ptrans[m].m_Offset;
                    if (ptrans[m].m_Syllable < s)
                        b = m + 1;
                    else
                        e = m;
                }
                return 0;
            }
    };

public:
    CPinyinTrie() : m_Size(0), m_mem(NULL), m_words(NULL) { }

    ~CPinyinTrie() 
        { free(); }

    bool
    load(const char* fileName);

    void
    free(void);

    bool
    isValid(const TNode* pnode, bool allowNonComplete, unsigned csLevel=0);

    unsigned int
    getRootOffset() const 
        { return 3 * sizeof(unsigned int); }

    const TNode*
    getRootNode() const 
        { return (TNode*)(m_mem+getRootOffset()); }

    const TNode*
    nodeFromOffset(unsigned int offset) const
        { return (offset < getRootOffset())?NULL:((TNode*)(m_mem+offset)); }

    unsigned int
    getWordCount(void) const 
        { return *(unsigned int*)m_mem; }

    unsigned int
    getNodeCount(void) const 
        { return *(unsigned int*)(m_mem+sizeof(unsigned int)); }

    unsigned int
    getStringOffset(void) const 
        { return *(unsigned int*)(m_mem+2*sizeof(unsigned int)); }

    inline const TNode*
    transfer(const TNode* pnode, unsigned s) const
        { return nodeFromOffset(pnode->transfer(s)); }

    inline const TNode*
    transfer(unsigned s) const
        { return transfer(getRootNode(), s); }

    unsigned int
    getSymbolId(const TWCHAR* wstr);

    unsigned int
    getSymbolId(const wstring & wstr);

    const TWCHAR*
    operator[](unsigned int idx) const 
        { return m_words[idx]; }

    int
    lengthAt(unsigned int idx) const;

    void
    print(FILE *fp) const;

protected:
    unsigned int           m_Size;
    char                  *m_mem;
    TWCHAR               **m_words;

    std::map<wstring, unsigned>  m_SymbolMap;

    void
    print(const TNode* pRoot, std::string& prefix, FILE *fp) const;
};

#endif /* __SUNPINYIN_PYTRIE_H__*/

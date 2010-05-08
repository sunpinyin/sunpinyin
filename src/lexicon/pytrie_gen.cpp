#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_ASSERT_H
#include <assert.h>
#endif

#ifdef HAVE_ICONV_H
#include <iconv.h>
#endif

#include <algorithm>

#include "pytrie_gen.h"
#include "pinyin_data.h"
#include "trie_writer.h"

static const char*
skipSpace(const char* p)
{
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        ++p;
    return p;
}

static const char*
skipNonSpace(const char* p)
{
    while (*p != '\0' && *p != ' ' && *p != '\t' && *p != '\n' && *p != '\r')
        ++p;
    return p;
}

static void
insertWordId(CPinyinTrieMaker::CWordSet& idset, CPinyinTrieMaker::TWordId id)
{
    CPinyinTrieMaker::CWordSet::const_iterator it = idset.find(id);
    if (it == idset.end())
        idset.insert(id);
    else {
        const CPinyinTrieMaker::TWordId& a = *it;
        if ((a.anony.m_bHide && !id.anony.m_bHide) || (a.anony.m_bHide == id.anony.m_bHide && a.anony.m_cost > id.anony.m_cost)) {
            idset.erase(it);
            idset.insert(id);
        }
    }
}

struct TSyllableInfo {
    std::string   m_py;
    int           m_cost;

    TSyllableInfo(const char* py=NULL, int cost=0) : m_py(py), m_cost(cost) {}
    bool operator< (const TSyllableInfo& b) const { return m_py < b.m_py; }
};

#ifdef HAVE_ICONV_H
bool isCorrectConverted(const char* utf8, iconv_t ic, iconv_t ric)
{
    static char gbstr[256];
    static char utstr[256];

    TIConvSrcPtr src = (TIConvSrcPtr)utf8;
    size_t srclen = strlen((char*)src)+1;
    char* dst = (char *)gbstr;
    size_t dstlen = 256;
    size_t res = iconv(ic, &src, &srclen, &dst, &dstlen);

    if (res != size_t(-1) && srclen == 0) {
        // do revert convertion and compare them
        src = (TIConvSrcPtr)gbstr;
        srclen = strlen((char*)src)+1;
        dst = (char *)utstr;
        dstlen = 256;
        res = iconv(ric, &src, &srclen, &dst, &dstlen);
        if (res != size_t(-1) && srclen == 0)
            return (strcmp(utf8, utstr) == 0);
    }
    return false;
}

//return: bit 0x1: contains some gbk out of gb2312, bit 0x2: contains some gb18030 outof gbk
unsigned getPureGBEncoding(const char* utf8str)
{
    static iconv_t ic_gb = iconv_open("GB2312", "UTF-8");
    static iconv_t ic_gbk = iconv_open("GBK", "UTF-8");
    static iconv_t ric_gb = iconv_open("UTF-8", "GB2312");
    static iconv_t ric_gbk = iconv_open("UTF-8", "GBK");

    unsigned ret = 0;

    if (!isCorrectConverted(utf8str, ic_gb, ric_gb)) {
        ret = 1; // at least it is contains some GBK char
        if (!isCorrectConverted(utf8str, ic_gbk, ric_gbk))
            ret = 3; //contains some GB18030-only char

        #ifdef DEBUG
            fprintf(stderr, "==> GB category of (%s) is (0x%x)\n ", utf8str, ret);
            fflush(stderr);
        #endif
    }
    return ret;
}
#else // !HAVE_ICONV_H
unsigned getPureGBEncoding(const char* utf8str)
{
    // FIXME
    return 0x3;
}
#endif // HAVE_ICONV_H

bool
parseLine(char* buf, char* word_buf, unsigned& id, std::set<TSyllableInfo>& pyset)
{
    pyset.clear();

    /* ignore the empty lines and comment lines */
    if (*buf == '\n' || *buf == '#')
        return 0;

    char* p = (char*)skipSpace(buf);
    char* t = (char*)skipNonSpace(p);
    while(p < t) *word_buf++ = *p++;
    *word_buf = 0;

    p = (char*)skipSpace(p);
    t = (char*)skipNonSpace(p);
    if (*t)
        *t++ = 0;
    id = atoi(p);
    p = (char*)skipSpace(t);
    while (*p) {
        const char* s = p;
        t = (char*)skipNonSpace(p);
        if (*t)
            *t++ = 0;
        while ((*p >= 'a' && *p <= 'z') || (*p == '\''))
            ++p;
        if ((p > s) && ((*p == 0) || (*p == ':'))) {
            int  cost = 0;
            if (*p == ':') {
                *p++ = 0;
                cost = atoi(p);
            }
            pyset.insert(TSyllableInfo(s, cost));
        }
        p = (char*)skipSpace(t);
    }
    return pyset.size() > 0;
}


CPinyinTrieMaker::CPinyinTrieMaker()
    : m_RootNode(), m_StateMap()
{
    m_RootNode.m_bExpanded = true;
}
/**********************************************************
    lexicon文件格式：
        以行为单位的文本文件。行中是空格或TAB(1个或多个)分
        隔开的字段。 第一个字段为词，第二个字段是word id。
        后面的字段中，如果一个字段仅仅由小写字母和'构成，
        则认为该字段是该词的一个拼音。行长最大4095;
**********************************************************/
#define RARE_MULTI_PHONETIC_STARTING_ID 140000 /* FIXME */
bool
CPinyinTrieMaker::constructFromLexicon(const char* fileName)
{
    static int  rmp_id = RARE_MULTI_PHONETIC_STARTING_ID;
    static char buf[4096];
    static char word_buf[2048];

    unsigned id;
    bool suc = true;
    std::set<TSyllableInfo> pyset;
    FILE *fp = fopen(fileName, "r");
    if (!fp) return false;
    printf("Adding pinyin and corresponding words..."); fflush(stdout);
    while (fgets(buf, sizeof(buf), fp) != NULL) {
        if (!parseLine(buf, word_buf, id, pyset)) {
            if (word_buf[0] != L'<' && word_buf[0] != 0) {
                if (m_Lexicon.size() < id+1) m_Lexicon.resize(id+1);
                m_Lexicon[id] = std::string(word_buf);
            }
            continue;
        }
        unsigned gbcategory = getPureGBEncoding(word_buf);

        std::set<TSyllableInfo>::const_iterator its = pyset.begin();
        std::set<TSyllableInfo>::const_iterator ite = pyset.end();
        for (; its != ite; ++its) {
            const char *t = its->m_py.c_str();
            int cost = its->m_cost;
            int myid = id;

            if (cost < 0) {
                cost = 30 / (-cost);
                myid = rmp_id ++;
            }

            if (m_Lexicon.size() < myid+1) m_Lexicon.resize(myid+1);
            m_Lexicon[myid] = std::string(word_buf);

            CPinyinTrieMaker::TWordId wid(myid, cost, its->m_cost < 0, gbcategory);
            suc = insertFullPinyinPair(t, wid) && suc;
        }
    }
    fclose(fp);

    printf("\n    %zd primitive nodes", TNode::m_AllNodes.size());  fflush(stdout);

    threadNonCompletePinyin();
    printf("\n    %zd total nodes", TNode::m_AllNodes.size());  fflush(stdout);

    std::string pyPrefix = "";
    //print(stderr, &m_RootNode, pyPrefix);
    printf("\n");  fflush(stdout);

    return suc;
}

CPinyinTrieMaker::CNodeList CPinyinTrieMaker::TNode::m_AllNodes;
CPinyinTrieMaker::TNode::TNode()
    : m_bFullSyllableTransfer(false), m_bExpanded(false), m_WordIdSet(),
      m_Trans(), m_cmbNodes()
{
    m_AllNodes.push_back(this);
}

bool
CPinyinTrieMaker::PNodeSet::operator< (const PNodeSet& another) const
{
    CNodeSet::const_iterator t1 = m_pns->begin();
    CNodeSet::const_iterator t2 = m_pns->end();
    CNodeSet::const_iterator a1 = another.m_pns->begin();
    CNodeSet::const_iterator a2 = another.m_pns->end();
    for (; t1 != t2 && a1 != a2; ++t1, ++a1) {
        if (*t1 < *a1) return true;
        if (*t1 > *a1) return false;
    }
    return (a1 != a2);
}

bool
CPinyinTrieMaker::PNodeSet::operator==(const PNodeSet& another) const
{
    CNodeSet::const_iterator t1 = m_pns->begin();
    CNodeSet::const_iterator t2 = m_pns->end();
    CNodeSet::const_iterator a1 = another.m_pns->begin();
    CNodeSet::const_iterator a2 = another.m_pns->end();
    for (; t1 != t2 && a1 != a2; ++t1, ++a1) {
        if (*t1 != *a1) return false;
    }
    return (a1 == a2 && t1 != t2);
}

static void
parseFullPinyin (const char *pinyin, std::vector<TSyllable> &ret)
{
    char *buf = strdup (pinyin);
    char *p = buf, *q = buf;
    ret.clear();

    while (*p) {
        if (*p == '\'') {
            *p = '\0';
            unsigned s = CPinyinData::encodeSyllable(q);
            ret.push_back (TSyllable(s));
            q = p+1;
        }
        p++;
    }

    if (*q) {
            unsigned s = CPinyinData::encodeSyllable(q);
            ret.push_back (TSyllable(s));
    }

    free(buf);
}

void
CPinyinTrieMaker::print(FILE* fp, TNode* root, std::string& pinyin)
{
    if (root && root->m_WordIdSet.size() > 0) {
        fprintf(fp, "%s", pinyin.c_str());
        CWordSet::const_iterator itId = root->m_WordIdSet.begin();
        CWordSet::const_iterator itIdLast = root->m_WordIdSet.end();
        for (; itId != itIdLast; ++itId) {
            fprintf(fp, " %s", m_Lexicon[itId->anony.m_id].c_str());
        }
        fprintf(fp, "\n");
    }
    if (root) {
        CTrans::const_iterator itTrans = root->m_Trans.begin();
        CTrans::const_iterator itTransLast = root->m_Trans.end();
        for (; itTrans != itTransLast; ++itTrans) {
            const char *str = CPinyinData::decodeSyllable(itTrans->first);
            pinyin = pinyin + str + '\'';
            print(fp, itTrans->second, pinyin);
            pinyin.resize(pinyin.size()-strlen(str)-1);
        }
    }
}

CPinyinTrieMaker::TNode*
CPinyinTrieMaker::insertTransfer(TNode* pnode, unsigned s)
{
    CTrans::const_iterator itt = pnode->m_Trans.find(s);
    CTrans::const_iterator ite = pnode->m_Trans.end();
    if (itt == ite) {
        TNode *p = new TNode();
        p->m_bFullSyllableTransfer = true;
        p->m_bExpanded = true;
        pnode->m_Trans[s] = p;
        return p;
    }
    return itt->second;
}

bool
CPinyinTrieMaker::insertFullPinyinPair(const char* pinyin, TWordId wid)
{
    TNode *pnode = &m_RootNode;
    std::vector<TSyllable> syllables;
    parseFullPinyin (pinyin, syllables);

    std::vector<TSyllable>::const_iterator it = syllables.begin();
    std::vector<TSyllable>::const_iterator ite = syllables.end();

    for (; it != ite; ++it)
        pnode = insertTransfer(pnode, *it);

    insertWordId(pnode->m_WordIdSet, wid);
    return true;
}

CPinyinTrieMaker::TNode*
CPinyinTrieMaker::addCombinedTransfers (TNode *pnode, unsigned s, const CNodeSet& nodes)
{
    assert (!nodes.empty());

    TNode *p = NULL;
    if (nodes.size() == 1) {
        p = *(nodes.begin());
    } else {
        p = new TNode();
        p->m_cmbNodes = nodes;
        m_StateMap[&p->m_cmbNodes] = p;

        CNodeSet::const_iterator it = nodes.begin();
        CNodeSet::const_iterator ite = nodes.end();
        for (; it != ite; ++it)
            p->m_WordIdSet.insert ((*it)->m_WordIdSet.begin(), (*it)->m_WordIdSet.end());
    }

    pnode->m_Trans[s] = p;
    return p;
}

void
CPinyinTrieMaker::combineInitialTrans (TNode *pnode)
{
    std::map<unsigned, CNodeSet> combTrans;

    CTrans::const_iterator itTrans = pnode->m_Trans.begin();
    CTrans::const_iterator itTransLast = pnode->m_Trans.end();
    for (; itTrans != itTransLast; ++itTrans) {
        TSyllable s = (TSyllable) itTrans->first;
        if (s.initial) {
            s.final = s.tone = 0;
            combTrans[s].insert(itTrans->second);
        }
    }

    std::map<unsigned, CNodeSet>::const_iterator itCombTrans = combTrans.begin();
    for (; itCombTrans != combTrans.end(); ++itCombTrans)
        addCombinedTransfers (pnode, itCombTrans->first, itCombTrans->second);
}

void
CPinyinTrieMaker::expandCombinedNode (TNode *pnode)
{
    assert (pnode->m_cmbNodes.size() >= 1);

    std::map<unsigned, CNodeSet> combTrans;
    CNodeSet::const_iterator itNode = pnode->m_cmbNodes.begin();
    CNodeSet::const_iterator itNodeLast = pnode->m_cmbNodes.end();
    for (; itNode != itNodeLast; ++itNode) {
        CTrans::const_iterator itTrans = (*itNode)->m_Trans.begin();
        CTrans::const_iterator itTransLast = (*itNode)->m_Trans.end();
        for (; itTrans != itTransLast; ++itTrans)
            combTrans[itTrans->first].insert(itTrans->second);
    }

    std::map<unsigned, CNodeSet>::const_iterator itCombTrans = combTrans.begin();
    for (; itCombTrans != combTrans.end(); ++itCombTrans)  {
        TNode* p = NULL;
        unsigned s = itCombTrans->first;
        CNodeSet nodes = itCombTrans->second;

        CStateMap::const_iterator itStateMap = m_StateMap.find(&nodes);
        if (itStateMap != m_StateMap.end())
            p = itStateMap->second;
        else
            p = addCombinedTransfers (pnode, s, nodes);

        pnode->m_Trans[s] = p;
    }

    pnode->m_bExpanded = true;
}

bool
CPinyinTrieMaker::threadNonCompletePinyin(void)
{
    CNodeList::const_iterator itNode = TNode::m_AllNodes.begin();
    for (; itNode != TNode::m_AllNodes.end(); ++itNode) {
        TNode* pnode = *itNode;
        if (pnode->m_bExpanded)
            combineInitialTrans (pnode);
        else
            expandCombinedNode (pnode);
    }
    return true;
}

bool
CPinyinTrieMaker::write(const char* fileName, CWordEvaluator* psrt,
                        bool revert_endian)
{
    bool suc = false;
    FILE* fp = fopen(fileName, "wb");
    if (fp != NULL) {
        suc = write(fp, psrt, revert_endian);
        fclose(fp);
    }
    return suc;
}

bool
CPinyinTrieMaker::write(FILE *fp, CWordEvaluator* psrt, bool revert_endian)
{
    bool suc = true;
    static TWCHAR wbuf[1024];

    std::map<TNode*, unsigned int> nodeOffsetMap;

    unsigned int nWord = m_Lexicon.size();
    unsigned int nNode = TNode::m_AllNodes.size();
    unsigned int lexiconOffset;
    unsigned int offset = sizeof(unsigned int) * 3;

    CNodeList::const_iterator itNode = TNode::m_AllNodes.begin();
    CNodeList::const_iterator itNodeLast = TNode::m_AllNodes.end();
    for (; itNode != itNodeLast; ++itNode) {
        nodeOffsetMap[*itNode] = offset;
        offset += CPinyinTrie::TNode::size_for((*itNode)->m_Trans.size(),
                                               (*itNode)->m_WordIdSet.size());
    }
    lexiconOffset = offset;
    CLexicon::const_iterator itWordStr = m_Lexicon.begin();
    CLexicon::const_iterator itWordStrLast = m_Lexicon.end();
    for (; itWordStr != itWordStrLast; ++itWordStr) {
        MBSTOWCS(wbuf, itWordStr->c_str(), 1024);
        int sz = WCSLEN(wbuf);
        offset += (sz+1)*sizeof(TWCHAR);
    }

    Writer f(fp, revert_endian);
    
    suc = f.write(nWord);
    suc = f.write(nNode);
    suc = f.write(lexiconOffset);

    itNode = TNode::m_AllNodes.begin();
    itNodeLast = TNode::m_AllNodes.end();
    
    for (; itNode != itNodeLast && suc; ++itNode) {
        CPinyinTrie::TNode outNode;
        TNode *pnode = *itNode;

        outNode.m_nTransfer = pnode->m_Trans.size();
        outNode.m_nWordId = pnode->m_WordIdSet.size();
        outNode.m_bFullSyllableTransfer = pnode->m_bFullSyllableTransfer;
        outNode.m_csLevel = 0;

        CWordSet::const_iterator itId = pnode->m_WordIdSet.begin();
        CWordSet::const_iterator itIdLast = pnode->m_WordIdSet.end();
        for (; itId != itIdLast && outNode.m_csLevel<3; ++itId) {
            if (outNode.m_csLevel < itId->anony.m_csLevel)
                outNode.m_csLevel = itId->anony.m_csLevel;
        }

        suc = f.write(outNode);

        CTrans::const_iterator itTrans = pnode->m_Trans.begin();
        CTrans::const_iterator itTransLast = pnode->m_Trans.end();
        for (; itTrans != itTransLast && suc; ++itTrans) {
            CPinyinTrie::TTransUnit tru;
            tru.m_Syllable = itTrans->first;
            tru.m_Offset = nodeOffsetMap[itTrans->second];
            assert(tru.m_Offset != 0 && tru.m_Offset < lexiconOffset);
            suc = f.write(tru);
        }

        CWordVec vec;
        itId = pnode->m_WordIdSet.begin();
        itIdLast = pnode->m_WordIdSet.end();
        for (; itId != itIdLast; ++itId)
            vec.push_back(TWordInfo(*itId, psrt->getCost(*itId), psrt->isSeen(*itId)));
        std::make_heap(vec.begin(), vec.end());
        std::sort_heap(vec.begin(), vec.end());

        CWordVec::const_iterator itv = vec.begin();
        CWordVec::const_iterator itve = vec.end();
        for (; itv != itve && suc; ++itv) {
            CPinyinTrie::TWordIdInfo wi;
            wi.m_id = itv->m_id.anony.m_id;
            assert (wi.m_id < nWord);
            wi.m_csLevel = itv->m_id.anony.m_csLevel;
            wi.m_bSeen = ((itv->m_bSeen)?(1):(0));
            wi.m_cost = itv->m_id.anony.m_cost;
            suc = f.write(wi);
        }
    }

    itWordStr = m_Lexicon.begin();
    itWordStrLast = m_Lexicon.end();
    for (; itWordStr != itWordStrLast && suc; ++itWordStr) {
        MBSTOWCS(wbuf, itWordStr->c_str(), 1024);
        int sz = WCSLEN(wbuf);
        suc = f.write(wbuf, (sz+1));
    }
    return suc;
}

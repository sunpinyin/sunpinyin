#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <deque>

#include "pytrie.h"
#include "pinyin_data.h"

#ifdef HAVE_SYS_MMAN_H
#include <sys/mman.h>
#endif

bool
CPinyinTrie::isValid(const TNode* pnode, bool allowNonComplete, unsigned csLevel)
{
    if ((pnode != NULL) && (csLevel <= pnode->m_csLevel))
        return (allowNonComplete || (pnode->m_bFullSyllableTransfer == 1));
    return false;
}

int
CPinyinTrie::lengthAt(unsigned int idx) const
{
    if (idx < getWordCount() -1 ) {
       return (m_words[idx+1] - m_words[idx]) - 1;
    } else if (idx == getWordCount() - 1) {
        return (((TWCHAR*)(m_mem+m_Size))-m_words[idx])-1;
    }
    return 0;
}

unsigned int
CPinyinTrie::getSymbolId(const TWCHAR* wstr)
{
    std::map<wstring, unsigned>::const_iterator it;

    it = m_SymbolMap.find(wstring(wstr));
    if (it != m_SymbolMap.end())
        return it->second;
    return 0;
}

unsigned int
CPinyinTrie::getSymbolId(const wstring & wstr)
{
    std::map<wstring, unsigned>::const_iterator it;

    it = m_SymbolMap.find(wstr);
    if (it != m_SymbolMap.end())
        return it->second;
    return 0;
}

void
CPinyinTrie::free(void)
{
    if (m_mem) {
#ifdef HAVE_SYS_MMAN_H
        munmap (m_mem, m_Size);
#else
        delete []m_mem;
#endif
        m_mem = NULL;
    }
    if (m_words) {
        delete []m_words;
        m_words = NULL;
    }
    m_SymbolMap.clear();
}

bool
CPinyinTrie::load(const char *fname)
{
    free();

    bool suc = false;
    int fd = open(fname, O_RDONLY);
    if (fd == -1) return false;

    m_Size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

#ifdef HAVE_SYS_MMAN_H
    suc = (m_mem = (char *) mmap (NULL, m_Size, PROT_READ, MAP_SHARED, fd, 0)) != MAP_FAILED;
#else
    suc = (m_mem = new char [m_Size]) != NULL;
    suc = suc && (read (fd, m_mem, m_Size) > 0);
#endif
    close(fd);

    suc = suc && ((m_words = new TWCHAR*[getWordCount()]) != NULL);

    if (suc) {
        TWCHAR *p = (TWCHAR *)(m_mem + getStringOffset());
        for (int i=0, sz=getWordCount(); i < sz; ++i) {
            m_words[i] = p;
            while (*p++)
                ;
        }
        for (unsigned i=1; i < 100; ++i) {
            if (*m_words[i] != WCH_NULL && *m_words[i] != WCH_LESSTHAN)
                m_SymbolMap[wstring(m_words[i])] = i;
        }
    }
    return suc;
}

void
CPinyinTrie::print(FILE *fp) const
{
    std::string prefix;
    print(getRootNode(), prefix, fp);
}

void
CPinyinTrie::print(const TNode* pRoot, std::string& prefix, FILE *fp) const
{
    static char buf[1024];
    if (pRoot->m_nWordId > 0) {
        fprintf(fp, "%s", prefix.c_str());
        if (pRoot->m_csLevel)
            fprintf(fp, "(GBK+)");
        unsigned int sz = pRoot->m_nWordId;
        const TWordIdInfo *pwids = pRoot->getWordIdPtr();
        for (unsigned int i = 0; i < sz; ++i) {
            unsigned int id = pwids[i].m_id;
            const TWCHAR *pw = operator[](id);
            int len = WCSLEN(pw);
            if (len != lengthAt(id)) {
                printf(" (lengthAt %d error) ", id);
            }
            WCSTOMBS(buf, pw, 1024);
            fprintf(fp, " %s", buf);
            if (pwids[i].m_bSeen == 0)
                fprintf(fp, "[x]");
            else
                fprintf(fp, "[o]");
        }
        fprintf(fp, "\n");
    }
    unsigned int sz = pRoot->m_nTransfer;
    const TTransUnit* ptrans = pRoot->getTrans();
    for (unsigned int i = 0; i < sz; ++i) {
        unsigned s = ptrans[i].m_Syllable;
        const TNode *pch = transfer(pRoot, s);
        const char *str = CPinyinData::decodeSyllable(s);
        if (!str) break;
        prefix = prefix + str + '\'';
        print(pch, prefix, fp);
        prefix.resize(prefix.size()-strlen(str)-1);
    }
}

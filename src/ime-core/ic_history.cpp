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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <cassert>
#include <sys/stat.h>
#include <sys/types.h>
#include <algorithm>
#include "ic_history.h"

const unsigned CICHistory::DCWID = (unsigned)-1;

CICHistory::~CICHistory()
{
}

static bool bBigramHistoryInited = false;
const size_t  CBigramHistory::contxt_memory_size = 8192;
std::set<unsigned>  CBigramHistory::s_stopWords;

/**
* Adding stop words including :
* 的 地 得 着 了 你 我 他 她 它 你们 我们 他们
* 们
* ??是 使 将
*/

void CBigramHistory::initClass()
{
    if (bBigramHistoryInited == false) {
        bBigramHistoryInited = true;
        s_stopWords.clear();

        s_stopWords.insert(0);     //unknown world
        s_stopWords.insert(DCWID); //seperator word id used by history memory interanlly

        s_stopWords.insert(44751); //的
        s_stopWords.insert(21410); //地
        s_stopWords.insert(71373); //得
        s_stopWords.insert(72583); //着
        s_stopWords.insert(1701);  //了

        s_stopWords.insert(35483); //你
        s_stopWords.insert(34834); //我
        s_stopWords.insert(17613); //他
        s_stopWords.insert(30394); //她
        s_stopWords.insert(19066); //它
        s_stopWords.insert(35484); //你们
        s_stopWords.insert(34838); //我们
        s_stopWords.insert(17617); //他们
        s_stopWords.insert(30395); //她们
        s_stopWords.insert(19069); //它们

        s_stopWords.insert(17345);  //们
    }
}

//FIXME: CBigramHistory need to be thread safe
CBigramHistory::CBigramHistory() : m_memory(), m_unifreq(), m_bifreq()
{

}

CBigramHistory::~CBigramHistory()
{
}

bool CBigramHistory::memorize(unsigned* its_wid, unsigned* ite_wid)
{
    TBigram bigram(DCWID, DCWID);

    // First , we insert an DC word id before the context history
    // to seperated from previous stream.
    if (m_memory.size() == contxt_memory_size) {
        TBigram hb;
        hb.first = m_memory.front();
        m_memory.pop_front();
        hb.second = m_memory.front();

        decUniFreq(hb.first);
        decBiFreq(hb);
    }
    m_memory.push_back(DCWID);

    //Now trying to memorize new stream and forget oldest
    for (; its_wid != ite_wid; ++its_wid) {
        if (m_memory.size() == contxt_memory_size) {
            TBigram hb;
            hb.first = m_memory.front();
            m_memory.pop_front();
            hb.second = m_memory.front();

            decUniFreq(hb.first);
            decBiFreq(hb);
        }
        bigram.first = bigram.second;
        bigram.second = *its_wid;
        m_memory.push_back(*its_wid);
        incUniFreq(bigram.second);
        incBiFreq(bigram);
    }
    return true;
}

void CBigramHistory::clear()
{
    m_memory.clear();
    m_unifreq.clear();
    m_bifreq.clear();
}

double CBigramHistory::pr(unsigned* its_wid, unsigned* ite_wid)
{
    TBigram bigram(DCWID, DCWID);
    if (its_wid != ite_wid) {
        --ite_wid;
        bigram.second = *ite_wid;
        if (its_wid != ite_wid)
            bigram.first = *(ite_wid-1);
    }
    return pr(bigram);
}

double CBigramHistory::pr(unsigned* its_wid, unsigned* ite_wid, unsigned wid)
{
    TBigram bigram(DCWID, DCWID);
    if (its_wid != ite_wid)
        bigram.first = *(ite_wid-1);
    bigram.second = wid;
    return pr(bigram);
}

inline uint16_t swap16(uint16_t x)
{
    return ((x << 8) | ((x >> 8) & 0xff));
}

inline uint32_t swap32(uint32_t x)
{
    return ((swap16(x) << 16) | (swap16(x >> 16) & 0xffff));
}

bool CBigramHistory::bufferize(void** buf_ptr, size_t* sz)
{
    *buf_ptr = NULL;
    *sz = 0;
    try {
        *sz = sizeof(uint32_t) * m_memory.size();
        if (*sz > 0) {
            *buf_ptr = malloc(*sz); // malloc for C compatible
#ifdef WORDS_BIGENDIAN
            std::copy(m_memory.begin(), m_memory.end(), (uint32_t*)*buf_ptr);
#else
            std::transform(m_memory.begin(), m_memory.end(), (uint32_t*)*buf_ptr, swap32);
#endif
        }
        return true;
    } catch (...) {
        if (*buf_ptr)
            free(*buf_ptr);
        *buf_ptr = NULL;
        *sz = 0;
    }
    return false;
}

bool CBigramHistory::loadFromFile (const char *fname)
{
    m_history_path = fname;

    bool suc = false;
    int fd = open (fname, O_CREAT, 0600);
    if (fd == -1) {
        perror("fopen bi-gram");
        return suc;
    }
    
    struct stat info;
    fstat (fd, &info);
    void* buf = malloc (info.st_size);
    
    if (buf) {
        read (fd, buf, info.st_size);
        suc = loadFromBuffer (buf, info.st_size);
        free (buf);
    }  
    close (fd);
    return suc;
}

bool CBigramHistory::saveToFile(const char *fname)
{
    if (!fname)
        fname = m_history_path.c_str();

    bool suc = false;
    size_t sz = 0;
    void* buf = NULL;
    if (bufferize(&buf, &sz) && buf) {
        FILE* fp = fopen (fname, "wb");
        if (fp) {
            suc = (fwrite(buf, 1, sz, fp) == sz);
            fclose(fp);
        }
        free(buf);
    }
    return suc;
}

bool CBigramHistory::loadFromBuffer(void* buf_ptr, size_t sz)
{
    clear();

    sz /= sizeof(uint32_t);
    uint32_t *pw = (uint32_t *)buf_ptr;

    if (pw && sz > 0) {
#ifndef WORDS_BIGENDIAN
        std::transform(pw, pw+sz, pw, swap32);
#endif
        TBigram bigram(DCWID, DCWID);
        for (unsigned i=0; i < sz; ++i) {
            bigram.first = bigram.second;
            bigram.second = *pw++;
            m_memory.push_back(bigram.second);
            incUniFreq(bigram.second);
            incBiFreq(bigram);
        }
    }
    return true;
}

double CBigramHistory::pr(TBigram& bigram)
{
    int uf0 = uniFreq(bigram.first);
    int bf = biFreq(bigram);
    int uf1 = uniFreq(bigram.second);
    double pr = 0.0;
    pr += 0.68*double(bf)/double(uf0+0.5);
    pr += 0.32*double(uf1)/double(m_memory.size() + (contxt_memory_size-m_memory.size())/10);
    //if (pr != 0) printf("cache pr(%d|%d) = %lf\n", bigram.second, bigram.first, pr);
    return pr;
}

int  CBigramHistory::uniFreq(TUnigram& ug)
{
    int freq = 0;
    if (s_stopWords.find(ug) == s_stopWords.end()) {
        TUnigramPool::iterator it = m_unifreq.find(ug);
        if (it != m_unifreq.end()) {
            freq = it->second;
        }
    }
    //if (freq != 0) printf("uniFreq[%d]-->%d\n", ug, freq);
    return freq;
}

int  CBigramHistory::biFreq(TBigram& bg)
{
    int freq = 0;
    //std::set<unsigned>::const_iterator ite = s_stopWords.end();
    if (bg.first != DCWID && bg.second != DCWID) {
        TBigramPool::const_iterator it = m_bifreq.find(bg);
        if (it != m_bifreq.end())
            freq =  it->second;
    }

    //if (freq != 0) printf("biFreq[%d,%d]-->%d\n", bg.first, bg.second, freq);
    return freq;
}

void CBigramHistory::decUniFreq(TUnigram& ug)
{
    TUnigramPool::iterator it = m_unifreq.find(ug);
    if (it != m_unifreq.end()) {
        if (it->second > 1)
            --(it->second);
        else
            m_unifreq.erase(it);
    }
}

bool CBigramHistory::seenBefore(unsigned wid)
{
    return (wid != DCWID && s_stopWords.find(wid) == s_stopWords.end() &&
            m_unifreq.find(wid) != m_unifreq.end());
}

void CBigramHistory::decBiFreq(TBigram& bg)
{
    TBigramPool::iterator it = m_bifreq.find(bg);
    if (it != m_bifreq.end()) {
        if (it->second > 1)
            --(it->second);
        else
            m_bifreq.erase(it);
    }
}

void CBigramHistory::incUniFreq(TUnigram& ug)
{
    ++m_unifreq[ug];
    //printf("Remebering uniFreq[%d]-->%d\n", ug, m_unifreq[ug]);
}

void CBigramHistory::incBiFreq(TBigram& bg)
{
    ++m_bifreq[bg];
    //printf("Remebering biFreq[%d,%d]-->%d\n", bg.first, bg.second, m_bifreq[bg]);
}

// so far, it's very expensive to erase a word from bigram pairs, need to design
// a better data structure for this.
//
// And Even though, we may also need to remove the individual characters in this
// word (identified by wid), which is current infeasible, 
//
// Here are what we need to do:
//   1. get the wstring by word id from userdict
//   2. iterate the character in this wstring
//   3. get the word id from each character from system lexicon (not supported yet)
//   4. remove the unigrams and bigrams of each character, and the entire word
//
void CBigramHistory::forget(unsigned wid)
{
    TUnigramPool::iterator uni_it = m_unifreq.find (wid);
    if (uni_it != m_unifreq.end())
        m_unifreq.erase(uni_it);

    TBigramPool::iterator it  = m_bifreq.begin();
    TBigramPool::iterator ite = m_bifreq.end();

    while (it != ite) {
        TBigram bigram = it->first;
        
        if (bigram.first == wid || bigram.second == wid)
            m_bifreq.erase (it++);
        else
            ++it;
    }
}

void CBigramHistory::forget(unsigned *its_wid, unsigned *ite_wid)
{
    for (; its_wid < ite_wid; ++its_wid) {
        TBigram bigram(*its_wid, DCWID);

        if (its_wid+1 != ite_wid)
            bigram.second = *(its_wid+1);

        TBigramPool::iterator it = m_bifreq.find(bigram);
        if (it != m_bifreq.end())
            m_bifreq.erase(it);
    }
}

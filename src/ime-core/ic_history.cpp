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

#include <stdio.h>
#include <sys/stat.h>
#include <algorithm>
#include "ic_history.h"

const unsigned CICHistory::DCWID = (unsigned)-1;

CICHistory::~CICHistory()
{
}

bool CICHistory::seenBefore(unsigned wid)
{
    return false;
}

bool CICHistory::memorize(unsigned* its_wid, unsigned* ite_wid)
{
    return true;
}

double CICHistory::pr(unsigned* its_wid, unsigned* ite_wid)
{
    return 0.0;
}

double CICHistory::pr(unsigned* its_wid, unsigned* ite_wid, unsigned wid)
{
    return 0.0;
}

bool CICHistory::bufferize(void** buf_ptr, size_t* sz)
{
    *buf_ptr = NULL;
    *sz = 0;
    return true;
}

bool CICHistory::loadFromBuffer(void* buf_ptr, size_t sz)
{
    return true;
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

        s_stopWords.insert(67659); //的
        s_stopWords.insert(24261); //地
        s_stopWords.insert(37471); //得
        s_stopWords.insert(68920); //着
        s_stopWords.insert(5071);  //了

        s_stopWords.insert(8396);  //你
        s_stopWords.insert(40646); //我
        s_stopWords.insert(6755);  //他
        s_stopWords.insert(28369); //她
        s_stopWords.insert(30143); //它
        s_stopWords.insert(8398);  //你们
        s_stopWords.insert(40648); //我们
        s_stopWords.insert(6759);  //他们
        s_stopWords.insert(28370); //她们
        s_stopWords.insert(30114); //它们

        s_stopWords.insert(7121);  //们
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

bool CBigramHistory::loadFromFile (const char *fname) {
     bool suc = false;
     FILE* fp = fopen (fname, "w+");

     if (fp) {
         struct stat info;

         fstat (fileno(fp), &info);
         void* buf = malloc (info.st_size);

         if (buf) {
             fread (buf, info.st_size, 1, fp);
             suc = loadFromBuffer (buf, info.st_size);
             free (buf);
         }  

         fclose (fp);
     }  
     return suc;
}

bool CBigramHistory::loadFromBuffer(void* buf_ptr, size_t sz)
{
    m_memory.clear();
    m_unifreq.clear();
    m_bifreq.clear();

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

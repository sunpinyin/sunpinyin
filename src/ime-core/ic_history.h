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

#ifndef _SUNPINYIN_CONTEXT_HISTORY_H
#define _SUNPINYIN_CONTEXT_HISTORY_H

#include "portability.h"

#include <map>
#include <deque>
#include <set>

/**
 * A forget all history memory
 */
class CICHistory {
public:
    /** don't care word id, or seperator word id */
    static const uint32_t DCWID;

    virtual ~CICHistory();

    virtual bool seenBefore(uint32_t wid) = 0;

    /**
     * memorize the context stream pointed by [its_wid, ite_wid)
     */
    virtual bool memorize(uint32_t* its_wid, uint32_t* ite_wid) = 0;
    virtual void clear() = 0;

    /**
     * remove a word id from history cache
     */
    virtual void forget(uint32_t wid) = 0;
    virtual void forget(uint32_t* its_wid, uint32_t* ite_wid) = 0;

    /**
     * @param its_wid is the first word pointer of the context stream
     * @param ite_wid is the last (exclusive) word pointer of the context stream
     * @return pr(*(ite_wid-1) | *its_wid, ..., *(ite_wid-2))
     * The return value could be zero, i.e. no need to smooth the probabilities
     */
    virtual double pr(uint32_t* its_wid, uint32_t* ite_wid) = 0;

    /**
     * @param its_wid is the first word pointer of the history stream
     * @param ite_wid is the last (exclusive) word pointer of the history stream
     * @return pr(*wid | *its_wid, ..., *(ite_wid-1))
     * The return value could be zero, i.e. no need to smooth the probabilities
     */
    virtual double pr(uint32_t* its_wid,
                      uint32_t* ite_wid,
                      uint32_t wid) = 0;

    /**
     * allocate a buffer, and put the context memory's contect into it
     * @param buf_ptr would be stored the buffer pointer
     * @param sz would be the size in byte of the buffer allocated
     * @return false on error
     * Note: the buf_ptr should be used free(*buf_ptr) to free after usage
     */
    virtual bool
    bufferize(void** buf_ptr, size_t* sz) = 0;

    /**
     * Load context memory according to the buf
     * @param buf_ptr uffer pointer
     * @param sz is the size in byte of the buffer
     * @return false on error
     * call with buf_ptr with NULL value would clear the context memory
     */
    virtual bool loadFromBuffer(void* buf_ptr, size_t sz) = 0;
    virtual bool loadFromFile(const char *fname) = 0;
    virtual bool saveToFile(const char *fname = NULL) = 0;

    virtual void addStopWords(const std::set<uint32_t>& stopWords) = 0;

    virtual void initStopWords() = 0;
};

class CBigramHistory : public CICHistory {
public:
    static void initClass();

    CBigramHistory();

    virtual ~CBigramHistory();

    virtual bool seenBefore(uint32_t wid);

    virtual bool memorize(uint32_t* its_wid, uint32_t* ite_wid);
    virtual void clear();

    virtual void forget(uint32_t wid);
    virtual void forget(uint32_t* its_wid, uint32_t* ite_wid);

    /**
     * @param its_wid is the first word pointer of the context stream
     * @param ite_wid is the last (exclusive) word pointer of the context stream
     * @return pr(*(ite_wid-1) | *(ite_wid-2))
     */
    virtual double pr(uint32_t* its_wid, uint32_t* ite_wid);

    /**
     * @param its_wid is the first word pointer of the history stream
     * @param ite_wid is the last (exclusive) word pointer of the history stream
     * @return pr(*wid | *(ite_wid-1))
     */
    virtual double pr(uint32_t* its_wid,
                      uint32_t* ite_wid,
                      uint32_t wid);

    virtual bool bufferize(void** buf_ptr, size_t* sz);

    virtual bool loadFromBuffer(void* buf_ptr, size_t sz);
    virtual bool loadFromFile(const char *fname);
    virtual bool saveToFile(const char *fname = NULL);

    virtual void addStopWords(const std::set<uint32_t>& stopWords);
    virtual void initStopWords();

protected:
    typedef uint32_t TWordId;
    typedef std::pair<TWordId, TWordId>           TBigram;
    typedef TWordId TUnigram;
    typedef std::map<TBigram, int>                TBigramPool;
    typedef std::map<TUnigram, int>               TUnigramPool;
    typedef std::deque<TWordId>                   TContextMemory;

    static const size_t contxt_memory_size;
    static const double focus_memory_ratio;

    TContextMemory m_memory;
    TUnigramPool m_unifreq;
    TBigramPool m_bifreq;

    std::string m_history_path;
    std::set<uint32_t>  m_stopWords;

protected:
    double pr(TBigram& bg);
    int  uniFreq(TUnigram& ug);
    int  biFreq(TBigram& bg);

    void decUniFreq(TUnigram& ug);
    void decBiFreq(TBigram& bg);
    void incUniFreq(TUnigram& ug);
    void incBiFreq(TBigram& bg);
};

#endif

// -*- indent-tabs-mode: nil -*- vim:et:ts=4

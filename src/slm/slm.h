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

#ifndef _SUN_AGC_SLM_H
#define _SUN_AGC_SLM_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "../portability.h"

#include <stdio.h>

/**
 * Thread slm make the following modifications to simple back-off language model
 *    -# Word id are limited to 18 bits, about 240K word ids
 *    -# Compact all float value of -log(pr) into 65536 (16 bits)
 *       level and use a table to map the index to a float value;
 *    -# Compact all float value of -log(pr) into 16384 (14 bits)
 *       level and use a table to map the index to a float value;
 *    -# threading infomation embed into binary model file. Threading include
 *         - bol(back-off-level) from current level
 *         - bon(back-off-node)'s index in the bol level array
 *         .
 *       The thread could be used:
 *         - when leaf node are arrived, it could use (bol,bon) as history for
 *           history node.
 *         - when a word could not be found in current node (cl, cn)'s children,
 *           searching could be transfered to (bol, bon) directly and continue
 *           searching the target word
 *    -# Add a basic type TState in Language model, a state is pair of\n
 *           (level, array_idx_of_the level)
 *    -# change all get probability interface to\n
 *          double transfer(TState& history, unsigned int wid, TState& result);
 */
class CThreadSlm {
public:
    enum {
        BITS_BOW        = 14,
        BITS_PR         = 16,
        ID_NOT_WORD     = 69,
    };

    /**
    * (level:idx) located a state in the language model very well
    * Please note the psuedo unigram state, with level == 0, but idx > 0
    * it's for used with bigram cache model
    */
    union TState{
        TState(const TState& b) : m_all(b.m_all) { }
        TState(unsigned level=0, unsigned idx=0) { anony.m_Level=level; anony.m_Idx=idx; }

        inline TState& operator++()              { ++anony.m_Idx; return *this; }

        inline void setIdx(unsigned int idx)     { anony.m_Idx = idx; }
        inline void setLevel(unsigned int lvl)   { anony.m_Level = lvl; }

        inline unsigned int getLevel() const     { return anony.m_Level; }
        inline unsigned int getIdx() const       { return anony.m_Idx; }
        inline operator unsigned() const         { return m_all; } //((anony.m_Level << 24) + anony.m_Idx); }
        inline bool operator==(const TState & b) const  { return m_all == b.m_all; }
        inline bool operator< (const TState & b) const  { return unsigned(*this) <  unsigned(b); }

    private:
        unsigned int m_all;
#ifndef WORDS_BIGENDIAN
        struct TAnonymous {
            unsigned m_Idx   :24;
            unsigned m_Level : 8;
        } anony;
#else
        struct TAnonymous {
            unsigned m_Level : 8;
            unsigned m_Idx   :24;
        } anony;
#endif
    };

    /**
     * Machine dependent
     */
    struct TNode {
    public:
        unsigned int wid() const
        {
            return m_wid;
        }

        unsigned int bow() const
        {
            return m_bow;
        }

        unsigned int pr()  const
        {
            return m_pr;
        }

        unsigned int bon() const
        {
            return m_bon;
        }

        unsigned int bol() const
        {
            return m_bol;
        }

        unsigned int ch()  const
        {
            return ((m_ch_hi << 16) + m_ch_lo);
        }

        void set_wid(unsigned int wid)
        {
            m_wid = wid;
        }

        void set_bow(unsigned int bow)
        {
            m_bow = bow;
        }

        void set_pr(unsigned int pr)
        {
            m_pr = pr;
        }

        void set_bon(unsigned int bon)
        {
            m_bon = bon;
        }

        void set_bol(unsigned int bol)
        {
            m_bol = bol;
        }

        void set_ch(unsigned int ch)
        {
            m_ch_hi=((ch >> 16) & 0x7F);
            m_ch_lo=(ch & 0xFFFF);
        }

    protected:
#ifndef WORDS_BIGENDIAN
        unsigned m_wid       :18;
        unsigned m_bow       :14;
        unsigned m_pr        :16;
        unsigned m_ch_lo     :16;
        unsigned m_bon       :23;
        unsigned m_bol       : 2;
        unsigned m_ch_hi     : 7;
#else
        unsigned m_ch_hi     : 7;
        unsigned m_bol       : 2;
        unsigned m_bon       :23;
        unsigned m_ch_lo     :16;
        unsigned m_pr        :16;
        unsigned m_bow       :14;
        unsigned m_wid       :18;
#endif

    private:
        /**
         * Machine dependent
        union TChildIdx {
        public:
            inline TChildIdx(unsigned val) : m_all(val) { }
            inline TChildIdx(const TChildIdx& b) : m_all(b.m_all) { }
            inline TChildIdx(unsigned int hi, unsigned lo) : m_all(0) { anony.m_hi = hi; anony.m_lo = lo; }

            inline unsigned int lo() { return anony.m_lo; }
            inline unsigned int hi() { return anony.m_hi; }
            inline unsigned int all(){ return m_all; }

            inline unsigned int set_lo(unsigned int lo) { return (anony.m_lo = lo); }
            inline unsigned int set_hi(unsigned int hi) { return (anony.m_hi = hi); }
            inline unsigned int set_all(unsigned int all) { return (m_all = all); }

        private:
            unsigned int m_all;
#ifndef WORDS_BIGENDIAN
            struct TAnony {
                unsigned m_lo :16;
                unsigned m_hi : 7;
                unsigned NOUSE: 9;
            } anony;
#else
            struct TAnony {
                unsigned NOUSE: 9;
                unsigned m_hi : 7;
                unsigned m_lo :16;
            } anony;
#endif
        };
        */
    };

    /**
     * Machine dependent
     */
    struct TLeaf {
    public:
        inline unsigned int wid() const { return m_wid; }
        inline unsigned int bon() const { return m_bon; }
        inline unsigned int bol() const { return m_bol; }
        inline unsigned int pr()  const { return ((m_pr_hi << 14) + m_pr_lo); }

        inline void set_wid(unsigned int wid) { m_wid = wid; }
        inline void set_bon(unsigned int bon) { m_bon = bon; }
        inline void set_bol(unsigned int bol) { m_bol = bol; }
        inline void set_pr(unsigned int pr)   { m_pr_hi = ((pr >> 14) & 0x3); m_pr_lo = pr & 0x3FFF; }

    protected:
#ifndef WORDS_BIGENDIAN
        unsigned m_wid       :18;
        unsigned m_pr_lo     :14;
        unsigned m_bon       :23;
        unsigned m_bol       : 2;
        unsigned m_pr_hi     : 2;
#else
        unsigned m_pr_hi     : 2;
        unsigned m_bol       : 2;
        unsigned m_bon       :23;
        unsigned m_pr_lo     :14;
        unsigned m_wid       :18;
#endif

    private:
    /*
        union TPr {
        public:
            inline TPr(unsigned int val) : m_all(val) { }
            inline TPr(const TPr & b) : m_all(b.m_all) { }
            inline TPr(unsigned int hi, unsigned lo) : m_all(0) { anony.m_hi=hi, anony.m_lo=lo; }

            inline unsigned int lo() { return anony.m_lo; }
            inline unsigned int hi() { return anony.m_hi; }
            inline unsigned int all(){ return m_all; }

            inline unsigned int set_lo(unsigned int lo) { return (anony.m_lo = lo); }
            inline unsigned int set_hi(unsigned int hi) { return (anony.m_hi = hi); }
            inline unsigned int set_all(unsigned int all) { return (m_all = all); }

        private:
            unsigned int m_all;
#ifndef WORDS_BIGENDIAN
            struct TAnony {
                unsigned m_lo  :14;
                unsigned m_hi  : 2;
                unsigned NONUSE:16;
            } anony;
#else
            struct TAnony {
                unsigned NONUSE:16;
                unsigned m_hi  : 2;
                unsigned m_lo  :14;
            } anony;
#endif
        };
        */
    };

public:
    CThreadSlm()
        : m_N(0), m_UseLogPr(0), m_Levels(NULL), m_LevelSizes(NULL),
          m_bowTable(NULL), m_prTable(NULL), m_bMMap(false), m_buf(NULL) { }

    ~CThreadSlm() { free(); }

    bool
    load(const char* fname, bool MMap=false);

    unsigned 
    isUseLogPr() const
    { return m_UseLogPr; }

    void
    free();

    double
    transferNegLog(TState history, unsigned int wid, TState& result);

    double
    transfer(TState history, unsigned int wid, TState& result);

    TState
    history_state_of(TState st);

    TState&
    historify(TState& st);

    unsigned int
    lastWordId(TState st);

protected:
    double
    rawTransfer(TState history, unsigned int wid, TState& result);

protected:
    typedef  void*   PtrVoid;

    unsigned  m_N;
    unsigned  m_UseLogPr;
    void    **m_Levels;
    unsigned *m_LevelSizes;
    float    *m_bowTable;
    float    *m_prTable;

private:
    ssize_t   m_bufSize;
    bool      m_bMMap;
    char     *m_buf;
};

#endif

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

#ifndef SUNPINYIN_UI_OBJECTS_H
#define SUNPINYIN_UI_OBJECTS_H

#include "portability.h"

#include <vector>
#include <string>
#include <map>

#include "imi_context.h"

class IECharType {
public:
    enum ECharType {
        DONTCARE_CHAR   = 0,

        NORMAL_CHAR     = 1,
        PINYIN_CHAR     = 2 + NORMAL_CHAR,
        ASCII_CHAR      = ((PINYIN_CHAR - NORMAL_CHAR) << 1) + NORMAL_CHAR,
        SYMBOL_CHAR     = ((ASCII_CHAR - NORMAL_CHAR) << 1) + NORMAL_CHAR,
        PUNC_CHAR       = ((SYMBOL_CHAR - NORMAL_CHAR) << 1) + NORMAL_CHAR,
        HANZI_CHAR      = ((PUNC_CHAR - NORMAL_CHAR) << 1) + NORMAL_CHAR,

        BOUNDARY        = 256,

        CONVERTED       = 0x10000,
        USER_CHOICE     = (CONVERTED << 1),
        ILLEGAL         = (USER_CHOICE << 1),
        SELECTION       = (ILLEGAL << 1)
    };
};

/** The interface for retrieve Preedit data*/
class IPreeditString : public virtual IECharType {
public:
    typedef std::vector<int>    CCharTypeVec;

public:
    virtual ~IPreeditString() = 0;

    /*@{*/
    /** Following functions are used for CIMIWinHandler to retrieve Preedit data. */
    virtual int                 size() const = 0;
    virtual const TWCHAR*       string() const = 0;
    virtual int                 charTypeSize() const = 0;
    virtual int                 charTypeAt(int idx) const = 0;
    virtual int                 caret() const = 0;
    virtual int                 candi_start() const = 0;
    /*@}*/

public:
    /*@{*/
    /**
     * Following functions are used for CIMIView to set Preedit data, should not
     * be used by CIMIWinHandler.
     */
    virtual void                clear() = 0;
    virtual wstring&            getString() = 0;
    virtual CCharTypeVec&       getCharTypeVec() = 0;
    virtual void                setCaret(int caret) = 0;
    virtual void                setCandiStart(int s) = 0;
    /*@}*/
};

class CPreEditString : virtual public IPreeditString {
public:
    CPreEditString();

    virtual ~CPreEditString();

    /*@{*/
    virtual int                 size() const;
    virtual const TWCHAR*       string() const;
    virtual int                 charTypeSize() const;
    virtual int                 charTypeAt(int idx) const;
    virtual int                 caret() const;
    virtual int                 candi_start() const;
    /*@}*/

    /*@{*/
    virtual void                clear();
    virtual wstring&            getString();
    virtual CCharTypeVec&       getCharTypeVec();
    virtual void                setCaret(int caret);
    virtual void                setCandiStart(int s);
    /*@}*/

protected:
    wstring m_wstr;
    int m_caret;
    int m_candi_start;
    CCharTypeVec m_charTypes;
};


/** The interface for retrieve Preedit candidate data*/
class ICandidateList : public virtual IECharType {
public:
    enum ECandidateType {
        NORMAL_WORD = 0,
        AS_INPUT,
        BEST_WORD,
        USER_SELECTED_WORD,
        BEST_TAIL,
        PLUGIN_TAIL,
        OTHER_BEST_TAIL
    };

    typedef IPreeditString::CCharTypeVec CCharTypeVec;
    typedef std::vector< CCharTypeVec >         CCharTypeVecs;
    typedef std::vector< wstring >              CCandiStrings;
    typedef std::vector<int>                    CCandiTypeVec;

public:
    virtual ~ICandidateList() = 0;

    /*@{*/
    /** Following functions are used for CIMIWinHandler to retrieve Candidate data. */
    virtual int                 size() const = 0;
    virtual int                 total() const = 0;
    virtual int                 first() const = 0;

    virtual int                 candiType(unsigned int item) const = 0;
    virtual int                 candiSize(unsigned int item) const = 0;
    virtual const TWCHAR*       candiString(unsigned int item) const = 0;

    virtual int                 candiCharTypeSizeAt(unsigned int item) const =
        0;
    virtual int                 candiCharTypeAt(unsigned int item,
                                                unsigned int idx) const = 0;
    /*@}*/

public:
    /*@{*/
    /**
     * Following functions are used for CIMIView to set Candidate List data, should not
     * be used by CIMIWinHandler.
     */
    virtual void                clear() = 0;
    virtual void                setTotal(int total) = 0;
    virtual void                setFirst(int first) = 0;
    virtual void                setSize(int count) = 0;

    virtual void shrinkList() = 0;
    virtual void pushBackCandidate(wstring wstr, int type,
                                   int userIdx = -1) = 0;
    virtual void insertCandidate(wstring wstr, int type, int rank,
                                 int userIdx = -1) = 0;
    virtual void insertCandidateNoDedup(wstring wstr, int type, int rank,
                                        int userIdx = -1) = 0;

    virtual CCandiStrings &     getCandiStrings() = 0;
    virtual CCandiTypeVec &     getCandiTypeVec() = 0;
    virtual CCharTypeVecs &     getCharTypeVecs() = 0;
    /*@}*/
};

class CCandidateList : virtual public ICandidateList {
public:
    CCandidateList();

    virtual ~CCandidateList();

    /*@{*/
    virtual int                 size() const;
    virtual int                 total() const;
    virtual int                 first() const;

    virtual int                 candiType(unsigned int item) const;
    virtual int                 candiSize(unsigned int item) const;
    virtual const TWCHAR*       candiString(unsigned int item) const;

    virtual int                 candiCharTypeSizeAt(unsigned int item) const;
    virtual int                 candiCharTypeAt(unsigned int item,
                                                unsigned int idx) const;
    /*@}*/

    /*@{*/
    virtual void                clear();
    virtual void                setTotal(int total);
    virtual void                setFirst(int first);
    virtual void                setSize(int size);

    virtual void shrinkList();
    virtual void pushBackCandidate(wstring wstr, int type, int userIdx = -1);
    virtual void insertCandidate(wstring wstr, int type, int rank,
                                 int userIdx = -1);
    virtual void insertCandidateNoDedup(wstring wstr, int type, int rank,
                                        int userIdx = -1);

    virtual CCandiStrings &     getCandiStrings();
    virtual CCandiTypeVec &     getCandiTypeVec();
    virtual CCharTypeVecs &     getCharTypeVecs();
    /*@}*/

    int getUserIndex(int idx) { return m_candiUserIndex[idx]; }

protected:
    int m_total;
    int m_first;
    int m_size;
    CCandiStrings m_candiStrings;
    CCandiTypeVec m_candiTypes;
    std::vector<int> m_candiUserIndex;
    CCharTypeVecs m_candiCharTypeVecs;

    std::map<wstring, int> m_candiStringsIndex;
};

#endif

// -*- indent-tabs-mode: nil -*- vim:et:ts=4

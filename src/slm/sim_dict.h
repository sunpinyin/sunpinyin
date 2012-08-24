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

#ifndef _SunAGCIM_Dict_H_
#define _SunAGCIM_Dict_H_

#include "../portability.h"

#include <map>
#include <string>

class CSIMDict {
public:
    struct TState;
    typedef const TState * PState;
    struct TState {
        TSIMWordId word_id;
        std::map<TWCHAR, TState>* follow;
        TState(TSIMWordId wid = SIM_ID_NOT_WORD) : word_id(wid), follow(NULL) { }
    };
    typedef std::map<TWCHAR, TState> Map_Type;

    CSIMDict() : m_root() {}
    ~CSIMDict() { close(); }

    bool parseText(const char* filename);
    void close(){ freeSubTree(m_root); m_root = TState(); }

    const TState* getRoot() const { return &m_root; }
    int     matchLongest(const CSIMDict::TState* root,
                         CSIMDict::PState &  result,
                         const TWCHAR* str);

    static const TState* step(const CSIMDict::TState* root, TWCHAR wch);
    void PrintOut(FILE* fp) { wstring ws; InnerPrint(fp, ws, getRoot()); }

protected:
    TState m_root;

protected:
    void freeSubTree(TState& root);
    void insertWord(const TWCHAR* wstr, TSIMWordId id);
    void InnerPrint(FILE* fp, wstring & wstr, const TState* pnode);
};

#endif

// -*- indent-tabs-mode: nil -*- vim:et:ts=4

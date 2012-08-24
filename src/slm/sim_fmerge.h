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

#ifndef _SIM_FILE_MERGE_H
#define _SIM_FILE_MERGE_H

#include "../portability.h"

#include <stdio.h>
#include <deque>
#include <vector>
#include <algorithm>

template<class unit_type>
class TUnitAndParaInfo {
public:
    typedef unit_type TUnit;
    TUnitAndParaInfo() : unit(), runOut(false) {}
public:
    TUnit unit;
    bool runOut;
};

template<class unit_type>
class file_para
{
public:
    typedef unit_type TUnit;
    typedef TUnitAndParaInfo<TUnit> UnitAndParaInfo;
    typedef std::deque<UnitAndParaInfo> TItemBuf;
    typedef typename TItemBuf::iterator TIBIterator;
    typedef typename TItemBuf::const_iterator TIBConstIterator;

    file_para(FILE* p_file, size_t start, size_t end)
        : fp(p_file), runOut(false), cur_offset(start), last_offset(end),
          buf() {}

    UnitAndParaInfo& operator*(){
        if (buf.size() == 0) {
            for (int i = 0; i < BUF_SIZE; ++i) {
                buf.push_back(UnitAndParaInfo());
                UnitAndParaInfo& e = buf.back();
                e.runOut = runOut = !(e.unit.read(fp, cur_offset, last_offset));
                if (runOut) break;
            }
        }
        return buf.front();
    }

    file_para& operator++(){
        if (buf.size() == 0)
            this->operator*();
        buf.pop_front();
        return *this;
    }

    bool operator<(file_para& another){
        const UnitAndParaInfo& me = this->operator*();
        const UnitAndParaInfo& you = *another;
        if (me.runOut) {
            if (you.runOut)
                return((char *) &me < (char *) &you);
            else
                return true;
        } else {
            if (you.runOut)
                return false;
            else {
                if (me.unit > you.unit) return true;
                if (me.unit == you.unit) return((char *) &me < (char *) &you);
                return false;
            }
        }
    }

private:
    static const int BUF_SIZE = 1024;
    FILE * fp;
    bool runOut;
    size_t cur_offset, last_offset;
    TItemBuf buf;
};

template <class PPara>
class PtrCompare {
public:
    bool operator()(const PPara& p1, const PPara& p2)
    { return(*p1 < *p2); }
};

template<class unit_type>
class CMultiWayFileMerger
{
public:
    typedef file_para<unit_type> TPara;
    typedef std::vector<TPara*> TParaVec;

    void addPara(FILE *fp, size_t first_offset, size_t last_offset) {
        paras.push_back(new TPara(fp, first_offset, last_offset));
    }

    void start() {
        std::make_heap(paras.begin(), paras.end(), PtrCompare<TPara*>());
    }
    TPara* getBest(){ //You then have to deal same items form different part
        std::pop_heap(paras.begin(), paras.end(), PtrCompare<TPara*>());
        return paras.back();
    }
    void next(){
        ++(*(paras.back()));
        std::push_heap(paras.begin(), paras.end(), PtrCompare<TPara*>());
    }
    ~CMultiWayFileMerger() {
        for (size_t i = 0; i < paras.size(); i++) {
            delete paras[i];
        }
    }
private:
    TParaVec paras;
};

#endif

// -*- indent-tabs-mode: nil -*- vim:et:ts=4

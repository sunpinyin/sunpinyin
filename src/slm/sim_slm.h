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

#ifndef _SIM_SLM_H
#define _SIM_SLM_H

#include "../portability.h"

class CSlmBuilder;

/* only used as middle-result, please use thread slm for applications. */
class CSIMSlm {
    friend class CSlmBuilder;
public:
    typedef unsigned int FREQ_TYPE;
    typedef float PR_TYPE;

    struct TLeaf {
        TSIMWordId id;
        union {
            FREQ_TYPE freq;     //for builder, not for use
            PR_TYPE pr;
        };
public:
        TLeaf(TSIMWordId theId = 0, FREQ_TYPE fr = 0) : id(theId), freq(fr)
        { }
        bool operator<(const TLeaf & r) const
        { return id < r.id; }
        bool operator>(const TLeaf & r) const
        { return id > r.id; }
        bool operator==(const TLeaf & r) const
        { return id == r.id; }
    };

    struct TNode : public TLeaf {
        int child;
        PR_TYPE bow;
public:
        TNode(TSIMWordId theId = 0,
              int ch = 0,
              FREQ_TYPE fr = 0,
              PR_TYPE theBOW = 0.0)
            : TLeaf(theId, fr), child(ch), bow(theBOW)
        { }
    };

public:
    bool Load(const char* fname);
    void Free();

    unsigned isUseLogPr() const
    { return bUseLogPr; }

    double getPr(int n, TSIMWordId* hw);
    double getNegLogPr(int n, TSIMWordId* hw);

    CSIMSlm() : N(0), sz(NULL), level(NULL), bUseLogPr(0) {}

protected:
    double getPrAsLog(int n, TSIMWordId* hw);
    double getPrDirect(int n, TSIMWordId* hw);

protected:
    int N;
    int* sz;
    void** level;
    unsigned bUseLogPr;
};


template<class _NodeT_>
_NodeT_* binary_find(_NodeT_* base, int h, int t, const _NodeT_ & val){
    while (h < t) {
        int m = (h + t) / 2;
        _NodeT_* pm = base + m;
        if (*pm < val)
            h = m + 1;
        else if (*pm == val)
            return pm;
        else
            t = m;
    }
    return NULL;
}

template <class _NodeT_>
_NodeT_* binary_find_id(_NodeT_ *ph, _NodeT_* pt, TSIMWordId id){
    int h = 0, t = pt - ph;
    while (h < t) {
        int m = (h + t) / 2;
        _NodeT_ * pm = ph + m;
        if (pm->id == id)
            return pm;
        else if (pm->id < id)
            h = m + 1;
        else
            t = m;
    }
    return NULL;
}

#endif


// -*- indent-tabs-mode: nil -*- vim:et:ts=4

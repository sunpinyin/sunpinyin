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

#include <stdio.h>
#include <math.h>

#include "sim_slm.h"

bool CSIMSlm::Load(const char* fname)
{
    bool suc = false;
    FILE* fp = fopen(fname, "rb");
    if (fp != NULL && fread(&N, sizeof(N), 1, fp)==1 &&
            fread(&bUseLogPr, sizeof(bUseLogPr), 1, fp)==1) {
        sz = new int[N+1];
        level = new void* [N+1];
        fread(sz, sizeof(int), N+1, fp);
        for (int lvl=0; lvl <= N; ++lvl) {
            if (lvl < N) {
                level[lvl] = new TNode [sz[lvl]];
                fread(level[lvl], sizeof(TNode), sz[lvl], fp);
            } else {
                level[lvl] = new TLeaf [sz[lvl]];
                fread(level[lvl], sizeof(TLeaf), sz[lvl], fp);
            }
        }
        suc = true;
    }
    fclose(fp);
    return suc;
}

void CSIMSlm::Free()
{
    delete [] sz;
    for (int lvl=0; lvl <= N; ++lvl) {
        if (lvl == N)
            delete [] ((TLeaf*)level[lvl]);
        else
            delete [] ((TNode*)level[lvl]);
    }
    level = NULL;
    sz = NULL;
    bUseLogPr = 0;
    N = 0;
}

double CSIMSlm::getNegLogPr(int n, TSIMWordId* hw)
{
    double val = (bUseLogPr)?(getPrAsLog(n, hw)):(getPrDirect(n, hw));
    return (bUseLogPr)?(val):(-log(val));
}

double CSIMSlm::getPr(int n, TSIMWordId* hw)
{
    double val = (bUseLogPr)?(getPrAsLog(n, hw)):(getPrDirect(n, hw));
    return (bUseLogPr)?(exp(-val)):(val);
}

/**
* Only used when this model using -log(pr) value, also
* the return value is also -log(pr) value
*/
double CSIMSlm::getPrAsLog(int n, TSIMWordId* hw)
{
    void* pstate = ((TNode*)level[0]);
    double bow = 0.0;
    if (n > N) {
        hw += (N-n);
        n = N;
    }
    for (int lvl=0; lvl < n && pstate != NULL; ++lvl) {
        int h = ((TNode*)pstate)->child;
        int t = (((TNode*)pstate)+1)->child;
        if (lvl == n-1)
            bow = ((TNode*)pstate)->bow;
        if (lvl == N-1) {
            TLeaf* p = (TLeaf*)level[lvl+1];
            pstate = (void*)binary_find_id(p+h, p+t, hw[lvl]);
        } else {
            TNode* p = (TNode*)level[lvl+1];
            pstate = (void*)binary_find_id(p+h, p+t, hw[lvl]);
        }
    }
    if (pstate == NULL) {
        return bow + getPrAsLog(n-1, hw+1);
    } else {
        return ((TLeaf*)pstate)->pr;  // as we derive TNode from TLeaf
    }
}

/**
* Only used when this model using direct pr value, also
* the return value is also direct pr value.
*/
double CSIMSlm::getPrDirect(int n, TSIMWordId* hw)
{
    void* pstate = ((TNode*)level[0]);
    double bow = 1.0;
    if (n > N) {
        hw += (N-n);
        n = N;
    }
    for (int lvl=0; lvl < n && pstate != NULL; ++lvl) {
        int h = ((TNode*)pstate)->child;
        int t = (((TNode*)pstate)+1)->child;
        if (lvl == n-1)
            bow = ((TNode*)pstate)->bow;
        if (lvl == N-1) {
            TLeaf* p = (TLeaf*)level[lvl+1];
            pstate = (void*)binary_find_id(p+h, p+t, hw[lvl]);
        } else {
            TNode* p = (TNode*)level[lvl+1];
            pstate = (void*)binary_find_id(p+h, p+t, hw[lvl]);
        }
    }
    if (pstate == NULL) {
        return bow * getPrDirect(n-1, hw+1);
    } else {
        return ((TLeaf*)pstate)->pr;  // as we derive TNode from TLeaf
    }
}

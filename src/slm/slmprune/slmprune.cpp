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
#include "config.h"
#endif

#ifdef HAVE_ASSERT_H
#include <assert.h>
#endif

#include <stdio.h>
#include <math.h>

#include "../sim_slm.h"
#include <algorithm>

class TNodeInfo {
public:
    double d;
#ifndef WORDS_BIGENDIAN
    unsigned child : 1;
    unsigned idx : 31;
#else
    unsigned idx : 31;
    unsigned child : 1;
#endif

public:
    TNodeInfo(double distance=0.0, int pos=0, bool children=0) : d(distance)
    { idx = pos; child = (children==0)?0:1; }

    bool operator< (const TNodeInfo& r) const
    { return ((child ^ r.child) == 0)?(d < r.d):(child == 0); }

    bool operator==(const TNodeInfo& r) const
    { return (child == r.child && d == r.d); }
};

class CSlmPruner : public CSIMSlm {
public:
    CSlmPruner() : CSIMSlm(), cut(NULL)
    { }

    ~CSlmPruner()
    { if (cut) delete [] cut; }

    void SetCut(int* nCut);
    void SetReserve(int* nReserve);
    void Prune();
    void Write(const char* filename);

protected:
    void PruneLevel(int lvl);
    double CalcDistance(int lvl, int* idx, TSIMWordId* hw);
    void CalcBOW();

protected:
    int* cut;
    int cache_level, cache_idx; // to accelerate the pruning method
    double cache_PA, cache_PB;
};

void CSlmPruner::Prune()
{
    printf("Erasing items using Entropy distance"); fflush(stdout);
    for (int lvl=N; lvl>0; --lvl)
        PruneLevel(lvl);
    printf("\n"); fflush(stdout);
    CalcBOW();
}
void CSlmPruner::Write(const char* filename)
{
    FILE* out = fopen(filename, "wb");
    fwrite(&N, sizeof(N), 1, out);
    fwrite(&bUseLogPr, sizeof(bUseLogPr), 1, out);
    fwrite(sz, sizeof(int), N+1, out);
    for (int i=0; i<N; ++i) {
        fwrite(level[i], sizeof(TNode), sz[i], out);
    }
    fwrite(level[N], sizeof(TLeaf), sz[N], out);
    fclose(out);
}

void CSlmPruner::SetReserve(int* nReserve)
{
    cut = new int [N+1];
    cut[0] = 0;
    for (int lvl=1; lvl<=N; ++lvl) {
        cut[lvl] = sz[lvl] - 1 - nReserve[lvl];
        if (cut[lvl] < 0) cut[lvl] = 0;
    }
}

void CSlmPruner::SetCut(int* nCut)
{
    cut = new int [N+1];
    cut[0] = 0;
    for (int lvl=1; lvl<=N; ++lvl)
        cut[lvl] = nCut[lvl];
}

template <class chIterator>
int CutLevel(CSIMSlm::TNode* pfirst, CSIMSlm::TNode* plast, chIterator chfirst, chIterator chlast, bool bUseLogPr)
{
   int idxfirst, idxchk;
   chIterator chchk = chfirst;
   for (idxfirst=idxchk=0; chchk != chlast; ++chchk, ++idxchk) {
        //cut item whoese pr == 1.0; and not psuedo tail
        if (chchk->pr != ((bUseLogPr)?0.0:1.0) || (chchk+1) == chlast) {
            if (idxfirst < idxchk) *chfirst = *chchk;
            while (pfirst != plast && pfirst->child <= idxchk)
                pfirst++->child = idxfirst;
            ++idxfirst;
            ++chfirst;
        }
    }
    return idxfirst;
}

void CSlmPruner::PruneLevel(int lvl)
{
    cache_level = cache_idx = -1;

    if (cut[lvl] <= 0) {
        printf("\n  Level %d (%d items), no need to cut as your command!", lvl, sz[lvl]-1); fflush(stdout);
        return;
    }

    printf("\n  Level %d (%d items), allocating...", lvl, sz[lvl]-1); fflush(stdout);

    int n = sz[lvl] - 1; //do not count last psuedo tail
    if (cut[lvl] >= n) cut[lvl] = n-1;
    TNodeInfo* pbuf = new TNodeInfo[n];
    TSIMWordId hw[16]; // it should be lvl+1, yet some compiler do not support it
    int idx[16];       // it should be lvl+1, yet some compiler do not support it

    printf(", Calculating..."); fflush(stdout);
    for (int i=0; i <=lvl; ++i)
        idx[i] = 0;
    while (idx[lvl] < n) {
        if (lvl == N) {
            hw[lvl] = (((TLeaf*)level[lvl])+idx[lvl])->id;
        } else {
            hw[lvl] = (((TNode*)level[lvl])+idx[lvl])->id;
        }
        for (int j=lvl-1; j >= 0; --j) {
            TNode* pnode = ((TNode*)level[j])+idx[j];
            for (; (pnode+1)->child <= idx[j+1]; ++pnode, ++idx[j])
                ;
            hw[j] = pnode->id;
        }
        bool has_child = false;
        if (lvl != N) {
            TNode* pn = ((TNode*)level[lvl]) + idx[lvl];
            if ((pn+1)->child > pn->child)
                has_child = true;
        }
        pbuf[idx[lvl]].child = (has_child)?1:0;
        pbuf[idx[lvl]].idx = idx[lvl];
        if (!has_child)
            pbuf[idx[lvl]].d = CalcDistance(lvl, idx, hw);
        ++idx[lvl];
    }
    printf(", sorting...");
    std::make_heap(pbuf, pbuf+n);
    std::sort_heap(pbuf, pbuf+n);

    int k = 0;
    // because pr in model can not be 1.0, so we use this to mark a item to be prune
    for (TNodeInfo* pinfo = pbuf; k < cut[lvl] && pinfo->child == 0; ++k, ++pinfo) {
        if (lvl == N) {
            if (bUseLogPr)
                (((TLeaf*)level[lvl]) + pinfo->idx)->pr = 0.0; // -log(1.0)
            else
                (((TLeaf*)level[lvl]) + pinfo->idx)->pr = 1.0;
        } else {
            if (bUseLogPr)
                (((TNode*)level[lvl]) + pinfo->idx)->pr = 0.0; // -log(1.0)
            else
                (((TNode*)level[lvl]) + pinfo->idx)->pr = 1.0; // -log(1.0)
        }
    }
    printf("(cut %d items), build parent ptr...", k); fflush(stdout);
    if (lvl == N) {
        k = CutLevel((TNode*)level[lvl-1], ((TNode*)level[lvl-1])+sz[lvl-1], (TLeaf*)level[lvl], ((TLeaf*)level[lvl])+sz[lvl], bUseLogPr);
    } else {
        k = CutLevel((TNode*)level[lvl-1], ((TNode*)level[lvl-1])+sz[lvl-1], (TNode*)level[lvl], ((TNode*)level[lvl])+sz[lvl], bUseLogPr);
    }
    sz[lvl] = k; //k is new size
    printf("done!");
    delete [] pbuf;
    cache_level = cache_idx = -1;
}

template<class chIterator>
double CalcNodeBow(CSlmPruner* pruner, int lvl, TSIMWordId words[], chIterator chh, chIterator cht, bool bUseLogPr)
{
    double sumnext = 0.0, sum=0.0;
    if (chh == cht)
        return 1.0;
    for (; chh < cht; ++chh) {
        if (bUseLogPr)
            sumnext += exp(-double(chh->pr));
        else
            sumnext += double(chh->pr);
        words[lvl+1] = chh->id;
        sum += pruner->getPr(lvl, words+2);
    }
    assert(sumnext >= 0.0 && sumnext < 1.0);
    assert(sum >= 0.0 && sum < 1.0);
    return (1.0-sumnext)/(1.0-sum);
}

void CSlmPruner::CalcBOW()
{
    printf("\nUpdating back-off weight"); fflush(stdout);
    for (int lvl=0; lvl < N; ++lvl) {
        printf("\n    Level %d...", lvl); fflush(stdout);
        TNode* base[16]; //it should be lvl+1, yet some compiler do not support it
        int idx[16];     //it should be lvl+1, yet some compiler do not support it
        for (int i=0; i <= lvl; ++i) {
            base[i] = (TNode*)level[i];
            idx[i] = 0;
        }
        TSIMWordId words[17];   //it should be lvl+2, yet some compiler do not support it
        for (int lsz = sz[lvl]-1; idx[lvl] < lsz; ++idx[lvl]) {
            words[lvl] = base[lvl][idx[lvl]].id;
            for (int k=lvl-1; k >= 0; --k) {
                while (base[k][idx[k]+1].child <= idx[k+1])
                    ++idx[k];
                words[k] = base[k][idx[k]].id;
            }
            TNode & node = base[lvl][idx[lvl]];
            TNode & nodenext = *((&node)+1);

            double bow = 1.0;
            if (lvl == N-1) {
                TLeaf* ch = (TLeaf*)level[lvl+1];
                bow = CalcNodeBow(this, lvl, words, &(ch[node.child]), &(ch[nodenext.child]), bUseLogPr);
            } else {
                TNode* ch = (TNode*)level[lvl+1];
                bow = CalcNodeBow(this, lvl, words, &(ch[node.child]), &(ch[nodenext.child]), bUseLogPr);
            }
            if (bUseLogPr)
                node.bow = PR_TYPE(-log(bow));
            else
                node.bow = PR_TYPE(bow);
        }
    }
    printf("\n"); fflush(stdout);
}

double CSlmPruner::CalcDistance(int lvl, int* idx, TSIMWordId* hw)
{
    double PA, PB, PHW, PH_W, PH, BOW, _BOW, pr, p_r;
    TSIMWordId w = hw[lvl];

    PH=1.0;
    TNode* parent = ((TNode*)level[lvl-1])+idx[lvl-1];
    if (bUseLogPr)
        BOW = exp(-double(parent->bow));  //Fix original bug to use the BOW directly
    else
        BOW = double(parent->bow);

    for (int i=1; i < lvl; ++i)
        PH *= getPr(i, hw+1+(lvl-i));
    assert(PH <= 1.0 && PH >0.0);

    if (lvl == N) {
        if (bUseLogPr)
            PHW = exp(-((((TLeaf*)level[lvl])+idx[lvl])->pr));
        else
            PHW = ((((TLeaf*)level[lvl])+idx[lvl])->pr);
        assert(w == (((TLeaf*)level[lvl])+idx[lvl])->id);
    } else {
        if (bUseLogPr)
            PHW = exp(-((((TNode*)level[lvl])+idx[lvl])->pr));
        else
            PHW = ((((TNode*)level[lvl])+idx[lvl])->pr);
        assert(w == (((TNode*)level[lvl])+idx[lvl])->id);

    }
    PH_W = getPr(lvl-1, hw+2);
    assert(PHW > 0.0 && PHW < 1.0);
    assert(PH_W > 0.0 && PH_W < 1.0);

    if (cache_level != lvl-1 || cache_idx != idx[lvl-1]) {
        cache_level = lvl-1;
        cache_idx = idx[lvl-1];
        cache_PA = cache_PB = 1.0;
        for (int h=parent->child, t = (parent+1)->child; h<t; ++h) {
            TSIMWordId id;
            if (lvl == N) {
                if (bUseLogPr)
                    pr = exp(-((((TLeaf*)level[lvl])+h)->pr));
                else
                    pr = ((((TLeaf*)level[lvl])+h)->pr);
                id = (((TLeaf*)level[lvl])+h)->id;

            } else {
                if (bUseLogPr)
                    pr = exp(-((((TNode*)level[lvl])+h)->pr));
                else
                    pr = ((((TNode*)level[lvl])+h)->pr);
                id = (((TNode*)level[lvl])+h)->id;

            }
            assert(pr > 0.0 && pr < 1.0);
            cache_PA -= pr;

            hw[lvl] = id;
            p_r = getPr(lvl-1, hw+2);  // Fix bug from pr = getPr(lvl-1, hw+1)
            assert(p_r > 0.0 && p_r < 1.0);
            cache_PB -= p_r;
        }
        assert(cache_PA > -0.01 && cache_PB > -0.01);
        if (cache_PA < 0.00001 || cache_PB < 0.00001) {
            printf("\n precision problem on %d gram:", lvl-1);
            for (int i=1; i < lvl; ++i) printf("%d ", idx[i]);
            printf("   ");
            if (cache_PA < 0.00001) {
                printf("{1.0 - sigma p(w|h)} ==> 0.00001");
                cache_PA = 0.00001;
            }
            if (cache_PB < 0.00001) {
                printf("{1.0 - sigma p(w|h')} ==> 0.00001");
                cache_PB = 0.00001;
            }
        }
    }
    PA = cache_PA;
    PB = cache_PB;

    _BOW = (PA+PHW) / (PB+PH_W); // Fix bug from "(1.0-PA+PHW)/(1.0-PB+PH_W);"

    assert(BOW > 0.0);
    assert(_BOW > 0.0);
    assert(PA+PHW < 1.01);     // %1 error rate
    assert(PB+PH_W < 1.01);    // %1 error rate

    /* 
     * PH = P(h), PHW = P(w|h), PH_W = P(w|h'), _BOW = bow'(h) (the new bow)
     * BOW = bow(h) (the original bow), PA = sum_{w_i:C(w_i,h)=0} P(w_i|h),
     * PB = sum_{w_i:C(w_i,h)=0} P(w_i|h')
     */
    return -(PH * (PHW * (log(PH_W)+log(_BOW)-log(PHW)) + PA * (log(_BOW)-log(BOW)) ));
}

void ShowUsage(void)
{
    printf("Usage:\n");
    printf("    slmprune input_slm result_slm [R|C] num1 num2...\n");
    printf("\nDescription:\n");
    printf("\
      This program uses entropy-based method to prune the size of back-off \n\
  language model 'input_slm' to a specific size and write to 'result_slm'. \n\
  the third parameter [R|C] means the following numbers is the number for\n\
  (R)eserve or (C)ut. If (C)ut, the num[k] means how many items in level K\n\
  would be cut. If (R)eserve, num[k] means how many item would be reserved\n\
  in level k. \n\
      Note that we do not ensure that during pruning process,  exactly the\n\
  the given number of items are cut or reserved, because some items may \n\
  contains high level children, so could not be cut. \n\
      Also it's your responsiblity to give right number of arguments based\n\
  on 'input_slm'.\n\
\nSee Also:\n\
    To get information of the back-off language model, try 'slminfo'.\n\n");
}

int nCut[32];
const char* srcfilename, *tgtfilename;

int main(int argc, char* argv[])
{
    memset(nCut, 0, sizeof(nCut));
    if (argc < 5) {
        ShowUsage(); exit(100);
    }
    srcfilename = argv[1];
    tgtfilename = argv[2];
    bool bCut = (argv[3][0] == 'C' || argv[3][0] == 'c');

    CSlmPruner pruner;
    printf("Reading language model %s...", srcfilename); fflush(stdout);
    pruner.Load(srcfilename);
    printf("done!\n"); fflush(stdout);

    for (int i=4; i < argc && i < 100; ++i)
        nCut[i-3] = atoi(argv[i]);

    if (bCut)
        pruner.SetCut(nCut);
    else
        pruner.SetReserve(nCut);
    pruner.Prune();

    printf("Writing target language model %s...", tgtfilename); fflush(stdout);
    pruner.Write(tgtfilename);
    printf("done!\n\n"); fflush(stdout);

    pruner.Free();
    return 0;
}

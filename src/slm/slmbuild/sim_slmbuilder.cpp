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

#include <stdlib.h>
#include <math.h>
#include <vector>
#include <algorithm>

#include "sim_slmbuilder.h"

void
CSlmGTDiscounter::init(int n, CSlmBuilder::FREQ_TYPE *nr)
{
    if (dis != NULL)
        delete [] dis;
    dis = new double[--n];
    if (thres > n) thres = n;
    for (int freq = 1; freq < n; ++freq) {
        if (nr[freq] == 0 || nr[freq + 1] == 0)
            dis[freq] = 1.0;
        else
            dis[freq] = double(nr[freq + 1]) / nr[freq];
        printf("%lf ", dis[freq]); fflush(stdout);
    }
}

double
CSlmGTDiscounter::discount(int freq)
{
    double newfreq = freq * ((freq < thres) ? dis[freq] : hd);
    if (newfreq >= double(freq))
        newfreq = freq * hd;
    return newfreq;
}

void
CSlmAbsoluteDiscounter::init(int n, CSlmBuilder::FREQ_TYPE *nr)
{
    // normally, c should not greater than 1.0, yet when cut-off is used, it could be so.
    if (c <= 0.0) {
        c = double(nr[1]) / (nr[1] + 2.0 * nr[2]);
        printf("parameter c=%lf", c); fflush(stdout);
    } else {
        printf("Using given parameter c=%lf", c); fflush(stdout);
    }
}

double
CSlmAbsoluteDiscounter::discount(int freq)
{
    return (freq > 0) ? (freq - c) : (0.0);
}

void
CSlmLinearDiscounter::init(int n, CSlmBuilder::FREQ_TYPE *nr)
{
    if (dis <= 0.0 || dis >= 1.0) {
        dis = 1.0 - double(nr[1]) / nr[0];
        printf("parameter d=%lf", dis); fflush(stdout);
    } else {
        printf("Using given parameter d=%lf", dis); fflush(stdout);
    }
}

double
CSlmLinearDiscounter::discount(int freq)
{
    return freq * dis;
}

// n=1 for unigram, n=2 for bigram;
// level[0] is for pseudo 0 gram, ...
void
CSlmBuilder::Create(int n)
{
    assert(n != 0);
    nlevel = n;
    level = new void * [n + 1];
    for (int i = 0; i < n; ++i) {
        level[i] = new std::vector<TNode>;
        if (i) ((TNodeLevel*)level[i])->reserve(1024);
    }
    //Add leaf level
    level[n] = new std::vector<TLeaf>;
    ((TLeafLevel*)level[n])->reserve(1024);

    //Add pseudo root node
    ((TNodeLevel*)level[0])->push_back(TNode(0, 0, 0));

    //Initialize the nr[n+1][SLM_MAX_R] 2-D array
    nr = new FREQ_TYPE[n + 1][SLM_MAX_R];
    for (int lvl = 0; lvl < n + 1; ++lvl)
        for (int r = 0; r < SLM_MAX_R; ++r)
            nr[lvl][r] = 0;
}

void
CSlmBuilder::SetCut(FREQ_TYPE threshold[])
{
    if (cut != NULL)
        delete [] cut;
    cut = new FREQ_TYPE[nlevel + 1];
    for (int i = 0; i < nlevel; ++i)
        cut[i + 1] = threshold[i];
}

void
CSlmBuilder::SetDiscounter(CSlmDiscounter* dis[])
{
    if (discounter != NULL)
        delete [] discounter;
    discounter = new CSlmDiscounter* [nlevel + 1];
    for (int i = 0; i < nlevel; ++i)
        discounter[i + 1] = dis[i];
}

void
CSlmBuilder::SetBreakerIds(int nId, TSIMWordId brks[])
{
    breaker.clear();
    for (int i = 0; i < nId; ++i)
        breaker.push_back(brks[i]);
    std::make_heap(breaker.begin(), breaker.end());
    std::sort_heap(breaker.begin(), breaker.end());
}

void
CSlmBuilder::SetExcludeIds(int nId, TSIMWordId excludes[])
{
    m_excludes.clear();
    for (int i = 0; i < nId; ++i)
        m_excludes.push_back(excludes[i]);
    std::make_heap(m_excludes.begin(), m_excludes.end());
    std::sort_heap(m_excludes.begin(), m_excludes.end());
}

bool
CSlmBuilder::isBreakId(TSIMWordId id)
{
    return std::binary_search(breaker.begin(), breaker.end(), id);
}

bool
CSlmBuilder::isExcludeId(TSIMWordId id)
{
    return std::binary_search(m_excludes.begin(), m_excludes.end(), id);
}

void
CSlmBuilder::AddNGram(TSIMWordId* ngram, FREQ_TYPE fr)
{
    int ch;
    bool brk = isExcludeId(*ngram);

    for (int i = 1; i < nlevel; ++i) {
        TNodeLevel* pnl = (TNodeLevel*)(level[i]);
        if (pnl->capacity() == pnl->size()) {
            size_t newsz = 2 * pnl->capacity();
            if (pnl->capacity() > 1024 * 1024)
                newsz = pnl->capacity() + 1024 * 1024;
            pnl->reserve(newsz);
        }
    }
    TLeafLevel* pll = (TLeafLevel*)(level[nlevel]);
    if (pll->capacity() == pll->size()) {
        size_t newsz = 2 * pll->capacity();
        if (pll->capacity() > 1024 * 1024)
            newsz = pll->capacity() + 1024 * 1024;
        pll->reserve(newsz);
    }

    if (!brk)
        (*(TNodeLevel*)(level[0]))[0].freq += fr;

    bool branch = false;
    for (int i = 1; (!brk && i < nlevel); ++i) {
        std::vector<TNode> & pv = *(TNodeLevel*)(level[i - 1]);
        std::vector<TNode> & v = *(TNodeLevel*)(level[i]);
        branch = branch || (pv.back().child >= (int) v.size()) ||
                 (v.back().id != ngram[i - 1]);
        if (branch) {
            if (i == nlevel - 1)
                ch = ((TLeafLevel*)(level[i + 1]))->size();
            else
                ch = ((TNodeLevel*)(level[i + 1]))->size();
            v.push_back(TNode(ngram[i - 1], ch, fr));
        } else {
            v.back().freq += fr;
        }
        brk = (i > 1 && isBreakId(ngram[i - 1])) || isExcludeId(ngram[i]);
    }

    // Insert to the leaf level
    if (!brk) {
        if (fr > cut[nlevel]) {
            TLeafLevel& v = *(TLeafLevel*)(level[nlevel]);
            v.push_back(TLeaf(ngram[nlevel - 1], fr));
        } else {
            nr[nlevel][0] += fr;
            nr[nlevel][fr] += fr;
        }
    }
}

void
CSlmBuilder::CountNr()
{
    printf("\nCounting Nr..."); fflush(stdout);
    for (int lvl = 1; lvl < nlevel; ++lvl) {
        TNodeLevel& v = *(TNodeLevel*)(level[lvl]);
        for (TNodeIterator it = v.begin(), ite = v.end(); it != ite; ++it) {
            FREQ_TYPE freq = it->freq;
            nr[lvl][0] += freq;
            if (freq < (int) SLM_MAX_R && freq > 0)
                nr[lvl][freq] += freq;
        }
    }
    TLeafLevel& v = *(TLeafLevel*)(level[nlevel]);
    for (TLeafIterator it = v.begin(), ite = v.end(); it != ite; ++it) {
        FREQ_TYPE freq = it->freq;
        nr[nlevel][0] += freq;
        if (freq < (int) SLM_MAX_R && freq > 0)
            nr[nlevel][freq] += freq;
    }
    printf("\n"); fflush(stdout);
}

int
CSlmBuilder::CutLeafLevel(TNodeIterator pfirst,
                          TNodeIterator plast,
                          TLeafIterator chfirst,
                          TLeafIterator chlast,
                          int thred)
{
    int idxfirst, idxchk;
    TLeafIterator chchk = chfirst;
    for (idxfirst = idxchk = 0; chchk != chlast; ++chchk, ++idxchk) {
        //do not cut item whoese 1. freq > thred; 2. pseudo tail
        if ((int) chchk->freq > thred || (chchk + 1) == chlast) {
            if (idxfirst < idxchk)
                *chfirst = *chchk;
            for (; pfirst != plast && pfirst->child <= idxchk; ++pfirst)
                pfirst->child = idxfirst;
            ++idxfirst;
            ++chfirst;
        }
    }
    assert(pfirst == plast);
    return idxfirst;
}

int
CSlmBuilder::CutNodeLevel(TNodeIterator pfirst,
                          TNodeIterator plast,
                          TNodeIterator chfirst,
                          TNodeIterator chlast,
                          int thred)
{
    int idxfirst, idxchk;
    TNodeIterator chchk = chfirst;
    for (idxfirst = idxchk = 0; chchk != chlast; ++chchk, ++idxchk) {
        //do not cut item whoese 1. freq > thred; 2. pseudo tail; 3. leading children
        TNodeIterator chnext = chchk + 1;
        if ((int) chchk->freq > thred || chnext == chlast ||
            (chnext->child != chchk->child)) {
            if (idxfirst < idxchk)
                *chfirst = *chchk;
            for (; pfirst != plast && pfirst->child <= idxchk; ++pfirst)
                pfirst->child = idxfirst;
            ++idxfirst;
            ++chfirst;
        }
    }
    assert(pfirst == plast);
    return idxfirst;
}

void
CSlmBuilder::Cut()
{
    printf("\nCuting according freq..."); fflush(stdout);
    for (int lvl = nlevel; lvl > 0; --lvl) {
        printf("\n    Cut level %d with threshold %d...", lvl, cut[lvl]);
        fflush(stdout);
        TNodeLevel& parent = *(TNodeLevel*)(level[lvl - 1]);
        if (lvl == nlevel) {
            if (cut[lvl] > 0) {
                TLeafLevel& v = *(TLeafLevel*)(level[lvl]);
                int newsize = CutLeafLevel(parent.begin(),
                                           parent.end(), v.begin(),
                                           v.end(), cut[lvl]);
                v.erase(v.begin() + newsize, v.end());
            }
        } else {
            if (cut[lvl] > 0) {
                TNodeLevel& v = *(TNodeLevel*)(level[lvl]);
                int newsize = CutNodeLevel(parent.begin(),
                                           parent.end(), v.begin(),
                                           v.end(), cut[lvl]);
                v.erase(v.begin() + newsize, v.end());
            }
        }
    }
    printf("\n"); fflush(stdout);
}

void
CSlmBuilder::AppendTails()
{
    printf("\nAppending pseudo tail node for each level..."); fflush(stdout);
    for (int lvl = 0; lvl < nlevel; ++lvl) {
        int child_size = 0;
        if (lvl == nlevel - 1) {
            child_size = ((TLeafLevel*)(level[lvl + 1]))->size();
        } else {
            child_size = ((TNodeLevel*)(level[lvl + 1]))->size();
        }
        TNodeLevel& v = *(TNodeLevel*)(level[lvl]);
        v.push_back(TNode(0x00FFFFFF, child_size, 1));
    }
    //also make a pseudo tail node for the leaf level
    ((TLeafLevel*)(level[nlevel]))->push_back(TLeaf(0, 1));
    printf("\n"); fflush(stdout);
}

template<class TChildLevel>
void
DiscountOneLevel(CSlmBuilder::TNodeLevel& v,
                 TChildLevel& ch,
                 CSlmDiscounter* disc,
                 int bUseLogPr)
{
    CSlmBuilder::TNodeIterator it = v.begin();
    CSlmBuilder::TNodeIterator ite = v.begin() + (v.size() - 1);
    for (; it != ite; ++it) { //do not calc the pseudo tail item
        CSlmBuilder::TNodeIterator itnext = it + 1;
        double root_freq = it->freq;
        for (int h = it->child, t = itnext->child; h < t; ++h) {
            double pr = disc->discount(ch[h].freq) / root_freq;
            assert(pr > 0.0 && pr < 1.0);
            if (bUseLogPr) {
                ch[h].pr = CSlmBuilder::PR_TYPE(-log(pr));
            } else {
                ch[h].pr = CSlmBuilder::PR_TYPE(pr);
            }
        }
    }
}

void
CSlmBuilder::Discount()
{
    printf("\nDiscounting...");
    for (int lvl = nlevel; lvl > 0; --lvl) {
        printf("\n    Initializing level %d's %s discount method: ",
               lvl,
               discounter[lvl]->getName());
        discounter[lvl]->init(SLM_MAX_R, nr[lvl]);
    }
    printf("\n");
    for (int lvl = nlevel - 1; lvl >= 0; --lvl) {
        printf("\n    Discounting level %d ...", lvl + 1); fflush(stdout);
        TNodeLevel& v = *(TNodeLevel*)(level[lvl]);
        if (lvl == nlevel - 1) { //its child is leaf
            TLeafLevel& ch = *(TLeafLevel*)(level[lvl + 1]);
            DiscountOneLevel(v, ch, discounter[lvl + 1], bUseLogPr);
        } else {
            TNodeLevel& ch = *(TNodeLevel*)(level[lvl + 1]);
            DiscountOneLevel(v, ch, discounter[lvl + 1], bUseLogPr);
        }
    }
    printf("\n    Giving pseudo root level 0 a distribution...");
    //make the pseudo 0-gram a equal distribution
    TNodeLevel& v0 = *(TNodeLevel*)(level[0]);
    if (bUseLogPr) {
        v0[0].pr = PR_TYPE(-log(double(1.0) / m_nWord));
    } else {
        v0[0].pr = PR_TYPE(double(1.0) / m_nWord);
    }
    printf("\n"); fflush(stdout);
}

template<class chIterator>
double
CalcNodeBow(CSlmBuilder* builder,
            int lvl,
            TSIMWordId words[],
            chIterator chh,
            chIterator cht,
            int bUseLogPr)
{
    if (chh == cht) return 1.0;
    double sumnext = 0.0, sum = 0.0;
    for (; chh < cht; ++chh) {
        if (bUseLogPr) {
            sumnext += exp(-(chh->pr));
        } else {
            sumnext += double(chh->pr);
        }
        words[lvl + 1] = chh->id;
        sum += builder->getPr(lvl, words + 2);
    }
    assert(sumnext > 0.0 && sumnext < 1.05);
    assert(sum < 1.05 && sum > 0.0);
    //消除计算误差的影响
    if (sumnext >= 1.0 || sum >= 1.0) {
        double bow = ((sumnext > sum) ? sumnext : sum) + 0.0001;
        bow = (bow - sumnext) / (bow - sum);
        printf(
            "\n (sigma(p(w|h)=%lf, sigma(p(w|h')=%lf) bow ==> %lf due to Calculation precision for %d-gram:",
            sumnext,
            sum,
            bow,
            lvl);
        for (int i = 1; i <= lvl; ++i)
            printf("%d ", words[i]);
        return bow;
    }
    return (1.0 - sumnext) / (1.0 - sum);
}

void
CSlmBuilder::CalcBOW()
{
    printf("\nCalculating Back-Off Weight...");
    for (int lvl = 0; lvl < nlevel; ++lvl) {
        printf("\n    Processing level %d ", lvl); fflush(stdout);
        TNode* base[16]; //it should be lvl+1, yet some compiler does not support it
        int idx[16];     //it should be lvl+1, yet some compiler does not support it
        for (int i = 0; i <= lvl; ++i) {
            base[i] = &((*(TNodeLevel*)level[i])[0]);
            idx[i] = 0;
        }
        TSIMWordId words[17];  //it should be lvl+2, yet some compiler do not support it
        int sz = ((TNodeLevel*)(level[lvl]))->size() - 1;
        printf("(%d items)...", sz + 1); fflush(stdout);
        for (; idx[lvl] < sz; ++idx[lvl]) {
            words[lvl] = base[lvl][idx[lvl]].id;
            for (int k = lvl - 1; k >= 0; --k) {
                while (base[k][idx[k] + 1].child <= idx[k + 1])
                    ++idx[k];
                words[k] = base[k][idx[k]].id;
            }
            TNode & node = base[lvl][idx[lvl]];
            TNode & nodenext = *((&node) + 1);
            double bow;
            if (lvl == nlevel - 1) {
                TLeaf * ch = &((*(TLeafLevel*)level[lvl + 1])[0]);
                bow = CalcNodeBow(this,
                                  lvl,
                                  words,
                                  ch + node.child,
                                  ch + nodenext.child,
                                  bUseLogPr);
            } else {
                TNode * ch = &((*(TNodeLevel*)level[lvl + 1])[0]);
                bow = CalcNodeBow(this,
                                  lvl,
                                  words,
                                  ch + node.child,
                                  ch + nodenext.child,
                                  bUseLogPr);
            }
            if (bUseLogPr) {
                node.bow = PR_TYPE(-log(bow));
            } else {
                node.bow = PR_TYPE(bow);
            }
        }
    }
    printf("\n"); fflush(stdout);
}

double
CSlmBuilder::getPr(int n, TSIMWordId *words)
{
    int lvl;
    double bow = 1.0;
    void* pnode = &((*(TNodeLevel*)level[0])[0]);

    assert(n <= nlevel);

    if (n == 0) {
        if (bUseLogPr) {
            return exp(-((TNode*)pnode)->pr);
        } else {
            return ((TNode*)pnode)->pr;
        }
    }

    for (lvl = 0; pnode != NULL && lvl < n; ++lvl) {
        if (bUseLogPr) {
            bow = exp(-((TNode*)pnode)->bow);
        } else {
            bow = ((TNode*)pnode)->bow;
        }
        pnode = FindChild(lvl, (TNode*)pnode, words[lvl]);
    }

    if (pnode != NULL) { // find the whole string
        if (bUseLogPr) {
            return exp(-((TLeaf*)pnode)->pr);
        } else {
            return ((TLeaf*)pnode)->pr;
        }
    } else if (lvl == n - 1) { // only find the history
        return bow * getPr(n - 1, words + 1);
    } else { //even not find the history
        return getPr(n - 1, words + 1);
    }
}

void*
CSlmBuilder::FindChild(int lvl, TNode* root, TSIMWordId id)
{
    int chh = root->child, cht = (root + 1)->child;
    if (lvl == nlevel - 1) {
        TLeaf* pleaf = &((*(TLeafLevel*)level[lvl + 1])[0]);
        return (void*)binary_find(pleaf, chh, cht, TLeaf(id));
    } else {
        TNode* pnode = &((*(TNodeLevel*)level[lvl + 1])[0]);
        return (void*)binary_find(pnode, chh, cht, TNode(id));
    }
}

void
CSlmBuilder::Build()
{
    CountNr();
    AppendTails();
    Cut();
    Discount();
    CalcBOW();
}

void
CSlmBuilder::Write(FILE *out)
{
    fwrite(&nlevel, sizeof(nlevel), 1, out);
    fwrite(&bUseLogPr, sizeof(bUseLogPr), 1, out);
    for (int lvl = 0; lvl <= nlevel; ++lvl) {
        int sz = 0;
        if (lvl == nlevel)
            sz = ((TLeafLevel*)(level[lvl]))->size();
        else
            sz = ((TNodeLevel*)(level[lvl]))->size();
        fwrite(&sz, sizeof(sz), 1, out);
    }
    for (int lvl = 0; lvl < nlevel; ++lvl) {
        TNodeLevel& v = *(TNodeLevel*)(level[lvl]);
        for (TNodeIterator it = v.begin(), ite = v.end(); it != ite; ++it)
            fwrite(&(*it), sizeof(TNode), 1, out);
    }
    TLeafLevel& v = *(TLeafLevel*)(level[nlevel]);
    for (TLeafIterator it = v.begin(), ite = v.end(); it != ite; ++it)
        fwrite(&(*it), sizeof(TLeaf), 1, out);
}

void
CSlmBuilder::Close(void)
{
    if (level != NULL) {
        for (int lvl = 0; lvl <= nlevel; ++lvl) {
            if (lvl == nlevel)
                delete (TLeafLevel*)(level[lvl]);
            else
                delete (TNodeLevel*)(level[lvl]);
        }
        delete [] level;
        level = NULL;
    }
    if (cut != NULL) {
        delete [] cut;
        cut = NULL;
    }
    if (discounter != NULL) {
        for (int lvl = 1; lvl <= nlevel; ++lvl) {
            delete discounter[lvl];
        }
        delete [] discounter;
        discounter = NULL;
    }
    if (nr != NULL) {
        delete [] nr;
        nr = NULL;
    }
    breaker.clear();
    m_nWord = 0;
    nlevel = 0;
}

// -*- indent-tabs-mode: nil -*- vim:et:ts=4

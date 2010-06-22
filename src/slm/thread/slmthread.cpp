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
#include <unistd.h>
#include <stdlib.h>

#include <vector>
#include <map>
#include <math.h>

#include "../sim_slm.h"
#include "../slm.h"

#include "ValueCompress.h"

class CSIMSlmWithIteration : public CSIMSlm{
public:
    struct TLevelIterator {
        std::vector<int>    m_history;
    };

public:
    int
    getLevelSize(int lvl) { return sz[lvl]; }

    int
    getN() { return N; }

    void
    getIdString(TLevelIterator& it, std::vector<TSIMWordId>& history);

    void
    beginLevelIteration(int lvl, TLevelIterator& it);

    void
    next(TLevelIterator& it);

    bool
    isEnd(TLevelIterator& it);

    TLeaf*
    getNodePtr(TLevelIterator& it);

    int
    findState(int n, TSIMWordId*hw);

    void
    findBackOffState(int n, TSIMWordId*hw, unsigned & bol, unsigned& bon);


protected:
    void
    adjustIterator(TLevelIterator& it);
};

int
CSIMSlmWithIteration::findState(int n, TSIMWordId*hw)
{
    if (n == 0) return 0;

    int m = -1;
        while (n > N) { --n; ++hw; }

        void* pstate = ((TNode*)level[0]);
        for (int lvl=0; lvl < n && pstate != NULL; ++lvl) {
            int h = ((TNode*)pstate)->child;
            int t = (((TNode*)pstate)+1)->child;
            if (lvl == N-1) {
                TLeaf* p = (TLeaf*)level[lvl+1];
                pstate = (void*)binary_find_id(p+h, p+t, hw[lvl]);
                m = (pstate != NULL)?(((TLeaf*)pstate) - p):(-1);
            } else {
                TNode* p = (TNode*)level[lvl+1];
                pstate = (void*)binary_find_id(p+h, p+t, hw[lvl]);
                m = (pstate != NULL)?(((TNode*)pstate) - p):(-1);
            }
        }
    return m;
}

void
CSIMSlmWithIteration::findBackOffState(int n, TSIMWordId*hw, unsigned & bol, unsigned& bon)
{
    while (n > 1) {
        --n; ++hw;
        int idx = findState(n, hw);
        if (idx >= 0 && ((TNode*)(level[n]))[idx].child < ((TNode*)(level[n]))[idx+1].child) {
            bol = n; bon = idx; return;
        }
    }
    bol = bon = 0;
    return;
}

void
CSIMSlmWithIteration::getIdString(TLevelIterator& it, std::vector<TSIMWordId>& history)
{
    history.clear();
    for (int i=1, tmp_sz=it.m_history.size(); i < tmp_sz; ++i) {
        int idx = it.m_history[i];
        if (i == N)
            history.push_back(((TLeaf*)(level[i]))[idx].id);
        else
            history.push_back(((TNode*)(level[i]))[idx].id);
    }
}

void
CSIMSlmWithIteration::beginLevelIteration(int lvl, TLevelIterator& it)
{
    it.m_history.clear();
    for (int i=0, tmp_sz=lvl; i <= tmp_sz; ++i)
        it.m_history.push_back(0);
    adjustIterator(it);
}

void
CSIMSlmWithIteration::next(TLevelIterator& it)
{
    ++(it.m_history.back());
    adjustIterator(it);
}

bool
CSIMSlmWithIteration::isEnd(TLevelIterator& it)
{
    return ((it.m_history.back()+1 >= sz[it.m_history.size()-1]));
}

void
CSIMSlmWithIteration::adjustIterator(TLevelIterator& it)
{
    int ch = it.m_history.back();
    for (int i= it.m_history.size()-2; i >= 0; --i) {
        int len = sz[i];
        int& parent = it.m_history[i];
        TNode* pn = (TNode*)(level[i]);
        while (parent < len && pn[parent+1].child <= ch)
            ++parent;
        ch = parent;
    }
}

CSIMSlm::TLeaf*
CSIMSlmWithIteration::getNodePtr(TLevelIterator& it)
{
    int lvl = it.m_history.size()-1;
    int idx = it.m_history.back();
    if (lvl == N)
        return (((TLeaf*)(level[lvl]))+idx);
    else
        return (((TNode*)(level[lvl]))+idx);
}



void ShowUsage()
{
    printf("Usage:\n");
    printf("    slmthread primitive_slm threaded_slm\n");
    printf("\nDescription:\n");
    printf("    slmthread add back-off-state for each slm node in the primitive_slm. ");
    printf("Also it compresses 32-bit float into 16 bit representation.\n\n");
    exit(100);
}

FILE* fp = NULL;
CThreadSlm::TNode* levels[16];
CThreadSlm::TLeaf* lastLevel;

int main(int argc, char* argv[])
{
    CValueCompressor vc;
    unsigned int bol, bon;
    CSIMSlmWithIteration slm;
    std::vector<TSIMWordId> history;
    float real_pr, eff_pr, real_bow, eff_bow;

    std::map<float, float> pr_eff, bow_eff;     // effval --> val
    std::map<float, int> pr_values, bow_values; // effval --> freq
    std::map<float, int> pr_map, bow_map;       // result: val --> int
    std::vector<float>   pr_table, bow_table;   // result: val vector
    std::vector<float>::iterator itt, itte;

    if (argc != 3)
        ShowUsage();

    printf("Loading original slm..."); fflush(stdout);
    if (slm.Load(argv[1]) == false)
        ShowUsage();

    bool usingLogPr = slm.isUseLogPr();

    #define EffectivePr(a)  (float((usingLogPr)?((a)/log(2.0)):(-log2((a)))))
    #define OriginalPr(b)   (float((usingLogPr)?((b)*log(2.0)):(exp2(-(b)))))
    #define EffectiveBow(a) (float((usingLogPr)?(exp(-(a))):((a))))
    #define OriginalBow(b)  (float((usingLogPr)?(-log((b))):((b))))

    printf("\nfirst pass..."); fflush(stdout);
    for (int lvl=0; lvl <= slm.getN(); ++lvl) {
        CSIMSlmWithIteration::TLevelIterator it;
        slm.beginLevelIteration(lvl, it);
        for (; !slm.isEnd(it); slm.next(it)) {
            CSIMSlm::TLeaf* pl = slm.getNodePtr(it);
            real_pr = pl->pr;
            eff_pr = EffectivePr(real_pr);
            if (pr_eff.find(eff_pr) == pr_eff.end()) {
                pr_eff[eff_pr] = real_pr;
            } else { // precision error cause non 1:1 mapping
                pr_eff[eff_pr] = OriginalPr(eff_pr);
            }
            ++(pr_values[eff_pr]);
            if (lvl < slm.getN()) {
                real_bow = ((CSIMSlm::TNode*)pl)->bow;
                eff_bow = EffectiveBow(real_bow);
                if (bow_eff.find(eff_bow) == bow_eff.end()) {
                    bow_eff[eff_bow] = real_bow;
                } else { // two values map to same distance value due to precision error
                    bow_eff[eff_bow] = OriginalBow(eff_bow);
                }
                ++(bow_values[eff_bow]);
            }
        }
    }

    // Following pr value should not be grouped, or as milestone values.
    static float msprs[] = {
        0.9, 0.8, 0.7, 0.6,
        1.0/2, 1.0/4, 1.0/8, 1.0/16, 1.0/32, 1.0/64, 1.0/128,
        1.0/256, 1.0/512, 1.0/1024, 1.0/2048, 1.0/4096, 1.0/8192,
        1.0/16384, 1.0/32768, 1.0/65536
    };

    for (unsigned i=0, sz=sizeof(msprs)/sizeof(float); i < sz; ++i) {
        float real_pr = (usingLogPr)?(-log(msprs[i])):(msprs[i]);
        float eff_pr = EffectivePr(real_pr);
        if (pr_eff.find(eff_pr) == pr_eff.end()) {
            pr_eff[eff_pr] = real_pr;
        } else { // precision error cause non 1:1 mapping
            pr_eff[eff_pr] = OriginalPr(eff_pr);
        }
        pr_values[eff_pr] = 0;
    }

    // Following bow value should not be grouped, or as milestone values.
    static float msbows[] = {
        1.0, 0.9, 0.8, 0.7, 0.6, 0.5, 0.4, 0.3, 0.2,
        0.1, 0.05, 0.01, 0.005, 0.001, 0.0005, 0.0001,
        0.00005, 0.00001, 0.000005, 0.000001, 0.0000005, 0.0000001
    };

    for (unsigned i=0, sz=sizeof(msbows)/sizeof(float); i < sz; ++i) {
        float real_bow = (usingLogPr)?(-log(msbows[i])):(msbows[i]);
        float eff_bow = EffectiveBow(real_bow);
        if (bow_eff.find(eff_bow) == bow_eff.end()) {
            bow_eff[eff_bow] = real_bow;
        } else { // two values map to same distance value due to precision error
            bow_eff[eff_bow] = OriginalBow(eff_bow);
        }
        bow_values[eff_bow] = 0;
    }

    printf("\nCompressing pr values..."); fflush(stdout);
    vc(pr_eff, pr_values, pr_map, pr_table, (1 << CThreadSlm::BITS_PR));
    pr_values.clear();
    itte = pr_table.end();
    for (itt = pr_table.begin(); itt != itte; ++itt) {
        *itt = OriginalPr(*itt);
        assert(usingLogPr || (*itt > 0.0 && *itt < 1.0));
        assert(!usingLogPr || *itt > 0.0);
    }
    printf("%lu float values ==> %lu values", pr_eff.size(), pr_table.size());

    printf("\nCompressing bow values..."); fflush(stdout);
    vc(bow_eff, bow_values, bow_map, bow_table, (1 << CThreadSlm::BITS_BOW));
    bow_values.clear();
    itte = bow_table.end();
    for (itt = bow_table.begin(); itt != itte; ++itt)
        *itt = OriginalBow(*itt);
    printf("%lu float values ==> %lu values", bow_eff.size(), bow_table.size());


    printf("\nThreading the new model..."); fflush(stdout);
    for (int lvl=0; lvl < slm.getN(); ++lvl) {
        levels[lvl] = new CThreadSlm::TNode[slm.getLevelSize(lvl)];

        CSIMSlmWithIteration::TLevelIterator it;
        slm.beginLevelIteration(lvl, it);
        for (; !slm.isEnd(it); slm.next(it)) {
            slm.getIdString(it, history);
            if (history.size() == 0) {
                slm.findBackOffState(lvl, NULL, bol, bon);
            } else {
                slm.findBackOffState(lvl, &history[0], bol, bon);
            }

            CSIMSlm::TNode* pn = (CSIMSlm::TNode*)slm.getNodePtr(it);
            CThreadSlm::TNode& nn = levels[lvl][it.m_history.back()];

            std::map<float, int>::iterator prit = pr_map.find(pn->pr);
            if (prit == pr_map.end()) { // This would be cause by precision error
                double val = EffectivePr(pn->pr);
                val = OriginalPr(val);
                prit = pr_map.find(val);
                assert(prit != pr_map.end());
            }
            int idx_pr = prit->second;
            nn.set_pr(idx_pr);

            nn.set_wid(pn->id);
            nn.set_bon(bon);
            nn.set_bol(bol);

            std::map<float, int>::iterator bowit = bow_map.find(pn->bow);
            if (bowit == bow_map.end()) { // precision error
                double val = EffectiveBow(pn->bow);
                val = OriginalBow(val);
                bowit = bow_map.find(val);
                assert(bowit != bow_map.end());
            }
            int idx_bow = bowit->second;
            nn.set_bow(idx_bow);

            nn.set_ch(pn->child);

            assert(usingLogPr || (pr_table[idx_pr] > 0.0 && pr_table[idx_pr] < 1.0));
            assert(!usingLogPr || pr_table[idx_pr] > 0.0);
        }
        CSIMSlm::TNode* pn = (CSIMSlm::TNode*)slm.getNodePtr(it);
        CThreadSlm::TNode& nn = levels[lvl][it.m_history.back()];
        nn.set_ch(pn->child);
    };


    lastLevel = new CThreadSlm::TLeaf [slm.getLevelSize(slm.getN())];
    CSIMSlmWithIteration::TLevelIterator it;
    slm.beginLevelIteration(slm.getN(), it);
    for (int lvl=slm.getN(); !slm.isEnd(it); slm.next(it)) {
        CSIMSlm::TLeaf* pn = slm.getNodePtr(it);
        slm.getIdString(it, history);
        slm.findBackOffState(lvl, &history[0], bol, bon);

        CThreadSlm::TLeaf& nn = lastLevel[it.m_history.back()];

        std::map<float, int>::iterator prit = pr_map.find(pn->pr);
        if (prit == pr_map.end()) { // This would be cause by precision error
            double val = EffectivePr(pn->pr);
            val = OriginalPr(val);
            prit = pr_map.find(val);
            assert(prit != pr_map.end());
        }
        int idx_pr = prit->second;
        nn.set_pr(idx_pr);

        nn.set_wid(pn->id);
        nn.set_bon(bon);
        nn.set_bol(bol);
    }


    printf("\nWriting out..."); fflush(stdout);

    float dummy = 0.0;
    fp = fopen(argv[2], "wb");
    int N = slm.getN();
    fwrite(&N, sizeof(int), 1, fp);
    unsigned ulp = slm.isUseLogPr();
    fwrite(&ulp, sizeof(unsigned), 1, fp);

    for (int lvl = 0; lvl <= N; ++lvl) {
        int len = slm.getLevelSize(lvl);
        fwrite(&len, sizeof(int), 1, fp);
    }
    fwrite(&pr_table[0], sizeof(float), pr_table.size(), fp);
    for (int i = pr_table.size(), sz=(1 << CThreadSlm::BITS_PR); i < sz; ++i)
        fwrite(&dummy, sizeof(float), 1, fp);

    fwrite(&bow_table[0], sizeof(float), bow_table.size(), fp);
    for (int i = bow_table.size(), sz=(1 << CThreadSlm::BITS_BOW); i < sz; ++i)
        fwrite(&dummy, sizeof(float), 1, fp);

    for (int lvl=0; lvl < N; ++lvl)
        fwrite(levels[lvl], sizeof(CThreadSlm::TNode), slm.getLevelSize(lvl), fp);
    fwrite(lastLevel, sizeof(CThreadSlm::TLeaf), slm.getLevelSize(N), fp);
    fclose(fp);

    printf("done!\n"); fflush(stdout);

    delete [] lastLevel;
    for (int lvl=0; lvl < N; ++lvl)
        delete []levels[lvl];

    bow_values.clear();
    bow_map.clear();
    bow_table.clear();

    pr_values.clear();
    pr_map.clear();
    pr_table.clear();

    slm.Free();


    return 0;
}

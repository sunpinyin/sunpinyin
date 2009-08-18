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

#ifndef _SIM_SLM_BUILDER_H
#define _SIM_SLM_BUILDER_H

#include "../portability.h"

#include "sim_slm.h"

class CSlmDiscounter;

class CSlmBuilder {
public:
    static const int SLM_MAX_R=16;
    typedef CSIMSlm::FREQ_TYPE FREQ_TYPE;
    typedef CSIMSlm::PR_TYPE PR_TYPE;
    typedef CSIMSlm::TNode TNode;
    typedef CSIMSlm::TLeaf TLeaf;

public:
    CSlmBuilder()
        : m_nWord(0), nlevel(0), level(NULL), cut(NULL), discounter(NULL),
          nr(NULL), breaker(), m_excludes(), bUseLogPr(0) { }
    ~CSlmBuilder()
        { Close(); }

    void Create(int n);
    void SetNumberOfWord(int nWord) { this->m_nWord = nWord; }
    void SetCut(FREQ_TYPE threshold[]);
    void SetDiscounter(CSlmDiscounter* dis[]);
    void SetBreakerIds(int nId, TSIMWordId brks[]);
    void SetExcludeIds(int nId, TSIMWordId excludes[]);
    void SetUseLogPr(int bUse)
         { bUseLogPr = bUse; }

    void AddNGram(TSIMWordId* ngram, FREQ_TYPE fr);
    void Build();
    void Write(FILE* out);
    void Close();

    //get pr(w[n-1] | w[0]...w[n-2]) on constructed partial model (low levels)
    double getPr(int n, TSIMWordId* w);

public:
    typedef std::vector<TNode> TNodeLevel;
    typedef std::vector<TLeaf> TLeafLevel;
    typedef TNodeLevel::iterator TNodeIterator;
    typedef TLeafLevel::iterator TLeafIterator;

protected:
    bool isBreakId(TSIMWordId id);
    bool isExcludeId(TSIMWordId id);
    void CountNr();
    void AppendTails();
    void Cut();
    void Discount();
    void CalcBOW();
    void*FindChild(int lvl, TNode* root, TSIMWordId id);
    int  CutNodeLevel(TNodeIterator pfirst, TNodeIterator plast,
                      TNodeIterator chfirst, TNodeIterator chlast, int thred);
    int  CutLeafLevel(TNodeIterator pfirst, TNodeIterator plast,
                      TLeafIterator chfirst, TLeafIterator chlast, int thred);

private:
    int    nlevel, bUseLogPr;
    void** level;
    //level[0] is psudeo root level, level[1] is unigram level, ..., all are vector type

    int m_nWord;
    FREQ_TYPE* cut; // cut[1] is not cut threshold for 1-gram, ...
    CSlmDiscounter** discounter; // discounter[1] is for 1-gram...
    FREQ_TYPE (*nr)[SLM_MAX_R];//nr[1][SLM_MAX_R] is for 1-gram...
    std::vector<TSIMWordId> breaker;
    std::vector<TSIMWordId> m_excludes;
};

class CSlmDiscounter {
public:
    // n is array size, nr is FREQ_TYPE[n], nr[0] is corpuse size,or sigma r*nr;
    // nr[1] is number of ngram items with freq 1, ...
    virtual void init(int n, CSlmBuilder::FREQ_TYPE *nr) = 0;

    // freq is the ngram frequence, not the conditional pr
    virtual double discount(int freq) = 0;
    virtual const char* getName() = 0;
};

//Good-Turing discount
class CSlmGTDiscounter : public CSlmDiscounter {
public:
    CSlmGTDiscounter(int threshold=10, double highfreq_discount=0.95) : thres(threshold), hd(highfreq_discount), dis(NULL) {}
    virtual void init(int n, CSlmBuilder::FREQ_TYPE *nr);
    virtual double discount(int freq);
    virtual const char* getName()
        { return "Good-Turing"; }
protected:
    int thres;
    double hd;
    double *dis;
};

class CSlmAbsoluteDiscounter : public CSlmDiscounter {
public:
    CSlmAbsoluteDiscounter(double substract = 0.0) : c(substract) {}
    //c == 0 mean this value should be count according to r[]
    virtual void init(int n, CSlmBuilder::FREQ_TYPE *nr);
    virtual double discount(int freq);	// return freq - c
    virtual const char* getName()
        { return "Absolution"; }
protected:
        double c;
};

class CSlmLinearDiscounter : public CSlmDiscounter {
public:
    CSlmLinearDiscounter(double shrink = 0.0) : dis(shrink) {}
    //dis == 0 mean this value should be count according to r[]
    virtual void init(int n, CSlmBuilder::FREQ_TYPE *nr);
    virtual double discount(int freq);	// return freq * dis
    virtual const char* getName()
        { return "Linear"; }
protected:
    double dis;
};

#endif

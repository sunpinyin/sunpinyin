/*
 * Copyright (c) 2009 Kov Chai <tchaikov@gmail.com>
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
#ifndef _ARPA_SLM_H
#define _ARPA_SLM_H

#include <istream>
#include "common.h"

using std::istream;

#define N_GRAM (3)


/* the ARPA style representation of sunpinyin's SLM */
class CArpaSlm {
public:
    struct TLeaf {
        TSIMWordId hw[N_GRAM];
        TSIMWordId wid;
        float pr;
        unsigned ch;
        unsigned bon;
        unsigned bol;
        void load(istream&, const TLexicon&);
        int load_words(char* buf, const TLexicon& lexicon);
        TLeaf() : wid(0), pr(.0), ch(0), bon(0), bol(0) {}
    };

    struct TNode : public TLeaf {
        float bow;
        void load(istream&, const TLexicon&);
        void load_level0(istream&);
    };
  
    typedef std::vector<TNode> TNodeLevel;
    typedef std::vector<TLeaf> TLeafLevel;

private:
    TNodeLevel m_levels[N_GRAM+1]; /* [0..N_GRAM] */
    TLeafLevel m_lastLevel;
    const bool m_usingLogPr;
    const unsigned m_N;
  
public:
    /* XXX, ARPA file does not provide these information.
       so we assume this SLM is trigram, and does not use LogPr */
    CArpaSlm() : m_usingLogPr(false), m_N(N_GRAM) {}
    bool good() const { return m_levels[0].size() != 0; }
    unsigned getN() const { return m_N; }
    bool usingLogPr() const { return m_usingLogPr; }
    const TNodeLevel& getLevel(unsigned lvl) const { return m_levels[lvl]; }
    const TLeafLevel& getLastLevel() const { return m_lastLevel; }
    unsigned getLevelSize(unsigned lvl) const {
        assert(lvl <= m_N);
        if (lvl < m_N) {
            return m_levels[lvl].size();
        } else {
            return m_lastLevel.size();
        }
    }
    /**
     * initialize the `ch' and `wid' fields of each node in levels
     */
    void threading();
    void load(const char* filename, const TLexicon& lexicon);
  
private:
    /**
     * find out the first child of a given node in its next level
     * @param lvl the level where node belongs to
     * @param node the node
     * @param last_child the child index of previous node
     * @return the index of the found child
     */
    unsigned find_1st_child(unsigned lvl, const TNode& node, int last_child);
};

#endif//_ARPA_SLM_H

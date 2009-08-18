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
#include <algorithm>
#include "common.h"
#include "arpa_slm.h"
#include "arpa_conv.h"


// 
// convert CArpaSlm::TLeaf to CThreadSlm::TLeaf
// 
class CArpaLeafConv
{
    const bool usingLogPr;
    CompressedTable& m_pr_table;
    RealIndexMap& m_pr_map;
    
public:
    CArpaLeafConv(bool usingLogPr_,
                  RealIndexMap* pr_map,
                  CompressedTable* pr_table) : 
        usingLogPr(usingLogPr_),
        m_pr_table(*pr_table),
        m_pr_map(*pr_map) {}
    
    CThreadSlm::TLeaf operator()(const CArpaSlm::TLeaf& leaf) {
        CThreadSlm::TLeaf tleaf;
        tleaf.set_wid(leaf.wid);
        tleaf.set_bon(leaf.bon);
        tleaf.set_bol(leaf.bol);
        unsigned pr_idx = get_pr_index(leaf.pr);
        tleaf.set_pr(pr_idx);
        return tleaf;
        
    }
    
    //
    // lookup the Real/Effective value in the RealIndexMap for its index
    // in the CompressedTable
    //
    unsigned get_pr_index(float pr) {
        std::map<float, int>::iterator prit = m_pr_map.find(pr);
        if (prit == m_pr_map.end()) { // This could be caused by precision error
            double val = EffectivePr(pr);
            val = OriginalPr(val);
            prit = m_pr_map.find(val);
            assert(prit != m_pr_map.end());
        }
        int idx_pr = prit->second;
        assert(usingLogPr || (m_pr_table[idx_pr] > 0.0 && m_pr_table[idx_pr] < 1.0));
        assert(!usingLogPr || m_pr_table[idx_pr] > 0.0);
        return idx_pr;
    }
};

// 
// convert CArpaSlm::TNode to CThreadSlm::TNode
// 
class CArpaNodeConv
{
    const bool usingLogPr;
    CArpaLeafConv m_leaf_conv;
    CompressedTable& m_bow_table;
    RealIndexMap& m_bow_map;
    
public:
    CArpaNodeConv(bool usingLogPr_,
                  RealIndexMap* pr_map,
                  CompressedTable* pr_table,
                  RealIndexMap* bow_map,
                  CompressedTable* bow_table) : 
        usingLogPr(usingLogPr_),
        m_leaf_conv(usingLogPr, pr_map, pr_table),
        m_bow_table(*bow_table),
        m_bow_map(*bow_map) {}
    
    CThreadSlm::TNode operator()(const CArpaSlm::TNode& node) {
        CThreadSlm::TNode tnode;
        tnode.set_wid(node.wid);
        tnode.set_bon(node.bon);
        tnode.set_bol(node.bol);
        tnode.set_ch(node.ch);
        unsigned pr_idx = m_leaf_conv.get_pr_index(node.pr);
        tnode.set_pr(pr_idx);
        unsigned bow_idx = get_bow_index(node.bow);
        tnode.set_bow(bow_idx);
        return tnode;
    }
    
    unsigned get_bow_index(float bow) {
        FreqMap::iterator bowit = m_bow_map.find(bow);
        if (bowit == m_bow_map.end()) {
            double val = EffectiveBow(bow);
            val = OriginalBow(val);
            bowit = m_bow_map.find(val);
            assert(bowit != m_bow_map.end());
        }
        return bowit->second;
    }
};

void compress(const CArpaSlm& slm, 
              CompressedTable& pr_table, RealIndexMap& pr_map,
              CompressedTable& bow_table, RealIndexMap& bow_map,
              TNodeLevels& nodeLevels, CThreadSlm::TLeaf*& leafLevel)
{
    CArpaLeafConv leaf_conv(slm.usingLogPr(), &pr_map, &pr_table);
    CArpaNodeConv node_conv(slm.usingLogPr(), &pr_map, &pr_table, &bow_map, &bow_table);
    const int N = slm.getN();
    TNodeLevels node_levels(N);
    for (int lvl = 0; lvl < N; ++lvl) {
        const CArpaSlm::TNodeLevel& level = slm.getLevel(lvl);
        unsigned len = level.size();
        node_levels[lvl] = new CThreadSlm::TNode[len+1];
        std::transform(level.begin(), level.end(), 
                       node_levels[lvl], node_conv);
        memset(&node_levels[lvl][len], 0, sizeof(CThreadSlm::TNode));
        node_levels[lvl][len].set_ch(slm.getLevelSize(lvl+1));
    }
    
    const CArpaSlm::TLeafLevel& level = slm.getLastLevel();
    unsigned len = level.size();
    CThreadSlm::TLeaf* leaf_level = new CThreadSlm::TLeaf[len+1];
    std::transform(level.begin(), level.end(),
                   leaf_level, leaf_conv);
    memset(&leaf_level[len], 0, sizeof(CThreadSlm::TLeaf));
    nodeLevels = node_levels;
    leafLevel = leaf_level;
}

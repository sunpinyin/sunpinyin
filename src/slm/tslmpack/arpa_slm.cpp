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
#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include "arpa_slm.h"

using namespace std;

/**
 * the GNU extension is not always available, so we invent another wheel.
 */
size_t getline(char *buf, size_t n, FILE* stream)
{
    char* p;
    char* end = buf+n;
    for (p = buf; p != end; ++p) {
        int c = fgetc(stream);
        if (c == '\n' || c == EOF)
            break;
        *p = c;
        --n;
    }
    if (p != end)
        *p = 0;
    else
        *(p-1) = 0;
    return p-buf;
}

char* getwords(char* buf, char** next)
{
    char* word = buf;
    char* delim = strstr(buf, "  ");
    if (delim == NULL) {
        cerr << "Unknown format in: " << buf << "." << endl;
        exit(2);
    }
    *delim = '\0';
    *next = delim+2;
    return word;
}

unsigned get_wid(const char* word, const TLexicon& lexicon)
{
    TLexicon::const_iterator lexi = lexicon.find(word);
    unsigned wid;
    if (lexi != lexicon.end()) {
        wid = lexi->second;
    } else {
        cerr << "Error:\"" << word << "\" not found in lexicon." << endl;
        wid = 0;
    }
    return wid;
}

int
CArpaSlm::TLeaf::load_words(char* buf, const TLexicon& lexicon)
{
    int nword = 0;
    char* word, *end;
    for (word = end = buf; *end != 0; ++end) {
        if (*end == ' ') {
            assert(nword < N_GRAM);
            *end = 0;
            hw[nword++] = get_wid(word, lexicon);
            word = end+1;
        }
    }
    if (buf != end) {
        wid = hw[nword++] = get_wid(word, lexicon);
    }
    return nword;
}

void 
CArpaSlm::TLeaf::load(istream& is, const TLexicon& lexicon)
{
    char buf[1024];
    is.getline(buf, sizeof(buf));
    char* next = 0;
    char* words = getwords(buf, &next);
    load_words(words, lexicon);
    sscanf(next, "%f (%1u, %u)",
           &pr, &bol, &bon);
}

void 
CArpaSlm::TNode::load(istream& is, const TLexicon& lexicon)
{
    char buf[1024];
    is.getline(buf, sizeof(buf));
    char* next = 0;
    char* words = getwords(buf, &next);
    load_words(words, lexicon);
    sscanf(next, "%f %f (%1u, %u)",
           &pr, &bow, &bol, &bon);
}

void 
CArpaSlm::TNode::load_level0(istream& is)
{
    hw[0] = 0;
    char buf[1024];
    is.getline(buf, sizeof(buf));
    sscanf(buf, "%f %f (%1u, %u)",
           &pr, &bow, &bol, &bon);
    wid = 0;
}

void
CArpaSlm::load(const char* filename, const TLexicon& lexicon)
{
    printf("Loading ARPA slm..."); fflush(stdout);
    ifstream file(filename);
    char buf[1024];
    for (int i = 0; i <= N_GRAM; ++i) {
        unsigned lvl;
        int size;
        file.getline(buf, sizeof(buf));
        if (!file) {
            cerr << "Failed to read from" << filename << endl;
            exit(1);
        }
        sscanf(buf, "\\%d-gram\\%d%*[\n]", &lvl, &size);
        assert(lvl <= N_GRAM);
        if (lvl == 0) {
            TNode node0;
            node0.load_level0(file);
            m_levels[0].push_back(node0);
        }
        else if (lvl < m_N) {
            m_levels[lvl].reserve(size);
            for (int i = 0; i < size; ++i) {
                TNode node;
                node.load(file, lexicon);
                m_levels[lvl].push_back(node);
            }
        } else {
            // leaf nodes
            m_lastLevel.reserve(size);
            for (int i = 0; i < size; ++i) {
                TLeaf leaf;
                leaf.load(file, lexicon);
                m_lastLevel.push_back(leaf);
            }
        }
    }
}

template <class NodeT>
struct CompareNode
{
    const unsigned m_lvl;
    CompareNode(unsigned lvl) : m_lvl(lvl) {}
    /**
     * @return true if strictly less, false otherwise
     */
    bool operator () (const NodeT& node, const TSIMWordId hw[N_GRAM]) {
        for (unsigned i = 0; i < m_lvl; ++i) {
            if (node.hw[i] < hw[i])
                return true;
            if (node.hw[i] > hw[i])
                return false;
        }
        // node.hw[:lvl] is the same as hw[:]
        return false;
    }
};
  
void 
CArpaSlm::threading()
{
    {
        TNode& node = m_levels[0][0];
        node.ch = 0;
    }
    for (unsigned lvl = 1; lvl < m_N; ++lvl) {
        TNodeLevel& level = m_levels[lvl];
        unsigned last_child = 0;
        for (TNodeLevel::iterator node = level.begin(); node != level.end(); ++node) {
            node->ch = last_child = find_1st_child(lvl, *node, last_child);
        }
    }
}

unsigned
CArpaSlm::find_1st_child(unsigned lvl, const TNode& node, int last_child)
{
    assert (lvl < m_N);
    if (lvl == m_N-1) {
        TLeafLevel::iterator found = lower_bound(m_lastLevel.begin(), m_lastLevel.end(), node.hw, CompareNode<TLeaf>(lvl));
        return distance(m_lastLevel.begin(), found);
    } else {
        const TNodeLevel& level = m_levels[lvl+1];
        TNodeLevel::const_iterator found = lower_bound(level.begin(), level.end(), node.hw, CompareNode<TNode>(lvl));
        return distance(level.begin(), found);
    }
}

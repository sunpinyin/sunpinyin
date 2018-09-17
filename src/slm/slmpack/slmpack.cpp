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

/*
 * pack ARPA format to a binary format which can be consumed by SunPinyin
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
#include <iostream>
#include <cmath>

#include "../sim_slm.h"
#include "arpa_slm.h"

void
ShowUsage(const char* progname)
{
    printf("Usage:\n");
    printf("    %s arpa_slm dict_file output_slm\n", progname);
    printf("\n");
    printf("Description:\n");
    printf(
        "    %s converts the ARPA representation of SLM to the binary format of SLM. \n",
        progname);
    exit(100);
}

TLexicon
read_lexicon(const char* filename)
{
    printf("Loading lexicon..."); fflush(stdout);
    static char word[1024 * 10];
    FILE* f_lex = fopen(filename, "r");
    if (f_lex == NULL) {
        fprintf(stderr, "Failed to open lexicon file %s: %s\n", filename, strerror(errno));
        exit(EXIT_FAILURE);
    }
    TLexicon lexicon;
    while (fgets(word, sizeof(word), f_lex)) {
        if (strlen(word) > 0) {
            // skip to the first non hanzi character
            char* p = word;
            while (*p == ' ' || *p == '\t')
                ++p;
            while (*p != 0 && *p != ' ' && *p != '\t')
                ++p;
            if (*p == 0) continue;
            *p++ = 0;
            // skip to the word_id
            while (*p == ' ' || *p == '\t')
                ++p;
            if (!(*p >= '0' && *p <= '9')) continue;

            int id;
            for (id = 0; *p >= '0' && *p <= '9'; ++p)
                id = 10 * id + (*p - '0');
            lexicon[std::string(word)] = id;
        }
    }
    fclose(f_lex);
    printf("done.\n"); fflush(stdout);

    return lexicon;
}

//
// filename [in]
// levels[0] [in]
// ...
// levels[N] [in]
// lastLevel [in]
//
void
write_out(const char* filename, const CArpaSlm& slm)
{
    printf("\nWriting out..."); fflush(stdout);

    FILE* fp = fopen(filename, "wb");
    const int N = slm.getN();
    fwrite(&N, sizeof(int), 1, fp);
    const unsigned usingLogPr = slm.usingLogPr();
    fwrite(&usingLogPr, sizeof(unsigned), 1, fp);

    for (int lvl = 0; lvl <= N; ++lvl) {
        unsigned len = slm.getLevelSize(lvl) + 1;
        fwrite(&len, sizeof(unsigned), 1, fp);
    }

    for (int lvl = 0; lvl < N; ++lvl) {
        const CArpaSlm::TNodeLevel& level = slm.getLevel(lvl);
        for (CArpaSlm::TNodeLevel::const_iterator iter = level.begin();
                iter != level.end(); ++iter) {
            CSIMSlm::TNode node(iter->wid, iter->child, 0, iter->bow);
            node.pr = iter->pr;
            fwrite(&node, sizeof(CSIMSlm::TNode), 1, fp);
        }
        CSIMSlm::TNode node(0x00FFFFFF, slm.getLevel(lvl + 1).size(), 1, 0);
        fwrite(&node, sizeof(CSIMSlm::TNode), 1, fp);
    }

    const CArpaSlm::TLeafLevel& level = slm.getLastLevel();
    for (CArpaSlm::TLeafLevel::const_iterator iter = level.begin();
            iter != level.end(); ++iter) {
        CSIMSlm::TLeaf node(iter->wid, 0);
        node.pr = iter->pr;
        fwrite(&node, sizeof(CSIMSlm::TLeaf), 1, fp);
    }
    CSIMSlm::TLeaf node(0, 1);
    fwrite(&node, sizeof(CSIMSlm::TLeaf), 1, fp);

    fclose(fp);
    printf("done!\n"); fflush(stdout);
}

int
main(int argc, char* argv[])
{
    if (argc != 4)
        ShowUsage(argv[0]);
    const char* arpa_path = argv[1];
    const char* lexicon_path = argv[2];
    const char* output_path = argv[3];

    CArpaSlm slm;
    TLexicon lexicon = read_lexicon(lexicon_path);
    slm.load(arpa_path, lexicon);

    if (!slm.good()) {
        std::cerr << "Failed to load language model from " << arpa_path <<
        "." << std::endl;
        exit(1);
    }
    slm.initChild();
    write_out(output_path, slm);
    return 0;
}

// -*- indent-tabs-mode: nil -*- vim:et:ts=4

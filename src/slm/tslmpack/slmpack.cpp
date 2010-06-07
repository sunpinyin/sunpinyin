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

//#include "../sim_slm.h"
#include "../slm.h"

#include "../thread/ValueCompress.h"
#include "arpa_slm.h"
#include "arpa_conv.h"


void ShowUsage(const char* progname)
{
    printf("Usage:\n");
    printf("    %s arpa_slm dict_file threaded_slm\n", progname);
    printf("\n");
    printf("Description:\n");
    printf("    %s converts the ARPA representation of SLM to the binary format of threaded SLM. \n", progname);
    exit(100);
}

/**
 * slm [in]
 * pr_eff, pr_values [out]
 * bow_eff, bow_values [out]
 */

void build_map(const CArpaSlm& slm, EffRealMap &pr_eff, FreqMap& pr_values, EffRealMap &bow_eff, FreqMap& bow_values)
{
    bool usingLogPr = slm.usingLogPr();

    printf("\nfirst pass..."); fflush(stdout);

    for (unsigned lvl=0; lvl < slm.getN(); ++lvl) {
        typedef CArpaSlm::TNodeLevel TNodeLevel;
        const TNodeLevel& level = slm.getLevel(lvl);
        for (TNodeLevel::const_iterator node = level.begin(); node != level.end(); ++node) {
            float real_pr, eff_pr;
            real_pr = node->pr;
            eff_pr = EffectivePr(real_pr);
            if (pr_eff.find(eff_pr) == pr_eff.end()) {
                pr_eff[eff_pr] = real_pr;
            } else { // precision error cause non 1:1 mapping
                pr_eff[eff_pr] = OriginalPr(eff_pr);
            }
            ++(pr_values[eff_pr]);

            float real_bow, eff_bow;
            real_bow = node->bow;
            eff_bow = EffectiveBow(real_bow);
            if (bow_eff.find(eff_bow) == bow_eff.end()) {
                bow_eff[eff_bow] = real_bow;
            } else { // two values map to same distance value due to precision error
                bow_eff[eff_bow] = OriginalBow(eff_bow);
            }
            ++(bow_values[eff_bow]);
        }
    }
    typedef CArpaSlm::TLeafLevel TLeafLevel;
    const TLeafLevel& level = slm.getLastLevel();
    for (TLeafLevel::const_iterator leaf = level.begin(); leaf != level.end(); ++leaf) {
        float real_pr, eff_pr;
        real_pr = leaf->pr;
        eff_pr = EffectivePr(real_pr);
        if (pr_eff.find(eff_pr) == pr_eff.end()) {
            pr_eff[eff_pr] = real_pr;
        } else { // precision error cause non 1:1 mapping
            pr_eff[eff_pr] = OriginalPr(eff_pr);
        }
        ++(pr_values[eff_pr]);
    }
    // Following pr value should not be grouped, or as milestone values.
    static const float msprs[] = {
        0.9, 0.8, 0.7, 0.6,
        1.0/2, 1.0/4, 1.0/8, 1.0/16, 1.0/32, 1.0/64, 1.0/128,
        1.0/256, 1.0/512, 1.0/1024, 1.0/2048, 1.0/4096, 1.0/8192,
        1.0/16384, 1.0/32768, 1.0/65536
    };

    for (unsigned i=0, sz=sizeof(msprs)/sizeof(float); i < sz; ++i) {
        float real_pr = (usingLogPr)?(-log(msprs[i])):(msprs[i]);
        float eff_pr = EffectivePr(real_pr);
        assert(usingLogPr || (real_pr > 0.0 && real_pr < 1.0));
        assert(!usingLogPr || real_pr > 0.0);

        if (pr_eff.find(eff_pr) == pr_eff.end()) {
            pr_eff[eff_pr] = real_pr;
        } else { // precision error causes non 1:1 mapping
            pr_eff[eff_pr] = OriginalPr(eff_pr);
        }
        pr_values[eff_pr] = 0;
    }

    // Following bow value should not be grouped, or as milestone values.
    static const float msbows[] = {
        1.0, 0.9, 0.8, 0.7, 0.6, 0.5, 0.4, 0.3, 0.2,
        0.1, 0.05, 0.01, 0.005, 0.001, 0.0005, 0.0001,
        0.00005, 0.00001, 0.000005, 0.000001, 0.0000005, 0.0000001
    };

    for (unsigned i=0; i < sizeof(msbows)/sizeof(msbows[0]); ++i) {
        float real_bow = (usingLogPr)?(-log(msbows[i])):(msbows[i]);
        float eff_bow = EffectiveBow(real_bow);
        if (bow_eff.find(eff_bow) == bow_eff.end()) {
            bow_eff[eff_bow] = real_bow;
        } else { // two values map to same distance value due to precision error
            bow_eff[eff_bow] = OriginalBow(eff_bow);
        }
        bow_values[eff_bow] = 0;
    }
}

/** 
 * group vaules into a smaller set of their approximations
 *
 * bow_eff [in], bow_values [in], bow_map [out], bow_table [out]
 * pr_eff [in], pr_values [in], pr_map [out], pr_table [out]
 *
 */ 
void group_values(bool usingLogPr,
                  EffRealMap& pr_eff, FreqMap& pr_values, CompressedTable& pr_table, RealIndexMap& pr_map,
                  EffRealMap& bow_eff, FreqMap& bow_values, CompressedTable& bow_table, RealIndexMap& bow_map)
{
    printf("\nCompressing pr values..."); fflush(stdout);
    CValueCompressor vc;
    vc(pr_eff, pr_values, pr_map, pr_table, (1 << CThreadSlm::BITS_PR));
    CompressedTable::iterator itt, itte;
    itte = pr_table.end();
    for (itt = pr_table.begin(); itt != itte; ++itt) {
        *itt = OriginalPr(*itt);
        assert(usingLogPr || (*itt > 0.0 && *itt < 1.0));
        assert(!usingLogPr || *itt > 0.0);
    }
    printf("%lu float values ==> %lu values", pr_eff.size(), pr_table.size());

    printf("\nCompressing bow values..."); fflush(stdout);
    vc(bow_eff, bow_values, bow_map, bow_table, (1 << CThreadSlm::BITS_BOW));
    itte = bow_table.end();
    for (itt = bow_table.begin(); itt != itte; ++itt)
        *itt = OriginalBow(*itt);
    printf("%lu float values ==> %lu values", bow_eff.size(), bow_table.size());
}

TLexicon read_lexicon(const char* filename)
{
    printf("Loading lexicon..."); fflush(stdout);
    static char word[1024*10];
    FILE* f_lex = fopen(filename, "r");
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
            for (id=0; *p >= '0' && *p <= '9'; ++p)
                id = 10*id + (*p - '0');
            lexicon[std::string(word)] = id;
        }
    }
    fclose(f_lex);
    printf("done.\n"); fflush(stdout);

    return lexicon;
}



// 
// filename [in]
// pr_table [in]
// bow_table [in]
// levels[0] [in]
// ...
// levels[N] [in]
// lastLevel [in]
// 
void write_out(const char* filename, const CArpaSlm& slm,
               CompressedTable& pr_table, CompressedTable& bow_table, 
               const TNodeLevels& levels, const CThreadSlm::TLeaf* lastLevel)
{
    printf("\nWriting out..."); fflush(stdout);
    
    FILE* fp = fopen(filename, "wb");
    const int N = slm.getN();
    fwrite(&N, sizeof(int), 1, fp);
    const unsigned usingLogPr = slm.usingLogPr();
    fwrite(&usingLogPr, sizeof(unsigned), 1, fp);
    
    for (int lvl = 0; lvl <= N; ++lvl) {
        unsigned len = slm.getLevelSize(lvl)+1;
        fwrite(&len, sizeof(unsigned), 1, fp);
    }
    
    for (int i = 0, sz = pr_table.size(); i < (1 << CThreadSlm::BITS_PR); ++i) {
        if (i < sz) {
            fwrite(&pr_table[i], sizeof(float), 1, fp);
        } else {
            float dummy = 0.0F;
            fwrite(&dummy, sizeof(float), 1, fp);
        }
    }
    
    for (int i = 0, sz = bow_table.size(); i < (1 << CThreadSlm::BITS_BOW); ++i) {
        if (i < sz) {
            fwrite(&bow_table[i], sizeof(float), 1, fp);
        } else {
            float dummy = 0.0F;
            fwrite(&dummy, sizeof(float), 1, fp);
        }
    }
    
    for (int lvl=0; lvl < N; ++lvl) {
        fwrite(levels[lvl], sizeof(CThreadSlm::TNode), slm.getLevelSize(lvl)+1, fp);
    }
    
    fwrite(lastLevel, sizeof(CThreadSlm::TLeaf), slm.getLevelSize(N)+1, fp);
    
    fclose(fp);

    printf("done!\n"); fflush(stdout);
}


void cleanup(CompressedTable& pr_table, CompressedTable& bow_table,
             TNodeLevels& levels, CThreadSlm::TLeaf* lastLevel)
{
    for (unsigned lvl=0; lvl < levels.size(); ++lvl)
        delete[] levels[lvl];
    delete[] lastLevel;
    bow_table.clear();
    pr_table.clear();
}

int main(int argc, char* argv[])
{
    
    if (argc != 4)
        ShowUsage(argv[0]);
    const char* arpa_path = argv[1];
    const char* lexicon_path = argv[2];
    const char* threaded_path = argv[3];
    
    CArpaSlm slm;
    TLexicon lexicon = read_lexicon(lexicon_path);
    slm.load(arpa_path, lexicon);
    
    if (!slm.good()) {
        std::cerr << "Failed to load language model from " << arpa_path << "." << std::endl;
        exit(1);
    }
    slm.threading();
    
    EffRealMap pr_eff, bow_eff;	   // effval --> val
    FreqMap pr_values, bow_values; // effval --> freq
    build_map(slm, pr_eff, pr_values, bow_eff, bow_values);
    
    RealIndexMap    pr_map, bow_map;		// result: val --> int
    CompressedTable pr_table, bow_table;	// result: val vector    
    group_values(slm.usingLogPr(),
                 pr_eff, pr_values, pr_table, pr_map,
                 bow_eff, bow_values, bow_table, bow_map);
    pr_values.clear();
    bow_values.clear();

    TNodeLevels levels;
    CThreadSlm::TLeaf* lastLevel;
    compress(slm, pr_table, pr_map, bow_table, bow_map,
             levels, lastLevel);
    
    pr_map.clear();
    bow_map.clear();
    write_out(threaded_path, slm, pr_table, bow_table, levels, lastLevel);
    
    cleanup(pr_table, bow_table, levels, lastLevel);
    return 0;
}

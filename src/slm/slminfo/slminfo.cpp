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

#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>

#include <map>

#include "../sim_slm.h"

void ShowUsage()
{
    printf("Usage:\n");
    printf("    slminfo [options] slm_file\n");
    printf("\nDescription:\n");
    printf("    slminfo tell information of back-off language model 'slm_file'. It can also print the model to ARPA format.\n");
    printf("  When no options given, slminfo will only print number of items in each level of the language model.\n");
    printf("\nOptions:\n");
    printf("    -v             # Verbose mode, printing arpa format.\n");
    printf("    -p             # Prefer normal probability than -log(Pr) which is default. Valid under -v option.\n");
    printf("    -l dict_file   # Lexicon. Valid under -v option. Substitute the word-id with word-text in the output.\n");
    printf("\n");
    exit(100);
}

static bool verbose = false;
static char* lexicon_filename = NULL;
static bool output_log_pr = true;

static struct option long_options[] =
{
    {"verbose", 0, 0, 'v'},
    {"pr", 0, 0, 'p'},
    {"lexicon", 1, 0, 'l'},
    {0, 0, 0, 0}
};

static void getParameters(int argc, char* argv[])
{
   int c, option_index = 0;
   while ((c=getopt_long(argc, argv, "vpl:", long_options, &option_index)) != -1)
   {
      switch (c) {
        case 'v':
            verbose = true;
            break;
      case 'l':
            lexicon_filename = strdup(optarg);
         break;
        case 'p':
            output_log_pr = false;
            break;
        default:
            ShowUsage();
        }
    }
    if (output_log_pr == false && !verbose) ShowUsage();
    if (lexicon_filename != NULL && !verbose) ShowUsage();
    if (optind != argc-1) ShowUsage();
}

typedef std::map<TSIMWordId, std::string> TReverseLexicon;

void PrintARPALevel(int lvl, FILE* fp, TReverseLexicon* plexicon, bool output_log_pr)
{
    int N, bLogPrFile;

    fseek(fp, 0, SEEK_SET);
    fread(&N, sizeof(int), 1, fp);
    fread(&bLogPrFile, sizeof(bLogPrFile), 1, fp);

    assert(N < 16);
    int sz[16];	           //it should be N+1, Yet some compiler do not support int sz[N+1]
    long level_offset[16]; //it should be N+1, Yet some compiler do not support it

    fread(sz, sizeof(int), N+1, fp);
    long offset = ftell(fp);
    for (int i=0; i <= N; ++i) {
        level_offset[i] = offset;
        offset += sz[i]*sizeof(CSIMSlm::TNode);
    }

    TSIMWordId ngram[16];   //it should be N+1, Yet some compiler do not support it
    int idx[16];   //it should be N+1, Yet some compiler do not support it
    CSIMSlm::TNode nodes[16][2];  //it should be N+1, Yet some compiler do not support it
    for (int i=0; i <= lvl; ++i) {
        idx[i] = 0;
        if (i == N) {
            fseek(fp, level_offset[i]+sizeof(CSIMSlm::TLeaf)*idx[i], SEEK_SET);
            fread(&(nodes[i][0]), sizeof(CSIMSlm::TLeaf), 1, fp);
            fread(&(nodes[i][1]), sizeof(CSIMSlm::TLeaf), 1, fp);
        } else {
            fseek(fp, level_offset[i]+sizeof(CSIMSlm::TNode)*idx[i], SEEK_SET);
            fread(&(nodes[i][0]), sizeof(CSIMSlm::TNode), 2, fp);
        }
    }

    printf("/%d-gram:%d/\n", lvl, sz[lvl]-1);
    while (idx[lvl] < sz[lvl]-1) {
        for (int i=lvl-1; i>0; --i) {
            bool change = false;
            while (nodes[i][1].child <= idx[i+1]) {
                change = true;
                //load next item
                ++idx[i];
                nodes[i][0] = nodes[i][1];
                fseek(fp, level_offset[i]+sizeof(CSIMSlm::TNode)*(idx[i]+1), SEEK_SET);
                fread(&(nodes[i][1]), sizeof(CSIMSlm::TNode), 1, fp);
            }
            if (change == false) break;
        }

        for (int i=1; i<=lvl; ++i) {
            TSIMWordId word_id = nodes[i][0].id;
            if (plexicon != NULL)
                printf("%s ", (*plexicon)[word_id].c_str());
            else
                printf("%d ", int(word_id));
        }
        if (bLogPrFile) {
            if (output_log_pr)
                printf("%20.17lf ", double(nodes[lvl][0].pr));
            else
                printf("%20.17lf ", exp(-double(nodes[lvl][0].pr)));
            if (lvl != N) {
                if (output_log_pr)
                    printf("%20.17lf", double(nodes[lvl][0].bow));
                else
                    printf("%20.17lf", exp(-double(nodes[lvl][0].bow)));
            }
        } else {
            if (output_log_pr)
                printf("%20.17lf ", -log(double(nodes[lvl][0].pr)));
            else
                printf("%20.17lf ", double(nodes[lvl][0].pr));
            if (lvl != N) {
                if (output_log_pr)
                    printf("%20.17lf", -log(double(nodes[lvl][0].bow)));
                else
                    printf("%20.17lf", double(nodes[lvl][0].bow));
            }
        }
        printf("\n");

        ++idx[lvl];
        nodes[lvl][0] = nodes[lvl][1];
        if (lvl == N) {
            fseek(fp, level_offset[lvl]+sizeof(CSIMSlm::TLeaf)*(idx[lvl]+1), SEEK_SET);
            fread(&(nodes[lvl][1]), sizeof(CSIMSlm::TLeaf), 1, fp);
        } else {
            fseek(fp, level_offset[lvl]+sizeof(CSIMSlm::TNode)*(idx[lvl]+1), SEEK_SET);
            fread(&(nodes[lvl][1]), sizeof(CSIMSlm::TNode), 1, fp);
        }
    }
}

void PrintARPA(FILE* fp, const char* lexicon_filename, bool output_log_pr)
{
    static int  id;
    static char word[10240];

    int N;
    TReverseLexicon* plexicon = NULL;
    if (lexicon_filename != NULL) {
        plexicon = new TReverseLexicon();
        FILE* f_lex = fopen(lexicon_filename, "r");
        while (fgets(word, 10240, f_lex) != NULL) {
            if (strlen(word) > 0) {
                char* p = word;
                while (*p == ' ' || *p == '\t')
                    ++p;
                while (*p != 0 && *p != ' ' && *p != '\t')
                    ++p;
                if (*p == 0) continue;
                *p++ = 0;
                while (*p == ' ' || *p == '\t')
                    ++p;
                if (!(*p >= '0' && *p <= '9')) continue;
                for (id=0; *p >= '0' && *p <= '9'; ++p)
                    id = 10*id + (*p - '0');
                (*plexicon)[TSIMWordId(id)] = std::string(word);
            }
        }
        fclose(f_lex);
    }
    fseek(fp, 0, SEEK_SET);
    fread(&N, sizeof(N), 1, fp);
    for (int lvl = 1; lvl <= N; ++lvl)
        PrintARPALevel(lvl, fp, plexicon, output_log_pr);
}

void PrintSimple(FILE* fp)
{
    int N;
    int nItem;
    int bLogPrFile;

    fseek(fp, 0, SEEK_SET);
    fread(&N, sizeof(N), 1, fp);
    fread(&bLogPrFile, sizeof(bLogPrFile), 1, fp);
    printf("This is a %d-gram back-off model, ", N);
    printf("%s\n", (bLogPrFile)?("using -log(pr)"):("using direct pr"));
    for (int i=0; i <=N; ++i) {
        fread(&nItem, sizeof(nItem), 1, fp);
        printf("  %d items in %d-level\n", nItem-1, i);
    }
}

int main(int argc, char* argv[])
{
    FILE* fp = NULL;

    getParameters(argc, argv);

    if ((fp = fopen(argv[argc-1], "rb+")) == NULL) {
        printf("Can not open back-off language model file %s\n", argv[argc-1]);
        return 99;
    }

    if (!verbose)
        PrintSimple(fp);
    else
        PrintARPA(fp, lexicon_filename, output_log_pr);

    fclose(fp);

    return 0;
}

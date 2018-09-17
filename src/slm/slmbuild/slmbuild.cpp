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
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>

#include <vector>
#include <algorithm>

#include "sim_slmbuilder.h"

static struct option long_options[] =
{
    { "ngram", 1, 0, 'n' },
    { "out", 1, 0, 'o' },
    { "cut", 1, 0, 'c' },
    { "discount", 1, 0, 'd' },
    { "wordcount", 1, 0, 'w' },
    { "breakid", 1, 0, 'b' },
    { "excludeid", 1, 0, 'e' },
    { "log", 1, 0, 'l' },
    { NULL, 0, 0, 0 }
};

static void
ShowUsage(void)
{
    printf(
        "\
Usage:\n\
  slmbuild options idngram\n\
\n\
Description:\n\
  This program generate language model from idngram file.\n\
\n\
Options:\n\
  -n --ngram     N            # 1 for unigram, 2 for bigram, 3 for trigram...\n\
  -o --out       output       # output file name\n\
  -l --log                    # using -log(pr), default use pr directly\n\
  -w --wordcount N            # Lexicon size, number of different word\n\
  -b --brk       id[,id...]   # set the ids which should be treat as breaker\n\
  -e --exclude   id[,id...]   # set the ids which should not be put into LM\n\
  -c --cut       c1[,c2...]   # k-gram whose freq <= c[k] are dropped\n\
  -d --discount  method,param # the k-th -d parm specify the discount method \n\
      for k-gram. Possible values for method/param:\n\
          GT,R,dis  : GT discount for r <= R, r is the freq of a ngram.\n\
                      Linear discount for those r > R, i.e. r'=r*dis\n\
                      0 << dis < 1.0, for example 0.999 \n\
          ABS,[dis] : Absolute discount r'=r-dis. And dis is optional\n\
                      0 < dis < cut[k]+1.0, normally dis < 1.0.\n\
          LIN,[dis] : Linear discount r'=r*dis. And dis is optional\n\
                      0 < dis < 1.0\n\
\n\
Notes:\n\
      -n must be given before -c -b. And -c must give right number of cut-off,\n\
  also -d must appear exactly N times specify discount for 1-gram, 2-gram..., \n\
  respectively.\n\
      BREAKER-IDs could be SentenceTokens or ParagraphTokens. Concepturally,\n\
  these ids has no meaning when they appeared in the middle of n-gram.\n\
      EXCLUDE-IDs could be ambiguious-ids. Concepturally, n-grams which\n\
  contain those ids are meaningless.\n\
      We can not erase ngrams according to BREAKER-IDS and EXCLUDE-IDs directly\n\
  from IDNGRAM file, because some low-level information still useful in it.\n\
\n\
Example:\n\
      Following example read 'all.id3gram' and write trigram model 'all.slm'.\n\
  At 1-gram level, use Good-Turing discount with cut-off 0, R=8, dis=0.9995. At\n\
  2-gram level, use Absolute discount with cut-off 3, dis auto-calc. At 3-gram\n\
  level, use Absolute discount with cut-off 2, dis auto-calc. Word id 10,11,12\n\
  are breakers (sentence/para/paper breaker, etc). Exclude-ID is 9. Lexicon \n\
  contains 200000 words. The result languagme model use -log(pr).\n\
\n\
        slmbuild -l -n 3 -o all.slm -w 200000 -c 0,3,2 -d GT,8,0.9995\n\
                 -d ABS -d ABS -b 10,11,12 -e 9 all.id3gram\n\
\n");
    exit(100);
}

static int N = 0;
static CSlmBuilder builder;
static char* inputfilename = NULL;
static char* outfilename = NULL;
static std::vector<CSlmDiscounter *> discounter;

static void
getParameters(int argc, char* argv[])
{
    int c = 0;
    char *ac = NULL, *cuts = NULL, *idstring = NULL, *dis_str = NULL;
    std::vector<TSIMWordId> ids;
    std::vector<CSlmBuilder::FREQ_TYPE> threshold;
    bool bUseLogPr = false;

    while ((c =
                getopt_long(argc, argv, "lw:n:c:d:o:b:e:", long_options,
                            NULL)) != -1) {
        int n, rmax, cut;
        double dis = 0;
        switch (c) {
        case 'l':
            bUseLogPr = true;
            break;
        case 'o':
            outfilename = strdup(optarg);
            break;
        case 'n':
            N = atoi(optarg);
            builder.Create(N);
            break;
        case 'w':
            n = atoi(optarg);
            builder.SetNumberOfWord(n);
            break;
        case 'c':
            cuts = strdup(optarg);
            ac = strtok(cuts, ",");
            while (ac != NULL) {
                cut = atoi(ac);
                threshold.push_back(CSlmBuilder::FREQ_TYPE(cut));
                ac = strtok(NULL, ",");
            }
            builder.SetCut(&(threshold[0]));
            free(cuts);
            break;
        case 'b':
            idstring = strdup(optarg);
            ac = strtok(idstring, ",");
            while (ac != NULL) {
                n = atoi(ac);
                ids.push_back(n);
                ac = strtok(NULL, ",");
            }
            builder.SetBreakerIds(ids.size(), &(ids[0]));
            free(idstring);
            break;
        case 'e':
            idstring = strdup(optarg);
            ac = strtok(idstring, ",");
            while (ac != NULL) {
                n = atoi(ac);
                ids.push_back(n);
                ac = strtok(NULL, ",");
            }
            builder.SetExcludeIds(ids.size(), &(ids[0]));
            free(idstring);
            break;
        case 'd':
            dis_str = strdup(optarg);
            ac = strtok(dis_str, ",");
            if (strcmp(ac, "GT") == 0) {
                ac = strtok(NULL, ",");
                rmax = atoi(ac);
                ac = strtok(NULL, ",");
                dis = atof(ac);
                discounter.push_back(new CSlmGTDiscounter(rmax, dis));
            } else if (strcmp(ac, "ABS") == 0) {
                if ((ac = strtok(NULL, ",")) != NULL)
                    dis = atof(ac);
                discounter.push_back(new CSlmAbsoluteDiscounter(dis));
            } else if (strcmp(ac, "LIN") == 0) {
                if ((ac = strtok(NULL, ",")) != NULL)
                    dis = atof(ac);
                discounter.push_back(new CSlmLinearDiscounter(dis));
            }
            break;
        default:
            ShowUsage();
        }
    }

    builder.SetUseLogPr(((bUseLogPr) ? 1 : 0));
    if (optind == argc - 1) {
        inputfilename = strdup(argv[optind]);
        builder.SetDiscounter(&(discounter[0]));
    } else {
        fprintf(stderr, "Parameter input_file error\n");
        for (int i = optind; i < argc; ++i)
            fprintf(stderr, "%s ", argv[i]);
        fprintf(stderr, "\n");
        ShowUsage();
    }
}

int
main(int argc, char* argv[])
{
    getParameters(argc, argv);

    TSIMWordId * ngram = new TSIMWordId[N + 1];
    CSlmBuilder::FREQ_TYPE freq;

    printf("Reading and Processing raw idngram..."); fflush(stdout);

    FILE* fp = fopen(inputfilename, "rb");
    if (fp == NULL) {
        fprintf(stderr, "Failed to open raw idngram file %s: %s\n", inputfilename, strerror(errno));
        return EXIT_FAILURE;
    }

    int nItems = 0;
    while (fread(ngram, sizeof(TSIMWordId), N, fp) == (size_t) N
           && fread(&freq, sizeof(freq), 1, fp) == 1) {
        builder.AddNGram(ngram, freq);
        ++nItems;
    }
    fclose(fp);
    delete[] ngram;
    printf("%d ngrams.\n", nItems); fflush(stdout);

    builder.Build();

    printf("\nWriting result file..."); fflush(stdout);
    FILE *out = fopen(outfilename, "wb");
    builder.Write(out);
    fclose(out);
    printf("\n"); fflush(stdout);

    return 0;
}

// -*- indent-tabs-mode: nil -*- vim:et:ts=4

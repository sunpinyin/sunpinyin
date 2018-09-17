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
#include <map>
#include <vector>
#include <algorithm>
#include <climits>

#include "../sim_fmerge.h"
#include "idngram.h"
#include "idngram_merge.h"

template<int N>
void
WriteOut(FILE* out, std::map<CSIM_Idngram<N>, unsigned int> & map)
{
    typedef typename std::map<CSIM_Idngram<N>,
                              unsigned int>::iterator TMapIterator;
    TMapIterator its = map.begin(), ite = map.end();
    for (; its != ite; ++its) {
        fwrite(its->first.ids, sizeof(TSIMWordId), N, out);
        fwrite(&(its->second), sizeof(unsigned int), 1, out);
    }
    map.clear();
}

template<int N>
void
ProcessingRead(FILE *fp,
               FILE* swap,
               std::vector<long>& para_offsets,
               size_t paraMax)
{
    typedef CSIM_Idngram<N> TNgram;
    typedef typename std::map<CSIM_Idngram<N>, unsigned int> TMap;

    TMap map;
    TNgram ngram;

    TSIMWordId* ids = ngram.ids;
    fread(ids, sizeof(TSIMWordId), N - 1, fp);
    while (fread(ids + N - 1, sizeof(TSIMWordId), 1, fp) == 1) {
        assert(map[ngram] < UINT_MAX);
        ++map[ngram];
        if (map.size() >= paraMax) {
            printf("."); fflush(stdout);
            WriteOut(swap, map);
            para_offsets.push_back(ftell(swap));
        }
        for (int i = 0; i < N - 1; ++i) ids[i] = ids[i + 1];
    }
    if (map.size() > 0) {
        printf("."); fflush(stdout);
        WriteOut(swap, map);
        para_offsets.push_back(ftell(swap));
    }
}

static struct option long_options[] =
{
    { "NMax", 1, 0, 'n' },
    { "out", 1, 0, 'o' },
    { "swap", 1, 0, 's' },
    { "para", 1, 0, 'p' },
    { 0, 0, 0, 0 }
};

static int N = 0;
static int paraMax = 0;
static char* output = NULL;
static char* swapfile = NULL;

void
ShowUsage()
{
    printf("Usage:\n\tids2ngram options idsfile[ idsfile...]\n");
    printf("\nDescription\n");
    printf(
        "   This program generate idngram file, which is a sorted [id1,..idN,freq] array, from binary id stream files.\n");
    printf("\nInput:\n");
    printf("\tBinary id stream files looks like [id0,...,idX]\n");
    printf("\nOptions:\n");
    printf("\t  -n N               # N-gram\n");
    printf("\t  -s swapfile        # intermedia temporary file\n");
    printf(
        "\t  -o outputfile      # result idngram file [id1, ... idN, freq]*\n");
    printf("\t  -p para_size       # maximum ngram-items per para\n");
    printf("\nExample:\n");
    printf(
        "   Following example will use three input idstream file idsfile[1,2,3] to generate the idngram file all.id3gram. Each para (internal map size or hash size) would be 1024000, using swap file for temp result. All temp para result would final be merged to got the final result.\n");
    printf(
        "\tids2idngram -n 3 -s /tmp/swap -o all.id3gram -p 1024000 idsfile1 idsfile2 idsfile3\n\n");
    exit(100);
}

static void
getParameters(int argc, char* const argv[])
{
    int option_index = 0;
    int c;
    while ((c =
                getopt_long(argc, argv, "p:n:s:o:", long_options,
                            &option_index)) != -1) {
        switch (c) {
        case 'n':
            N = atoi(strdup(optarg));
            break;
        case 'p':
            paraMax = atoi(strdup(optarg));
            break;
        case 'o':
            output = strdup(optarg);
            break;
        case 's':
            swapfile = strdup(optarg);
            break;
        default:
            ShowUsage();
        }
    }
    if (N < 1 || N > 3 || paraMax < 1024 || output == NULL || swapfile == NULL)
        ShowUsage();
}

static std::vector<long> para_offsets;

int
main(int argc, char* argv[])
{
    getParameters(argc, argv);
    FILE *swap = fopen(swapfile, "wb+");
    FILE *out = fopen(output, "wb+");
    if (optind >= argc) ShowUsage();
    for (; optind < argc; ++optind) {
        printf("Processing %s:", argv[optind]); fflush(stdout);
        FILE *fp = fopen(argv[optind], "rb");
        if (fp == NULL) {
            fprintf(stderr, "Failed to open %s: %s\n", argv[optind], strerror(errno));
            printf("\n");
            continue;
        }
        switch (N) {
        case 1:
            ProcessingRead<1>(fp, swap, para_offsets, paraMax);
            break;
        case 2:
            ProcessingRead<2>(fp, swap, para_offsets, paraMax);
            break;
        case 3:
            ProcessingRead<3>(fp, swap, para_offsets, paraMax);
            break;
        }
        fclose(fp);
        printf("\n");
    }
    printf("Merging..."); fflush(stdout);
    switch (N) {
    case 1:
        ProcessingIdngramMerge<1>(swap, out, para_offsets);
        break;
    case 2:
        ProcessingIdngramMerge<2>(swap, out, para_offsets);
        break;
    case 3:
        ProcessingIdngramMerge<3>(swap, out, para_offsets);
        break;
    }
    printf("Done\n"); fflush(stdout);
    fclose(out);
    fclose(swap);
    return 0;
}

// -*- indent-tabs-mode: nil -*- vim:et:ts=4

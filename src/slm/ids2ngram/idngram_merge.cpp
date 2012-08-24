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
#include <vector>
#include <algorithm>

#include "../sim_fmerge.h"
#include "idngram.h"
#include "idngram_merge.h"

static struct option long_options[] =
{
    { "NMax", 1, 0, 'n' },
    { "out", 1, 0, 'o' },
    { 0, 0, 0, 0 }
};

static int N = 0;
// static int paraMax = 0;
static char* output = NULL;
// static char* swapfile = NULL;

void
ShowUsage()
{
    printf("Usage:\n\tidngram_merge options idngramfile[ idngramfile...]\n");
    printf("\nDescription:\n");
    printf(
        "   This program merge multiple idngram file, each of them are sorted [id1,...,idN,freq] array, into one idngram file. For those id1..idN which appear in more than one files, only one item appear in the final file, and its freq are sumed.\n");
    printf("\nOptions:\n");
    printf("\t  -n N           # N-gram\n");
    printf("\t  -o outputfile  # finale merged idngram file\n");
    printf("\nExample:\n");
    printf("   Following example merge 2 id3gram files into a large one:\n");
    printf(
        "\tidngram_merge -n3 -o all.id3gram first.id3gram second.id3gram\n\n");
}

static void
getParameters(int argc, char* const argv[])
{
    int option_index = 0;
    int c;
    while ((c =
                getopt_long(argc, argv, "n:o:", long_options,
                            &option_index)) != -1) {
        switch (c) {
        case 'n':
            N = atoi(strdup(optarg));
            break;
        case 'o':
            output = strdup(optarg);
            break;
        default:
            ShowUsage();
            exit(1000);
        }
    }
    if (N < 1 || N > 3 || output == NULL) {
        ShowUsage();
        exit(1000);
    }
}

int
main(int argc, char* argv[])
{
    getParameters(argc, argv);
    FILE *out = fopen(output, "wb+");
    std::vector<FILE* > idngram_files;

    if (optind >= argc) ShowUsage();
    while (optind < argc) {
        printf("Open %s:...", argv[optind]);
        FILE *fp = fopen(argv[optind++], "rb");
        if (fp == NULL) {
            printf("error!\n");
            exit(200);
        }
        idngram_files.push_back(fp);
        printf("\n");
    }
    printf("Merging...");
    switch (N) {
    case 1:
        ProcessingIdngramMerge<1>(out, idngram_files);
        break;
    case 2:
        ProcessingIdngramMerge<2>(out, idngram_files);
        break;
    case 3:
        ProcessingIdngramMerge<3>(out, idngram_files);
        break;
    }
    printf("\n");
    fclose(out);
    for (size_t i = 0; i < idngram_files.size(); i++)
        fclose(idngram_files[i]);
    return 0;
}

// -*- indent-tabs-mode: nil -*- vim:et:ts=4

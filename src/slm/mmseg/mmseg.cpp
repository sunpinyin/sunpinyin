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
#include <unistd.h>
#include <locale.h>

#include "../sim_dict.h"
#include "../sim_sen.h"

static struct option long_options[] =
{
    {"dict", 1, 0, 'd'},
    {"format", 1, 0, 'f'},
    {"show-id", 0, 0, 'i'},
    {"s-tok", 1, 0, 's'},
    {"ambiguious-id", 1, 0, 'a'},
    {0, 0, 0, 0}
};

static char* s_strDictFile = NULL;
static bool s_bTextOut = false;
static bool s_bShowId = false;
static TSIMWordId s_iSTOKID = 10;
static TSIMWordId s_iAmbiID = 0;

static CSIMDict *s_dict = NULL;

static void
ShowUsage()
{
    fprintf(stderr, "\nUsage:\n");
    fprintf(stderr, "mmseg -d dict_file [-f (text|bin)] [-i] [-s STOK_ID] [-a AMBI_ID]\n\n");
    fprintf(stderr, "  -f --format:\n");
    fprintf(stderr, "    Output Format, can be 'text' or 'bin'. default 'bin'\n");
    fprintf(stderr, "    Normally, in text mode, word text are output, while in binary mode,\n");
    fprintf(stderr, "    binary short integer of the word-ids are written to stdout.\n");
    fprintf(stderr, "  -s --stok:\n");
    fprintf(stderr, "    Sentence token id. Default 10.\n");
    fprintf(stderr, "    It will be written to output in binary mode after every sentence.\n");
    fprintf(stderr, "  -i --show-id:\n");
    fprintf(stderr, "    Show Id info. Under text output format mode, attach id after known.\n");
    fprintf(stderr, "    words. If under binary mode, print id(s) in text.\n");
    fprintf(stderr, "  -a --ambiguious-id:\n");
    fprintf(stderr, "    Ambiguious means ABC => A BC or AB C. If specified (AMBI-ID != 0), \n");
    fprintf(stderr, "    The sequence ABC will not be segmented, in binary mode, the AMBI-ID \n");
    fprintf(stderr, "    is written out; in text mode, <ambi>ABC</ambi> will be output. Default \n");
    fprintf(stderr, "    is 0.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Notes:\n");
    fprintf(stderr, "  Under binary mode, consecutive id of 0 are merged into one 0.\n");
    fprintf(stderr, "  Under text mode, no space are inserted between unknown-words. \n");
    fprintf(stderr, "\n");
    fprintf(stderr, "\n");
    exit(1000);
}

static void
getParameters(int argc, char* argv[])
{
    int c;
    while ((c=getopt_long(argc, argv, "d:if:s:a:", long_options, NULL)) != -1)
    {
        switch (c) {
        case 'd':
            s_strDictFile = strdup(optarg);
            break;
        case 'i':
            s_bShowId = true;
            break;
        case 'f':
            s_bTextOut = (strcmp(optarg, "text") == 0);
            break;
        case 's':
            s_iSTOKID = atoi(optarg);
            break;
        case 'a':
            s_iAmbiID = atoi(optarg);
            break;
        default:
            ShowUsage();
            break;
        }
    }
    if (s_strDictFile == NULL)
        ShowUsage();
}

static void
output_stok(int& nWords)
{
    if (s_bShowId) {
        if (nWords > 0)
            printf(" ");
        printf("%d", unsigned(s_iSTOKID));
    } else {
        fwrite(&s_iSTOKID, sizeof(TSIMWordId), 1, stdout);
    }
    ++nWords;
}

static void
output(int len, const TWCHAR* p, TSIMWordId idprev, TSIMWordId idcur, int& nWords)
{
    static char mbword[1024];
    static TWCHAR wcword[1024];

    bool bRealGap = (idcur != SIM_ID_NOT_WORD || idprev != SIM_ID_NOT_WORD);
    if (s_bTextOut) {
        for (int i=0; i < len; ++i, ++p)
            wcword[i] = *p;
        wcword[len] = 0;
        WCSTOMBS(mbword, wcword, sizeof(mbword));
        if (bRealGap && idprev == SIM_ID_NOT_WORD)
            printf("(%d)", unsigned(idprev));
        if (bRealGap && (nWords > 0))
            printf(" ");
	(s_iAmbiID && idcur == s_iAmbiID)? printf ("<ambi>%s</ambi>", mbword): 
                                           printf("%s", mbword);
        if (s_bShowId && idcur != SIM_ID_NOT_WORD)
            printf("(%d)", unsigned(idcur));
    } else {
        if (bRealGap) {
            if (s_bShowId) {
                if (nWords > 0)
                    printf(" ");
                printf("%d", unsigned(idcur));
            } else
                fwrite(&idcur, sizeof(TSIMWordId), 1, stdout);
        }
    }
    if (bRealGap)
        ++nWords;
}

/**
* Return 最大交集歧义长度. For example, ABCDEF if ABC CD DEF are words.
* if return len > word_len, then ambiguious exists at word [p p+len)...
*/
int
getAmbiLen(const TWCHAR* p, int word_len)
{
    const CSIMDict::TState* pstate;

    for (int i=1; i<word_len && *(p+i) != WCH_NULL; ++i) {
        int len = s_dict->matchLongest(s_dict->getRoot(), pstate, p+i);
        if (word_len < i+len)
            word_len = i+len;
    }

    return word_len;
}

static bool
processSingleFile(FILE* fp, int &nWords, int &nAmbis)
{
    nWords = 0;
    nAmbis = 0;

    wstring sntnc;
    CSIMCharReader *pReader = new CSIMCharReader(fp);
    CSIMCharReader::iterator iter = pReader->begin();
    TSIMWordId idcur, idprev = s_iSTOKID;

    if (!s_bTextOut)
        output_stok(nWords);

    while (true){
        if (ReadSentence(sntnc, iter, false) == false)
            break;

        for (const TWCHAR *p = sntnc.c_str(); (*p); ) {

            const CSIMDict::TState* pstate;
            int len = s_dict->matchLongest(s_dict->getRoot(), pstate, p);
            if (len <= 0) {
                idcur = SIM_ID_NOT_WORD;
                len = 1;
            } else
                idcur = pstate->word_id;

            if (s_iAmbiID != WCH_NULL) {
                int ambiLen=getAmbiLen(p, len);
                if (ambiLen > len) {
                    len = ambiLen;
                    idcur = s_iAmbiID;
                    ++nAmbis;
                }
            }

            output(len, p, idprev, idcur, nWords);

            idprev = idcur;
            p += len;
        }

        if (!s_bTextOut) {
            output_stok(nWords);
            idprev = s_iSTOKID;
        }
    }

    fflush(stdout);
    return true;
}

int
main(int argc, char *argv[])
{
    int nWords, nAmbis;

    setlocale(LC_ALL, "");
    getParameters(argc, argv);
    argc -= optind;
    argv += optind;

    fprintf(stderr, "Loading lexicon..."); fflush(stderr);
    s_dict = new CSIMDict();
    if (!s_dict->parseText(s_strDictFile)) {
      fprintf(stderr, "fail\n"); fflush(stderr);
      exit(1001);
    }
    fprintf(stderr, "done"); fflush(stderr);

    if (argc == 0) {
        fprintf(stderr, "\nProcessing from stdin..."); fflush(stderr);
        processSingleFile(stdin, nWords, nAmbis);
        fprintf(stderr, "%d words, %d ambiguious. Done!\n", nWords, nAmbis); fflush(stderr);
    } else {
        for (int i=0; i < argc; ++i) {
            fprintf(stderr, "\nProcessing %s...", argv[i]); fflush(stderr);
            FILE *fp = fopen(argv[i], "r");
            if (fp != NULL) {
                processSingleFile(fp, nWords, nAmbis);
                fprintf(stderr, "@Offset %ld, %d words, %d ambiguious. Done!\n", ftell(fp), nWords, nAmbis); fflush(stderr);
            } else {
                fprintf(stderr, "Can not Open!!!!!!!\n"); fflush(stderr);
            }
            fclose(fp);
        }
    }

    s_dict->close();
    delete s_dict;
    s_dict = NULL;
    return 0;
}

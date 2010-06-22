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

#include <vector>
#include <map>
#include <algorithm>

#include "../sim_dict.h"
#include "../sim_sen.h"
#include "../slm.h"

static struct option long_options[] =
{
    {"dict", 1, 0, 'd'},
    {"format", 1, 0, 'f'},
    {"show-id", 0, 0, 'i'},
    {"s-tok", 1, 0, 's'},
    {"model", 1, 0, 'm'},
    {0, 0, 0, 0}
};

static char* s_strDictFile = NULL;
static char* s_strSlmFile = NULL;
static bool s_bTextOut = false;
static bool s_bShowId = false;
static TSIMWordId s_iSTOKID = 10;

static CSIMDict *s_dict = NULL;
static CThreadSlm *s_tslm = NULL;

static void
ShowUsage()
{
    fprintf(stderr, "\nUsage:\n");
    fprintf(stderr, "slmseg -d dict_file [-f (text|bin)] [-i] [-s STOK_ID] [-m lm_file]\n\n");
    fprintf(stderr, "  -f --format:\n");
    fprintf(stderr, "    Output Format, can be 'text' or 'bin'. default 'bin'\n");
    fprintf(stderr, "    Normally, in text mode, word text are output, while in binary mode,\n");
    fprintf(stderr, "    binary short integer of the word-ids are writed to stdout.\n");
    fprintf(stderr, "  -s --stok:\n");
    fprintf(stderr, "    Sentence token id. Default 10.\n");
    fprintf(stderr, "    It will be write to output in binary mode after every sentence.\n");
    fprintf(stderr, "  -i --show-id:\n");
    fprintf(stderr, "    Show Id info. Under text output format mode, Attach id after known-words.\n");
    fprintf(stderr, "                  Under binary mode, print id in text.\n");
    fprintf(stderr, "  -m --model:\n");
    fprintf(stderr, "    Language model file name");
    fprintf(stderr, "\n");
    fprintf(stderr, "Notes:\n");
    fprintf(stderr, "  Under binary mode, consecutive id of 0 are merged into one 0.\n");
    fprintf(stderr, "  Under text mode, no space are insert between unknown-words. \n");
    fprintf(stderr, "\n");
    fprintf(stderr, "\n");
    exit(1000);
}

static void
getParameters(int argc, char* argv[])
{
    int c;
    while ((c=getopt_long(argc, argv, "d:if:s:m:", long_options, NULL)) != -1)
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
        case 'm':
            s_strSlmFile  = strdup(optarg);
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

struct TLatticeWord {
  int m_left;
  int m_right;
  int m_wordId;

  TLatticeWord(int left=0, int right=0, int wid=0)
      : m_left(left), m_right(right), m_wordId(wid) { }
};

typedef std::vector<TLatticeWord> TLatticeWordVec;

struct TLatticeStateValue {
  double                m_pr;
  TLatticeWord*         mp_btword;
  CThreadSlm::TState    m_btstate;

  TLatticeStateValue(double pr=0.0, TLatticeWord* btword=NULL, CThreadSlm::TState btstate = CThreadSlm::TState())
      : m_pr(pr), mp_btword(btword), m_btstate(btstate) { }
};

typedef std::map<CThreadSlm::TState, TLatticeStateValue> TLatticeColumnStates;

struct TLatticeColumn {
  TLatticeWordVec          m_wordstarting;
  TLatticeColumnStates     m_states;
};

typedef std::vector<TLatticeColumn> CLattice;

inline void insertLatticeWord(CLattice& lattice, TLatticeWord word)
{
    lattice[word.m_left].m_wordstarting.push_back(word);
}

int
getAmbiLen(const TWCHAR* p, int word_len)
{
    const CSIMDict::TState* pstate;

    for (int i=1; (i<word_len) && *(p+i) != WCH_NULL; ++i) {
        int len = s_dict->matchLongest(s_dict->getRoot(), pstate, p+i);
        if (word_len < i+len)
            word_len = i+len;
    }

    return word_len;
}

void fullSegBuildLattice(wstring& sntnc, int left, int len, CLattice& lattice)
{
    for (int right=left+len; left < right; ++left) {
        bool found = false;

        const TWCHAR* p = sntnc.c_str()+left;
        const CSIMDict::TState* pds = s_dict->getRoot();
        for (len = 0; left+len < right; ++len) {
            if ((pds = s_dict->step(pds, *p++)) == NULL)
                break;
            if (pds->word_id != SIM_ID_NOT_WORD) {
                found = true;
                insertLatticeWord(lattice, TLatticeWord(left, left+len+1, pds->word_id));
            }
        }
        if (!found)
            insertLatticeWord(lattice, TLatticeWord(left, left+1, SIM_ID_NOT_WORD));
    }
}

/**
* Lattice head should have one state, with its TState using slm's root. its
* pr = 0 and its mp_btword == NULL;
* Lattice tail must contain no word, and it previous node contain only one word
* with its right = left+1, right == tail.
* The lattice should ensure the lattice path existing
*/
void buildLattice(wstring &sntnc, CLattice& lattice)
{
    lattice.clear();
    lattice.resize(sntnc.size()+2);

    unsigned int idcur = SIM_ID_NOT_WORD;
    lattice[0].m_states[CThreadSlm::TState()] = TLatticeStateValue(0.0, NULL, CThreadSlm::TState());

    for (int i=0, sz=sntnc.size(); i < sz; ) {
        const CSIMDict::TState* pstate;
        const TWCHAR* p = sntnc.c_str()+i;
        int len = s_dict->matchLongest(s_dict->getRoot(), pstate, p);
        if (len <= 0) {
            idcur = SIM_ID_NOT_WORD;
            len = 1;
        } else {
            idcur = pstate->word_id;
        }
        int ambilen = getAmbiLen(p, len);

        if (ambilen <= len) {
            insertLatticeWord(lattice, TLatticeWord(i, i+len, idcur));
            i += len;
        } else {
            fullSegBuildLattice(sntnc, i, ambilen, lattice);
            i += ambilen;
        }
    }
    lattice[sntnc.size()].m_wordstarting.push_back(TLatticeWord(sntnc.size(), sntnc.size()+1, s_iSTOKID));
}

void searchBest(CLattice& lattice)
{
    for (int i=0, sz=lattice.size(); i < sz; ++i) {
        TLatticeColumnStates & states = lattice[i].m_states;
        TLatticeColumnStates::iterator itss = states.begin();
        TLatticeColumnStates::iterator itse = states.end();
        for (; itss != itse; ++itss) {
            TLatticeWordVec::iterator itws = lattice[i].m_wordstarting.begin();
            TLatticeWordVec::iterator itwe = lattice[i].m_wordstarting.end();
            for (; itws != itwe; ++itws) {
                CThreadSlm::TState his = itss->first;
                double pr = itss->second.m_pr;
                pr += s_tslm->transferNegLog(his, itws->m_wordId, his);
                TLatticeColumnStates & rss = lattice[itws->m_right].m_states;
                s_tslm->historify(his);
                TLatticeColumnStates::iterator itn = rss.find(his);
                if (itn == rss.end()) {
                    rss[his] = TLatticeStateValue(pr, &(*itws), itss->first);
                } else {
                    if (itn->second.m_pr > pr) {
                        rss[his] = TLatticeStateValue(pr, &(*itws), itss->first);
                    }
                }
            }
        }
    }
}

void getBestPath(CLattice& lattice, TLatticeWordVec& segResult)
{
    TLatticeColumnStates & states = lattice.back().m_states;
    TLatticeColumnStates::iterator its = states.begin();

    TLatticeWord* pbtword = its->second.mp_btword;
    CThreadSlm::TState btstate = its->second.m_btstate;
    its = lattice[pbtword->m_left].m_states.find(btstate);
    assert(its != lattice[pbtword->m_left].m_states.end());

    segResult.clear();
    while (true) {
        pbtword = its->second.mp_btword;
        if (pbtword != NULL) {
#ifndef HOST_OS_GNUC_2
            segResult.push_back(*pbtword);
#else // HOST_OS_GNUC_2
            segResult.insert(segResult.begin(), *pbtword);
#endif // !HOST_OS_GNUC_2
            btstate = its->second.m_btstate;
            its = lattice[pbtword->m_left].m_states.find(btstate);
            assert(its != lattice[pbtword->m_left].m_states.end());
        } else {
            break;
        }
    }
#ifndef HOST_OS_GNUC_2
    std::reverse(segResult.begin(), segResult.end());
#endif // HOST_OS_GNUC_2
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

        CLattice lattice;
        buildLattice(sntnc, lattice);
        searchBest(lattice);

        TLatticeWordVec segResult;
        getBestPath(lattice, segResult);

        for (int i=0, sz=segResult.size(); i < sz; ++i) {
            const TWCHAR *p = sntnc.c_str()+segResult[i].m_left;
            int len = segResult[i].m_right - segResult[i].m_left;
            idcur = segResult[i].m_wordId;

            output(len, p, idprev, idcur, nWords);
            idprev = idcur;
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

    fprintf(stderr, "Loading lexicon...");
    fflush(stderr);
    s_dict = new CSIMDict();
    s_tslm = new CThreadSlm();
    if (!s_dict->parseText(s_strDictFile)) {
        fprintf(stderr, "fail to open Lexicon file!\n");
        fflush(stderr);
        exit(11);
    }
    if (!s_tslm->load(s_strSlmFile, true)) {
        fprintf(stderr, "fail to open slm file!\n");
        fflush(stderr);
        exit(12);
    }
    fprintf(stderr, "done");
    fflush(stderr);

    if (argc == 0) {
        fprintf(stderr, "\nProcessing from stdin...");
        fflush(stderr);
        processSingleFile(stdin, nWords, nAmbis);
        fprintf(stderr, "%d words, %d ambiguious. Done!\n", nWords, nAmbis);
        fflush(stderr);
    } else {
        for (int i=0; i < argc; ++i) {
            fprintf(stderr, "\nProcessing %s...", argv[i]); fflush(stderr);
            FILE *fp = fopen(argv[i], "r");
            if (fp != NULL) {
                processSingleFile(fp, nWords, nAmbis);
                fprintf(stderr, "@Offset %ld, %d words, %d ambiguious. Done!\n",
                                ftell(fp), nWords, nAmbis);
                fflush(stderr);
            } else {
                fprintf(stderr, "Can not Open!!!!!!!\n");
                fflush(stderr);
            }
            fclose(fp);
        }
    }

    s_tslm->free();
    delete s_tslm;
    s_tslm = NULL;
    s_dict->close();
    delete s_dict;
    s_dict = NULL;
    return 0;
}

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
#include <string>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>

#include <vector>
#include <map>

#include "../slm.h"

class CIterateThreadSlm : public CThreadSlm {
public:
    typedef std::vector<TState> iterator;

    int
    getLevelSize(int lvl)
    {
        return m_LevelSizes[lvl];
    }

    int
    getN()
    {
        return m_N;
    }

    bool
    beginLevel(int lvl, iterator& it);

    void
    next(iterator& it)
    {
        ++(it.back()); adjustIterator(it);
    }

    bool
    isEnd(iterator& it)
    {
        return (int) ((it.back().getIdx()) + 1)
            == getLevelSize(it.back().getLevel());
    }

    void*
    getNodePtr(TState s);

    double
    mapPr(unsigned int pr_idx, bool log_format) const
    {
        double val = m_prTable[pr_idx];
        if (log_format) {
            return (m_UseLogPr) ? (val) : (-log(val));
        } else {
            return (m_UseLogPr) ? (exp(-val)) : (val);
        }
    }

    double
    mapBow(unsigned int bow_idx, bool log_format) const
    {
        double val = m_bowTable[bow_idx];
        if (log_format) {
            return (m_UseLogPr) ? (val) : (-log(val));
        } else {
            return (m_UseLogPr) ? (exp(-val)) : (val);
        }
    }

protected:
    void
    adjustIterator(iterator& it);
};

bool
CIterateThreadSlm::beginLevel(int lvl, iterator& it)
{
    it.clear();
    if (lvl > (int) m_N) return false;
    for (int i = 0; i <= lvl; ++i)
        it.push_back(TState(i, 0));
    adjustIterator(it);
    return true;
}

void*
CIterateThreadSlm::getNodePtr(TState s)
{
    unsigned int lvl = s.getLevel();
    if (lvl == m_N) {
        return(((TLeaf*)m_Levels[lvl]) + s.getIdx());
    } else {
        return(((TNode*)m_Levels[lvl]) + s.getIdx());
    }
}

void
CIterateThreadSlm::adjustIterator(iterator& it)
{
//    if (!isEnd(it)) {
    for (int lvl = it.size() - 2; lvl >= 0; --lvl) {
        int sz = getLevelSize(lvl);
        unsigned child = (it[lvl + 1]).getIdx();
        while ((int) it[lvl].getIdx() < (sz - 1) &&
               (((TNode*)getNodePtr(it[lvl])) + 1)->ch() <= child) {
            ++(it[lvl]);
        }
    }
//    }
}

void
ShowUsage()
{
    printf("Usage:\n");
    printf("    tslminfo [options] threaded_slm_file\n");
    printf("\nDescription:\n");
    printf(
        "    tslminfo tell information of a threaded back-off language model 'threaded_slm_file'. It can also print the model to ARPA format.");
    printf(
        " When no options given, slminfo will only print number of items in each level of the language model.\n");
    printf("\nOptions:\n");
    printf("    -v             # Verbose mode, printing arpa format.\n");
    printf(
        "    -p             # Prefer normal probability instead of -log(Pr) which is default. Valid under -v option.\n");
    printf(
        "    -l dict_file   # Lexicon. Valid under -v option. Substitute the word-id with word-text in the output.\n");
    printf("\n");
    exit(100);
}

static bool verbose = false;
static char *lexicon_filename = NULL;
static bool use_log_pr = true;

static struct option long_options[] =
{
    { "verbose", 0, 0, 'v' },
    { "pr", 0, 0, 'p' },
    { "lexicon", 1, 0, 'l' },
    { 0, 0, 0, 0 }
};

static void
getParameters(int argc, char* argv[])
{
    int c, option_index = 0;
    while ((c =
                getopt_long(argc, argv, "vpl:", long_options,
                            &option_index)) != -1) {
        switch (c) {
        case 'v':
            verbose = true;
            break;
        case 'l':
            lexicon_filename = strdup(optarg);
            break;
        case 'p':
            use_log_pr = false;
            break;
        default:
            ShowUsage();
        }
    }
    if (use_log_pr == false && !verbose) ShowUsage();
    if (lexicon_filename != NULL && !verbose) ShowUsage();
    if (optind != argc - 1) ShowUsage();
}

typedef std::map<unsigned int, std::string> TReverseLexicon;


void
PrintARPA(CIterateThreadSlm& itslm,
          const char* lexicon_filename,
          bool use_log_pr)
{
    static unsigned int id;
    static char word[10240];

    TReverseLexicon* plexicon = NULL;
    if (lexicon_filename != NULL) {
        plexicon = new TReverseLexicon();
        FILE* f_lex = fopen(lexicon_filename, "r");
        if (f_lex == NULL) {
            fprintf(stderr, "Failed to open lexicon file %s: %s\n", lexicon_filename, strerror(errno));
            exit(EXIT_FAILURE);
        }
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
                for (id = 0; *p >= '0' && *p <= '9'; ++p)
                    id = 10 * id + (*p - '0');
                (*plexicon)[id] = std::string(word);
            }
        }
        fclose(f_lex);
    }

    CIterateThreadSlm::iterator it;
    for (int lvl = 0; lvl <= itslm.getN(); ++lvl) {
        printf("\\%d-gram\\%d\n", lvl, itslm.getLevelSize(lvl) - 1);
        for (itslm.beginLevel(lvl, it); !itslm.isEnd(it); itslm.next(it)) {
            for (int i = 1; i < lvl; ++i) {
                CIterateThreadSlm::TNode*pn =
                    (CIterateThreadSlm::TNode*)itslm.getNodePtr(it[i]);
                if (plexicon != NULL)
                    printf("%s ", (*plexicon)[pn->wid()].c_str());
                else
                    printf("%9d ", pn->wid());
            }
            if (lvl < itslm.getN()) {
                CIterateThreadSlm::TNode*pn =
                    (CIterateThreadSlm::TNode*)itslm.getNodePtr(it[lvl]);
                if (lvl > 0) {
                    if (plexicon != NULL)
                        printf("%s ", ((*plexicon)[pn->wid()]).c_str());
                    else
                        printf("%9d ", pn->wid());
                }

                double pr = itslm.mapPr(pn->pr(), use_log_pr);
                double bow = itslm.mapBow(pn->bow(), use_log_pr);
                printf("%16.12lf %16.12lf ", pr, bow);
                printf("(%1u,%u)\n", pn->bol(), pn->bon());
            } else {
                CIterateThreadSlm::TLeaf*pn =
                    (CIterateThreadSlm::TLeaf*)itslm.getNodePtr(it[lvl]);
                if (lvl > 0) {
                    if (plexicon != NULL)
                        printf("%s ", ((*plexicon)[pn->wid()]).c_str());
                    else
                        printf("%9d ", pn->wid());
                }

                double pr = itslm.mapPr(pn->pr(), use_log_pr);
                printf("%16.12lf ", pr);
                printf("(%1u,%u)\n", pn->bol(), pn->bon());
            }
        }
    }

    delete plexicon;
}


/**
 * tslminfo [-v] threaded_slm_file
 */
int
main(int argc, char* argv[])
{
    getParameters(argc, argv);

    CIterateThreadSlm itslm;

    if (itslm.load(argv[argc - 1], true)) {
        if (!verbose) {
            printf("Total %d level ngram: ", itslm.getN());
            for (int lvl = 1; lvl <= itslm.getN(); ++lvl)
                printf("%d ", itslm.getLevelSize(lvl) - 1);
            printf(
                (itslm.isUseLogPr()) ? " using -log(pr)\n" :
                " using direct pr\n");
        } else {
            PrintARPA(itslm, lexicon_filename, use_log_pr);
        }
        itslm.free();
        return 0;
    }
    return 100;
}

// -*- indent-tabs-mode: nil -*- vim:et:ts=4

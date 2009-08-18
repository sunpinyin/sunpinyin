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

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>

#include <vector>
#include <iostream>

#include "../slm.h"

using namespace std;

static void
showUsage()
{
    cerr << "Usage:\n";
    cerr << "getWordFreq [-s corpus_size] [-v] [-e] -m slm_file -l lexicon\n";
    cerr << "    default corpus_size is 300000000 if not given\n";
    cerr << "    -v means output other information after word and freq for each line\n";
    cerr << "    -e give format for ervin\n";
    exit(10);
}

static char* slm_file = NULL;
static char* lexicon_file = NULL;
static int   corpus_size = 300000000;
static bool  verbose = false;
static bool  ervin   = false;

static bool
getParameters(int argc, char* argv[])
{
    int ch;
    while ((ch = getopt(argc, argv, "m:l:s:ve")) != -1) {
        switch (ch) {
        case 'm':
            slm_file = strdup(optarg); break;
        case 'l':
            lexicon_file = strdup(optarg); break;
        case 's':
            corpus_size = atoi(optarg); break;
        case 'v':
            verbose = true; break;
        case 'e':
            ervin = true; break;
        default:
            return false;
        }
    }
    return (slm_file && lexicon_file && corpus_size > 10);
}

static char buf[8192];

static void
tagFile(FILE *fp, CThreadSlm& slm)
{
    int freq = 0;
    while (fgets(buf, sizeof(buf), fp) != NULL) {
        freq = 0;
        char* wrd = strtok(buf, "\n\r \t");
        char* idstr = strtok(NULL, "\n\r \t");
        char* info = strtok(NULL, "\n\r");
        if (wrd && idstr) {
            int id = atoi(idstr);
            if (id > 0) {
                CThreadSlm::TState st;
                double neglogpr = slm.transfer(st, (unsigned int)id, st);
                if (st.getLevel() == 1) {
                    freq = int(exp(-neglogpr) * corpus_size);
                }
            }
        }
        if (wrd) {
            if (ervin) {
                vector<char* > pyv;
                if (info) {
                    for (char *p = strtok(info, " \t\n\r"); p != NULL; p = strtok(NULL, " \t\n\t"))
                        pyv.push_back(p);
                }
                for (int i=0, sz=pyv.size(); i < sz; ++i) {
                    cout << wrd << " " << pyv[i] <<  " " << freq << "\n";
                }
            } else if (idstr && verbose) {
                cout << wrd << " " << idstr <<  " " << freq;
                if (info)
                    cout << " " << info;
                cout << "\n";
            } else {
                cout << wrd << " " << freq << "\n";
            }
        }
    }
}

int
main(int argc, char*argv[])
{
    if (!getParameters(argc, argv))
        showUsage();

    FILE *fp;
    CThreadSlm slm;
    if (slm.load(slm_file, true) && (fp = fopen(lexicon_file, "r")) != NULL) {
        tagFile(fp, slm);
        slm.free();
        fclose(fp);
        return 0;
    } else {
        slm.free();
        return 20;
    }
}





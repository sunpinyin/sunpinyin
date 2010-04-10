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

#include <locale.h>
#include <stdlib.h>

#include "pytrie.h"
#include "pytrie_gen.h"
#include "../slm/slm.h"
#include "trie_writer.h"

class CUnigramSorter : public CWordEvaluator{
public:
    virtual double
    getCost(unsigned int wid);

    virtual bool
    isSeen(unsigned int wid);

    bool
    open(const char* lm_file)
        { return m_Model.load(lm_file); }

    void
    close() { m_Model.free(); }

protected:

    CThreadSlm m_Model;
};

double
CUnigramSorter::getCost(unsigned int wid)
{
    CThreadSlm::TState st(0,0);
    return m_Model.transferNegLog(st, wid, st);
}

bool
CUnigramSorter::isSeen(unsigned int wid)
{
    CThreadSlm::TState st(0,0);
    double logpr = m_Model.transferNegLog(st, wid, st);
    //printf("    -log(pr(%d)) = %lf\n", wid, logpr);
    return (st.getLevel() == 1);
}

/**
 * This program is used to generate the PINYIN Lexicon. It
 * Only works on zh_CN.utf8 locale.\n
 * args:
 *    -# dictionary file, in utf8 encoding, line-based text file,
 *       each line looks like\n
 *       CCC  id  [pinyin'pinyin'pinyin]*
 *    -# output binary PINYIN Lexicon file name
 *    -# log file to print the generated PINYIN Lexicon
 *    -# language model to sort the words of each node
 */
void
ShowUsage(const char* progname)
{
    fprintf(stderr,
            "Usage:\n"
            "    %s -i lexicon_file -o result_file -l log_file -s slm_file [-e le|be]\n",
            progname);
    fprintf(stderr,
            "Description:\n"
            "    This program is used to generate the PINYIN Lexicon. It Only works on zh_CN.utf8 locale\n"
            "\n");
    exit(100);
}

int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "");

    const char* lexicon_file = NULL;
    const char* result_file = NULL;
    const char* log_file = NULL;
    const char* slm_file = NULL;
    int build_endian = get_host_endian();
    int opt;
    while ((opt = getopt(argc, argv, "i:o:l:s:e:")) != -1) {
        switch (opt) {
        case 'i':
            lexicon_file = optarg;
            break;
        case 'o':
            result_file = optarg;
        case 'l':
            log_file = optarg;
            break;
        case 's':
            slm_file = optarg;
            break;
        case 'e':
            build_endian = parse_endian(optarg);
            break;
        }
    }
    if (!lexicon_file || !result_file || !log_file || !slm_file ||
        build_endian == -1) {
        ShowUsage(argv[0]);
    }
    
    printf("Opening language model..."); fflush(stdout);
    CUnigramSorter srt;
    if (!srt.open(slm_file)) {
        printf("error!\n");
        return -1;
    }
    printf("done!\n"); fflush(stdout);

    CPinyinTrieMaker maker;

    maker.constructFromLexicon(lexicon_file);

    printf("Writing out..."); fflush(stdout);
    maker.write(result_file, &srt, get_host_endian() != build_endian);
    printf("done!\n"); fflush(stdout);

    srt.close();

    if (get_host_endian() != build_endian) {
        printf("host endian is different from build endian. "
               "log_file will not be written.\n");
        fflush(stdout);
        return 0;
    }
    
    printf("Printing the lexicon out to log_file..."); fflush(stdout);
    CPinyinTrie t;
    t.load(result_file);

    FILE *fp = fopen(log_file, "w");
    t.print(fp);
    fclose(fp);

    printf("done!\n");

    return 0;
}

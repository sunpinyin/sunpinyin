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

#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include "slm_file.h"
#include "writer.h"

void
ShowUsage(const char* progname)
{
    printf("Usage:\n");
    printf(
        "    %s [-v] [-e endian] [-i <input-lm-file>] [-o <output-lm-file>]\n",
        progname);
    printf("\n");
    printf("Description:\n");
    printf(
        "    %s converts the binary language model files used by SunPinyin from big-endian to small-endian or vice versa.\n",
        progname);
    printf("\nOptions:\n");
    printf(
        "    -v                   # print out the endian-ness of <input-lm-file>.\n");
    printf(
        "    -e endian            # the endian-ness of <output-lm-file>. It can be either \"be\" or \"le\".\n");
    printf("    -i <input-lm-file>   # input file name, e.g. lm_sc.t3g\n");
    printf(
        "    -o <output-lm-file>  # converted output file name. the endian-ness is of host by default.\n");

    exit(100);
}

void
showEndian(const CThreadSlmFile& slm_file)
{
    int endian = slm_file.getEndian();
    printf("%s\n", endian2str(endian));
}

int
convert(CThreadSlmFile& slm_file, const char* output, int endian)
{
    printf("converting from %s to %s ...",
           endian2str(slm_file.getEndian()),
           endian2str(endian));

    size_t nwritten = slm_file.save(output, endian);
    if (nwritten != slm_file.size()) {
        fprintf(stderr,
                "\nfailed to write %s. %zu/%zu bytes written.\n",
                output,
                nwritten,
                slm_file.size());
        return 1;
    }
    printf("done.\n");
    return 0;
}

int
main(int argc, char* argv[])
{
    int opt;
    int endian = CThreadSlmFile::getHostEndian();

    bool opt_info = false;
    const char* input = NULL;
    const char* output = NULL;
    while ((opt = getopt(argc, argv, "e:i:o:v")) != -1) {
        switch (opt) {
        case 'e':
            endian = parse_endian(optarg);
            break;
        case 'v':
            opt_info = true;
            break;
        case 'i':
            input = optarg;
            break;
        case 'o':
            output = optarg;
            break;
        }
    }

    CThreadSlmFile slm_file;
    if (!input) {
        ShowUsage(argv[0]);
    }
    if (!slm_file.load(input)) {
        fprintf(stderr, "failed to parse %s. corrupt file?\n", input);
        return 1;
    }
    if (opt_info) {
        showEndian(slm_file);
    }
    if (output) {
        if (endian == -1) {
            ShowUsage(argv[0]);
        } else {
            return convert(slm_file, output, endian);
        }
    }
    return 0;
}

// -*- indent-tabs-mode: nil -*- vim:et:ts=4

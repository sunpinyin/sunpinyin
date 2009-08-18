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
#include <stdlib.h>

#include "ValueCompress.h"


std::map<float, int> values;
std::map<float, int> mapper;
std::vector<float>   table;

int
main(int argc, char* argv[])
{
    srand(1024);
    values[0.0] = 0;
    values[1e-10] = 0;
    values[1e-9] = 0;
    values[1e-8] = 0;
    values[1e-7] = 0;
    values[1e-6] = 0;
    values[1e-5] = 0;
    values[1e-4] = 0;
    values[1e-3] = 0;
    values[1e-2] = 0;
    values[1e-1] = 0;
    values[3.0] = 1;
    values[4.0] = 1;
    values[5.0] = 1;
    for (int i = 4; i < 4*65536; ++i) {
        unsigned int r = rand();
        float v = 2.0*(float)r/(float)RAND_MAX;
        values[v] = int(1 + (1000.0*rand())/RAND_MAX);
    }

    CValueCompressor vc;
    vc(values, mapper, table, 4096);

    std::map<float, int>::iterator it, ite = mapper.end();
    for (it = mapper.begin(); it != ite; ++it) {
        printf("%d of %15.12f ---> %d ---> %15.12f\n", values[it->first], it->first, it->second, table[it->second]);
    }

    return 0;
}

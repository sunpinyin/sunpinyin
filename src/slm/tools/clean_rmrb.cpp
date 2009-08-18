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

// erase 0xff character
// return true for empty line(only space)
bool processline(unsigned char * buf)
{
        bool space = true;
        unsigned char *dst = buf;
        while (*buf != 0) {
                if ((unsigned int)*buf == 0xFF || (unsigned int)*buf == (unsigned int)'\n') {
                        ++buf; continue;
                }
                if ((unsigned int)*buf == 0x80 && (unsigned int)*(buf+1) == 0x20) {
                        ++buf; continue;
                }
                if (space && (unsigned int)*buf != (unsigned int)' ' && (unsigned int)*buf != (unsigned int)'\t')
                        space = false;
                *dst++ = *buf++;
        }
        *dst = *buf;
        return space;
}

int main(int argc, char *argv[])
{
        unsigned char buf[10240];
        for (int i=1; i < argc; ++i) {
                FILE *fp = fopen(argv[i], "r");
                while (fgets((char*)buf, sizeof(buf), fp) != NULL) {
                        bool emptyline = processline(buf);
                        if ((unsigned int)buf[0] == '#' || (unsigned int)buf[0] == '0')
                                continue;
                        if (emptyline)
                                printf("\n");
                        else
                                printf("%s", buf);
                }
                fclose(fp);
                printf("\n");
        }
}

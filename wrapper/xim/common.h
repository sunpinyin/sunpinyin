/*
 * Copyright (c) 2010 Mike Qin <mikeandmore@gmail.com>
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

#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>
#include <stdbool.h>

#ifdef __cplusplus
 # ifndef __BEGIN_DECLS
 # define __BEGIN_DECLS extern "C" {
 # endif
 # ifndef __END_DECLS
 # define __END_DECLS }
 # endif
#else
 # ifndef __BEGIN_DECLS
 # define __BEGIN_DECLS
 # endif
 # ifndef __END_DECLS
 # define __END_DECLS
 # endif
#endif


#ifdef __cplusplus
#define __EXPORT_API extern "C"
#else
#define __EXPORT_API
#endif

#ifdef LOG_ENABLED


#define __PRINT(label, ...)                             \
    do {                                                \
        fprintf(stderr, "[%s] %s at %s: ", label,       \
                __FILE__, __FUNCTION__);                \
        fprintf(stderr, __VA_ARGS__);                   \
        fprintf(stderr, "\n");                          \
    } while (0)                                         \

#define LOG(...) __PRINT("log",  __VA_ARGS__)
#define DEBUG(...) __PRINT("debug",  __VA_ARGS__)
#else
#define LOG(...) 
#define DEBUG(...)
#endif

#define XIM_VERSION "0.0.4"
#define XIM_PROGRAM_NAME "SunPinyin-XIM"
#define XIM_WEBSITE "http://mike.struct.cn/sunpinyin-xim"
#define XIM_COMMENTS "a XIM front-end for SunPinyin."


#endif /* _COMMON_H_ */

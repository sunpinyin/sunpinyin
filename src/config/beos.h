/*
 * Copyright (c) 2008 Anthony Lee <don.anthony.lee@gmail.com>
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

/*
 * Authors:
 *    Anthony Lee <don.anthony.lee@gmail.com>
 */

#ifndef ____HOST_OS_H____
#define ____HOST_OS_H____

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_ASSERT_H
#include <assert.h>
#endif

#include <stdio.h>
#include <math.h>

#define BEOS_OS 1

#ifndef WEOF
#define WEOF 0xFFFFFFFF
#endif /* !WEOF */

#if defined(__cplusplus) && !defined(__HAIKU__)
inline unsigned int fgetwc(FILE *fp)
{
    unsigned int v1, v2;

    if((v1 = fgetc(fp)) == EOF || (v2 = fgetc(fp)) == EOF) return WEOF;

#ifdef __INTEL__
    return (v1 | (v2 << 8));
#else
    return ((v1 << 8) | v2);
#endif
}
#endif /* __cplusplus && !__HAIKU__ */

#ifndef HAVE_LOG2
    inline double log2(double value)    { return (log(v) / M_LN2); }
#else
    extern "C" double log2(double);
#endif /* !HAVE_LOG2 */

#ifndef HAVE_EXP2
    inline double exp2(double value)    { return pow(2, v); }
#else
    extern "C" double exp2(double);
#endif /* !HAVE_EXP2 */

#ifdef HAVE_ICONV_H
    #ifndef __HAIKU__
        typedef const char* TIConvSrcPtr;
    #else /* __HAIKU__ */
        typedef char* TIConvSrcPtr;
    #endif /* !__HAIKU__ */
#endif /* HAVE_ICONV_H */

#ifdef __GNUC__
    #if __GNUC__ < 3
        #define HOST_OS_GNUC_2
    #endif /* __GNUC__ < 3 */
#endif /* __GNUC__ */

#if defined(HOST_OS_GNUC_2) && defined(__cplusplus)
#include <iterator>
#include <heap.h>
#endif /* HOST_OS_GNUC_2 && __cplusplus */

#endif /* ____HOST_OS_H____ */

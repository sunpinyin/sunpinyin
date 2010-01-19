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
#include <config.h>
#endif

#ifdef HAVE_ICONV_H
#include <iconv.h>
#else
#include "portability.h"
#endif // HAVE_ICONV_H

#include "imi_data.h"

void
print_wide(const TWCHAR* wstr)
{
    char buf[512];

#ifdef HAVE_ICONV_H
    iconv_t icv = iconv_open("UTF-8", TWCHAR_ICONV_NAME);
    TIConvSrcPtr src = (TIConvSrcPtr)wstr;
    size_t srclen = (WCSLEN(wstr)+1)*sizeof(TWCHAR);
    char *dst = buf;
    size_t dstlen = 1024;
    iconv(icv, &src, &srclen, &dst, &dstlen);
    iconv_close(icv);
#else // !HAVE_ICONV_H
    memset(&buf[0], 0, sizeof(buf));
    WCSTOMBS(&buf[0], wstr, sizeof(buf) - 1);
#endif // HAVE_ICONV_H

    printf("%s", buf);
}

bool
CIMIData::loadResource(const char* lm_file_path, const char* pytrie_file_path)
{
    clear();

    #ifdef DEBUG
        printf("\n    openning lm file %s, pytrie file %s...", lm_file_path, pytrie_file_path);
    #endif

    if ((m_pLM = new CThreadSlm()) != NULL && m_pLM->load(lm_file_path, true) &&
        (m_pPinyinTrie = new CPinyinTrie()) != NULL && m_pPinyinTrie->load(pytrie_file_path)) {
        
        #ifdef DEBUG
            printf("done! LM @%p, Trie @%p\n", m_pLM, m_pPinyinTrie);
            fflush(stdout);
        #endif

        return true;
    }

    clear();

    #ifdef DEBUG
        printf("fail!\n");
        fflush(stdout);
    #endif

    return false;
}

void
CIMIData::clear()
{
    delete m_pLM;
    delete m_pPinyinTrie;

    m_pPinyinTrie = NULL;
    m_pLM = NULL;
}

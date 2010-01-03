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

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_SYS_MMAN_H
#include <sys/mman.h>
#endif

#include "datrie.h"

template <typename T, encoder_func_ptr encoder>
bool CDATrie<T, encoder>::load (const char * fname)
{
    free ();

    bool suc = false;
    int fd =  open (fname, O_RDONLY);
    if (fd == -1) return false;

    m_memSize = lseek (fd, 0, SEEK_END);
    lseek (fd, 0, SEEK_SET);

#ifdef HAVE_SYS_MMAN_H
    suc = (m_mem = (char*) mmap (NULL, m_memSize, PROT_READ, MAP_SHARED, fd, 0)) != MAP_FAILED;
#else
    suc = (m_mem = new char [m_memSize]) != NULL;
    suc = suc && (read (fd, m_mem, m_memSize) > 0);
#endif /* HAVE_SYS_MMAN_H */
    close (fd);

    if (!suc)
        return suc;

    m_len = * ((unsigned *) m_mem);
    unsigned short elm_size  = * ((unsigned short*) (m_mem + sizeof(m_len)));
    unsigned short has_value = * ((unsigned short*) (m_mem + sizeof(m_len) + sizeof(elm_size)));

    if (sizeof (T) != elm_size)
        return false;

    m_base = (T *) (m_mem + sizeof(m_len) + sizeof(elm_size) + sizeof(has_value));
    m_check = m_base + m_len;
    m_value = has_value? (int *)(m_check + m_len): NULL;

    return suc;
}

template <typename T, encoder_func_ptr encoder>
void CDATrie<T, encoder>::free ()
{
    if (m_mem) {
#ifdef HAVE_SYS_MMAN_H
        munmap (m_mem, m_memSize);
#else
        delete [] m_mem;
#endif
        m_mem = NULL;
    }

    m_len = 0;
    m_base = m_check = NULL;
    m_value = NULL;
}

template <typename T, encoder_func_ptr encoder>
unsigned CDATrie<T, encoder>::walk (unsigned s, unsigned ch, int &v)
{
    unsigned c = encoder(ch);
    unsigned t = abs (m_base[s]) + c;
        
    if (t < m_len && m_check[t] == (T)s && m_base[t]) {
        if (m_value)
            v = m_value[t];
        else
            v = m_base[t] < 0? -1: 0;

        return t;
    }

    v = 0;
    return 0;
}

template <typename T, encoder_func_ptr encoder>
int CDATrie<T, encoder>::match_longest (const char *str, unsigned &length)
{
    return match_longest (str, str+strlen(str), length);
}

template <typename T, encoder_func_ptr encoder>
int CDATrie<T, encoder>::match_longest (wstring wstr, unsigned &length)
{
    return match_longest (wstr.begin(), wstr.end(), length);
}

template <typename T, encoder_func_ptr encoder>
template <typename InputIterator>
int CDATrie<T, encoder>::match_longest (InputIterator first, InputIterator last, unsigned &length)
{
    int l=0, ret_v=0, curr_state=0;
    length = 0;

    for (; first != last; ++first) {
        unsigned ch = *first;
        int val;
        curr_state = walk (curr_state, ch, val);
        if (!curr_state) break;

        l += 1;
        if (val) {
            length = l;
            ret_v = val;
        }
    }

    return ret_v;
}

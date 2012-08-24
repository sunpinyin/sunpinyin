// -*- mode: c++ -*-
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

/*
 * convert the threaded lm binary file from big-endian to small-endian or vice versa
 */
#ifndef SLM_FILE_H
#define SLM_FILE_H

#include <stdint.h>

#include "../slm.h"

// TODO: may need consolidate this class with CThreadSlm
class CThreadSlmFile
{
public:
    CThreadSlmFile();
    ~CThreadSlmFile();
    /**
     * read from a lm_sc.t3g file and convert it to host byte order
     * @param fname the filenam
     * @return true if the given file is read successfully, false otherwise
     */
    bool load(const char* fname);
    /**
     * write the language model to a given file
     * @param fname filenam of output file
     * @param endian should be BIG_ENDIAN or LITTLE_ENDIAN
     * @return the number of bytes wrote
     */
    size_t save(const char* fname, int endian);
    /**
     * return the size of the whole file
     * @return size in byte
     */
    size_t size() const;
    int getEndian() const;
    static int getHostEndian();

private:
    CThreadSlmFile(const CThreadSlmFile&);

private:
    size_t m_bufLen;
    char*     m_buf;
    uint32_t m_N;
    uint32_t m_usingLogPr;
    uint32_t* m_levelSizes;
    float*    m_prTable;
    float*    m_bowTable;

    size_t m_nnode;
    CThreadSlm::TNode* m_nodes;
    size_t m_nleaf;
    CThreadSlm::TLeaf* m_leafs;
};

#endif //SLM_FILE_H

// -*- indent-tabs-mode: nil -*- vim:et:ts=4

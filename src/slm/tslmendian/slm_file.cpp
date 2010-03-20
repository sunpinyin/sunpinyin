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
 * the layout of binary file
 * - N : uin23_t
 * - usingLogPr : uint32_t
 * - slm.getLevelSize(0..N) : uint32_t * N
 * - pr_table() : float * 1<<CThreadSlm::BITS_PR    # padding with 0.0F
 * - bow_table() : float * 1 <<CThreadSlm::BITS_BOW # padding with 0.0F
 * - node[0][] : CThreadSlm::TNode * slm.getLevelSize(0)
 * - node[1][] : CThreadSlm::TNode * slm.getLevelSize(1)
 * - ...
 * - node[N-1][] : CThreadSlm::TNode * slm.getLevelSize(N-1)
 * - leaf[N-1][] : CThreadSlm::TLeaf * slm.getLevelSize(N)
 */

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <numeric>
#include <algorithm>
#include <iostream>
#include <cassert>


#include "slm_file.h"
#include "writer.h"

using namespace std;

// for convenience
typedef CThreadSlm::TNode TNode;
typedef CThreadSlm::TLeaf TLeaf;

// byte order reversed CThreadSlm::TLeaf small-endian presentation on big-endian machine
// 5-bits padding before m_pr_lo so that m_pr_lo is 4-byte aligned
// 
// would be easier if m_bon is 28 bits long
struct Leaf_BE
{
    unsigned m_wid       :18;
    unsigned m_pr_lo     :14;
    unsigned padding     : 5;
    unsigned m_bon       :23;
    unsigned m_bol       : 2;
    unsigned m_pr_hi     : 2;
};
 
CThreadSlm::TLeaf leaf_betole(const CThreadSlm::TLeaf& v)
{
   // since the CThreadSlm::TLeaf `protects' its bits, we duplicated a `public' one
    struct Leaf_LE_
    {
        unsigned m_wid       :18;
        unsigned m_pr_lo     :14;
        unsigned m_bon       :23;
        unsigned m_bol       : 2;
        unsigned m_pr_hi     : 2;
    };
    
    Leaf_BE be_leaf = change_byte_order(*(Leaf_BE*)(&v));
    be_leaf.padding = 0;
    
    TLeaf leaf;
    Leaf_LE_* le_leaf = (Leaf_LE_*)(&leaf);
    le_leaf->m_wid   = be_leaf.m_wid;
    le_leaf->m_pr_lo = be_leaf.m_pr_lo;
    le_leaf->m_bon   = be_leaf.m_bon;
    le_leaf->m_bol   = be_leaf.m_bol;
    le_leaf->m_pr_hi = be_leaf.m_pr_hi;
    return leaf;
}

// byte order reversed little-endian presentation
// 5-bits padding at end of struct
struct Leaf_LE_
{
    unsigned padding     : 5;
    unsigned m_pr_hi     : 2;
    unsigned m_bol       : 2;
    unsigned m_bon       :23;
    unsigned m_pr_lo     :14;
    unsigned m_wid       :18;
};

TLeaf leaf_letobe(const TLeaf& v)
{
    // since the TLeaf `protects' its bits, we duplicated a `public' one
    struct Leaf_BE_
    {
        unsigned m_pr_hi     : 2;
        unsigned m_bol       : 2;
        unsigned m_bon       :23;
        unsigned m_pr_lo     :14;
        unsigned m_wid       :18;
    };

    
    Leaf_LE_ le_leaf = change_byte_order(*(Leaf_LE_*)(&v));
    le_leaf.padding = 0;
    
    TLeaf leaf;
    Leaf_BE_* be_leaf = (Leaf_BE_*)(&leaf);
    be_leaf->m_wid   = le_leaf.m_wid;
    be_leaf->m_pr_lo = le_leaf.m_pr_lo;
    be_leaf->m_bon   = le_leaf.m_bon;
    be_leaf->m_bol   = le_leaf.m_bol;
    be_leaf->m_pr_hi = le_leaf.m_pr_hi;
    return leaf;
}

template<typename Type>
Type read_value(char*& buf, bool do_swap)
{
    Type v = *(Type*)buf;
    if (do_swap) {
        v = change_byte_order(v);
    }
    buf += sizeof(v);
    return v;
}

// change the byte order of all elements in the array in place
template<typename Type>
Type* read_values(char*& buf, size_t len, bool do_swap)
{
    Type* begin = (Type*)buf;
    Type* end = begin + len;
    
    if (do_swap) {
        for (Type* p = begin; p != end; ++p) {
            *p = change_byte_order(*p);
        }
    }
    buf = (char*)end;
    return begin;
}


typedef TLeaf(*convert_func_t)(const TLeaf&);

template<>
TLeaf* read_values<TLeaf>(char*& buf, size_t len, bool do_swap)
{
    TLeaf* begin = (TLeaf*)buf;
    TLeaf* end = begin + len;    
    convert_func_t convert = (CThreadSlmFile::getHostEndian() == BIG_ENDIAN) ? leaf_letobe : leaf_betole;
    
    if (do_swap) {
        for (TLeaf* p = begin; p != end; ++p) {
            *p = convert(*p);
        }
    }
    buf = (char*)end;
    return begin;
}

template<typename Type>
size_t write_value(FILE* fp, const Type& value, bool do_swap)
{
    Type v = value;
    if (do_swap) {
        v = change_byte_order(v);
    }
    return fwrite(&v, sizeof(v), 1, fp)*sizeof(v);
}

template<typename Type>
size_t write_values(FILE* fp, Type* const begin, size_t len, bool do_swap)
{
    const Type* end = begin + len;
    if (do_swap) {
        for (Type* p = begin; p != end; ++p) {
            *p = change_byte_order(*p);
        }
    }
    return fwrite(begin, sizeof(Type), end-begin, fp) * sizeof(Type);
}

template<>
size_t write_values<TLeaf>(FILE* fp, TLeaf* const begin, size_t len, bool do_swap)
{
    const TLeaf* end = begin + len;
    convert_func_t convert = (CThreadSlmFile::getHostEndian() == BIG_ENDIAN) ? leaf_betole : leaf_letobe;
    if (do_swap) {
        for (TLeaf* p = begin; p != end; ++p) {
            *p = convert(*p);
        }
    }
    return fwrite(begin, sizeof(TLeaf), end-begin, fp) * sizeof(TLeaf);
}

CThreadSlmFile::CThreadSlmFile()
    : m_buf(NULL),
      m_N(0), m_usingLogPr(0), m_levelSizes(NULL), m_prTable(NULL), m_bowTable(NULL),
      m_nnode(0), m_nodes(NULL), m_nleaf(0), m_leafs(NULL)
{}

// TODO: may need consolidate this class with CThreadSlm
bool CThreadSlmFile::load(const char* fname)
{
    assert(m_buf == NULL);
    
    int fd = open(fname, O_RDONLY);
    ssize_t len = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);
    m_buf = new char[len];
    if (read(fd, m_buf, len) != len) {
        delete [] m_buf;
        m_buf = NULL;
        cerr << "Failed to read from " << fname << endl;
        return false;
    }
    close(fd);

    char* buf = m_buf;
    bool do_swap = (getEndian() != getHostEndian());

    m_N          = read_value<uint32_t>(buf, do_swap);
    m_usingLogPr = read_value<uint32_t>(buf, do_swap);

    m_levelSizes = read_values<uint32_t>(buf, m_N+1, do_swap);
    m_prTable    = read_values<float>(buf, 1 << CThreadSlm::BITS_PR, do_swap);
    m_bowTable   = read_values<float>(buf, 1 << CThreadSlm::BITS_BOW, do_swap);
    
    m_nnode = accumulate(m_levelSizes, m_levelSizes+m_N, 0);
    m_nodes = read_values<TNode>(buf, m_nnode, do_swap);
    m_nleaf = m_levelSizes[m_N];
    m_leafs = read_values<TLeaf>(buf, m_nleaf, do_swap);

    m_bufLen = len;
    return (m_buf + len == buf);
}

size_t CThreadSlmFile::save(const char* fname, int endian)
{
    bool do_swap = (endian != getHostEndian());
    FILE* fp = fopen(fname, "wb");
    size_t nwrite = 0;
    nwrite += write_value(fp, m_N, do_swap);
    nwrite += write_value(fp, m_usingLogPr, do_swap);
    nwrite += write_values(fp, m_levelSizes, m_N+1, do_swap);
    nwrite += write_values(fp, m_prTable, 1 << CThreadSlm::BITS_PR, do_swap);
    nwrite += write_values(fp, m_bowTable, 1 << CThreadSlm::BITS_BOW, do_swap);
    nwrite += write_values(fp, m_nodes, m_nnode, do_swap);
    nwrite += write_values(fp, m_leafs, m_nleaf, do_swap);
    fclose(fp);
    return nwrite;
}

size_t CThreadSlmFile::size() const
{
    return m_bufLen;
}

CThreadSlmFile::~CThreadSlmFile()
{
    delete [] m_buf;
    m_buf = NULL;
}

int CThreadSlmFile::getEndian() const
{
    assert(m_buf != NULL);
    // assuming the N of this lm is not larger than 0x0100 0000
    return (*(uint8_t*)m_buf) == 0 ? BIG_ENDIAN : LITTLE_ENDIAN;
}

int CThreadSlmFile::getHostEndian()
{
    return htons(0x0001) == 0x0100 ? LITTLE_ENDIAN : BIG_ENDIAN;
}


# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
# 
# Copyright (c) 2007 Sun Microsystems, Inc. All Rights Reserved.
# 
# The contents of this file are subject to the terms of either the GNU Lesser
# General Public License Version 2.1 only ("LGPL") or the Common Development and
# Distribution License ("CDDL")(collectively, the "License"). You may not use this
# file except in compliance with the License. You can obtain a copy of the CDDL at
# http://www.opensource.org/licenses/cddl1.php and a copy of the LGPLv2.1 at
# http://www.opensource.org/licenses/lgpl-license.php. See the License for the 
# specific language governing permissions and limitations under the License. When
# distributing the software, include this License Header Notice in each file and
# include the full text of the License in the License file as well as the
# following notice:
# 
# NOTICE PURSUANT TO SECTION 9 OF THE COMMON DEVELOPMENT AND DISTRIBUTION LICENSE
# (CDDL)
# For Covered Software in this distribution, this License shall be governed by the
# laws of the State of California (excluding conflict-of-law provisions).
# Any litigation relating to this License shall be subject to the jurisdiction of
# the Federal Courts of the Northern District of California and the state courts
# of the State of California, with venue lying in Santa Clara County, California.
# 
# Contributor(s):
# 
# If you wish your version of this file to be governed by only the CDDL or only
# the LGPL Version 2.1, indicate your decision by adding "[Contributor]" elects to
# include this software in this distribution under the [CDDL or LGPL Version 2.1]
# license." If you don't indicate a single choice of license, a recipient has the
# option to distribute your version of this file under either the CDDL or the LGPL
# Version 2.1, or to extend the choice of license to its licensees as provided
# above. However, if you add LGPL Version 2.1 code and therefore, elected the LGPL
# Version 2 license, then the option applies only if the new code is made subject
# to such option by the copyright holder. 

cdef extern from "Python.h":
    ctypedef struct PyUnicodeObject:
        pass
    ctypedef unsigned wchar_t
    ctypedef wchar_t * const_wchar_t_ptr "const wchar_t *"
    object PyUnicode_FromWideChar (wchar_t *, Py_ssize_t)
    Py_ssize_t PyUnicode_AsWideChar (PyUnicodeObject *, wchar_t *, Py_ssize_t)

cdef extern from "portability.h":
    ctypedef unsigned TWCHAR
    ctypedef TWCHAR * const_TWCHAR_ptr "const TWCHAR *"
    unsigned WCSLEN (const_TWCHAR_ptr ws)

cdef extern from "pytrie.h":
    ctypedef struct CPinyinTrie_TWord "CPinyinTrie::TWordIdInfo":
        bint     m_bSeen
        unsigned m_cost
        unsigned m_csLevel
        unsigned m_id
        unsigned m_len

    ctypedef struct CPinyinTrie_TNode "CPinyinTrie::TNode":
        unsigned m_nWordId
        CPinyinTrie_TWord * getWordIdPtr ()

    ctypedef struct CPinyinTrie "CPinyinTrie":
        bint load(char *filename)
        void free()
        int getWordCount ()
        CPinyinTrie_TNode * getRootNode ()
        CPinyinTrie_TNode * transfer (CPinyinTrie_TNode *, unsigned)
        const_TWCHAR_ptr getitem "operator []" (unsigned)
        int getSymbolId (const_TWCHAR_ptr)
        bint isValid (CPinyinTrie_TNode*, bint, unsigned)

    CPinyinTrie *new_CPinyinTrie "new CPinyinTrie" ()
    void del_CPinyinTrie "delete" (CPinyinTrie *pytrie)

cdef class WordInfo:
    cdef public bint seen
    cdef public int wid, cost, length, charset_level

    def __cinit__ (self, wid, seen=True, cost=0, length=0, charset_level=0):
        self.wid = wid
        self.seen = seen
        self.cost = cost
        self.length = length
        self.charset_level = charset_level

    def __str__ (self):
        return "wid=%d, seen=%d, cost=%d, length=%d, charset_level=%d" % \
               (self.wid, self.seen, self.cost, self.length, self.charset_level)

cdef class PinyinTrieNode:
    cdef CPinyinTrie_TNode *pnode

    def get_words (self):
        words = []
        cdef CPinyinTrie_TWord *p= <CPinyinTrie_TWord*> self.pnode.getWordIdPtr ()
        for i in xrange (self.pnode.m_nWordId):
            words.append (WordInfo(p[i].m_id, p[i].m_bSeen, p[i].m_cost, p[i].m_len, p[i].m_csLevel))
        return words 

cdef class PinyinTrie:
    cdef CPinyinTrie *thisptr

    def __cinit__ (self):
        self.thisptr = new_CPinyinTrie ()

    def __dealloc__ (self):
        del_CPinyinTrie (self.thisptr)

    def load (self, fname):
        return self.thisptr.load (fname)

    def free (self):
        self.thisptr.free ()

    def get_word_count (self):
        return self.thisptr.getWordCount()

    def get_root_node (self):
        cdef CPinyinTrie_TNode * pnode = <CPinyinTrie_TNode*> self.thisptr.getRootNode ()
        node = PinyinTrieNode ()
        (<PinyinTrieNode>node).pnode = pnode
        return node

    def transfer (self, node, syllable):
        cdef CPinyinTrie_TNode *pnode = (<PinyinTrieNode>node).pnode
        pnode = <CPinyinTrie_TNode*> self.thisptr.transfer (pnode, <int>syllable)
        if not pnode:
            return None

        node = PinyinTrieNode ()
        (<PinyinTrieNode>node).pnode = pnode
        return node

    def __getitem__ (self, idx):
        if idx<0 or idx>=self.thisptr.getWordCount(): return ''
        cdef const_TWCHAR_ptr cwstr = self.thisptr.getitem (idx)
        return PyUnicode_FromWideChar (<const_wchar_t_ptr>cwstr, WCSLEN(cwstr))

    def get_symbol_id (self, symbol):
        cdef wchar_t buf[2]
        if len (symbol) != 1: return 0
        PyUnicode_AsWideChar (<PyUnicodeObject*> symbol, buf, sizeof(buf))
        return self.thisptr.getSymbolId (<const_TWCHAR_ptr> buf)

    def is_valid (self, node, allowNonComplete=True, csLevel=0):
        cdef CPinyinTrie_TNode *pnode = (<PinyinTrieNode>node).pnode
        return self.thisptr.isValid (pnode, allowNonComplete, csLevel)

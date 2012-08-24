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

cdef extern from "slm.h":

    ctypedef union CThreadSlm_TState "CThreadSlm::TState":
        void setIdx (unsigned int idx)
        void setLevel (unsigned int level)
        unsigned int getLevel()
        unsigned int getIdx()

    ctypedef struct CThreadSlm "CThreadSlm":
        bint load(char *filename, bint MMap)
        bint isUseLogPr()
        void free()
        double transferNegLog (CThreadSlm_TState history, unsigned int wid, CThreadSlm_TState result)
        double transfer (CThreadSlm_TState history, unsigned int wid, CThreadSlm_TState result)
        CThreadSlm_TState history_state_of(CThreadSlm_TState st)
        CThreadSlm_TState historify(CThreadSlm_TState st)
        unsigned int lastWordId(CThreadSlm_TState st)

    CThreadSlm *new_CThreadSlm "new CThreadSlm" ()
    void del_CThreadSlm "delete" (CThreadSlm *slm)

cdef class SlmState:
    cdef public int level, idx
    def __cinit__(self, level=0, idx=0):
        self.level = level
        self.idx = idx

    def __richcmp__ (self, other, op):
        equal = self.level == other.level and self.idx == other.idx
        if   op == 2:   return equal
        elif op == 3:   return not equal
        else:           return NotImplemented

    def __hash__ (self):
        return ((self.level, self.idx)).__hash__()

    def __str__(self):
        return "[level=%d, idx=%d]" % (self.level, self.idx)

cdef inline pystate_to_cstate (state, CThreadSlm_TState *st):
    st.setLevel (state.level)
    st.setIdx (state.idx)

cdef class Slm:
    cdef CThreadSlm *thisptr

    def __cinit__(self):
        self.thisptr = new_CThreadSlm()

    def __dealloc__(self):
        del_CThreadSlm (self.thisptr)

    def load (self, fname):
        return self.thisptr.load(fname, True)

    def is_using_log_pr(self):
        return self.thisptr.isUseLogPr()

    def free(self):
        self.thisptr.free()

    def transfer_neglog(self, history, wid):
        cdef CThreadSlm_TState his, ret
        pystate_to_cstate (history, &his)
        pr = self.thisptr.transferNegLog(his, wid, ret)
        return pr, SlmState(ret.getLevel(), ret.getIdx())

    def transfer(self, history, wid):
        cdef CThreadSlm_TState his, ret
        pystate_to_cstate (history, &his)
        pr = self.thisptr.transfer(his, wid, ret)
        return pr, SlmState(ret.getLevel(), ret.getIdx())
    
    def history_state_of(self, state):
        cdef CThreadSlm_TState st, ret
        pystate_to_cstate (state, &st)
        ret = self.thisptr.history_state_of(st)
        return SlmState(ret.getLevel(), ret.getIdx())

    def historify (self, state):
        cdef CThreadSlm_TState st
        pystate_to_cstate (state, &st)
        self.thisptr.historify(st)
        state.level = int(st.getLevel())
        state.idx = int(st.getIdx())

    def last_word_id (self, state):
        cdef CThreadSlm_TState st
        pystate_to_cstate (state, &st)
        return self.thisptr.lastWordId(st)

# -*- indent-tabs-mode: nil -*- vim:et:ts=4

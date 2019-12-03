#!/usr/bin/python3
# -*- coding: UTF-8 -*-

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

import os
import mmap
import struct
import heapq
import tempfile

class NGram:
    key = ()
    freq = 0

    def __init__(self, key, freq):
        self.key = key
        self.freq = freq

    def __cmp__(self,other):
        return cmp(self.key, other.key)

    def __str__(self):
        return "ngram: " + self.key.__str__() + " freq: " + str(self.freq)

def read_ch_sentences(file):
    buf = []
    for line in file:
        if buf and (line[0].isspace() or len(buf) <= 40):
            yield ''.join(buf)
            buf = []

        for ch in line:
            if ch.isspace() or ch == u'—':
                continue

            if ch in u"；。！？…—":
                if buf:
                    buf.append(ch)
                    yield ''.join (buf)
                    buf = []
            else:
                buf.append (ch)
    if buf:
        yield ''.join (buf)

def mergesort (iters):
        heap=[]

        for it in iters:
            try:
                heap.append((it.next(), it))
            except StopIteration:
                pass

        heapq.heapify(heap)

        while heap:
            val, it = heap[0]
            yield val

            try:
                heapq.heapreplace(heap, (it.next(),it))
            except StopIteration:
                heapq.heappop(heap)

def read_ngrams (fname, n):
    file = open(fname, "r")
    fsize = os.path.getsize(fname)
    mem = mmap.mmap(file.fileno(), fsize, mmap.MAP_SHARED, mmap.PROT_READ)

    while True:
        ngram = mem.read((n+1)*4)
        if ngram:
            data = struct.unpack('%dl' % (n+1), ngram)
            yield NGram(data[:n], data[n])
        else:
            break

    mem.close()
    file.close()

class MMArray:
    __file = __mem = None
    __realsize = __capsize = 0

    def __init__(self, elmsize=1, fname=None, capsize=1024*1024):
        self.__elmsize = elmsize

        if not fname:
            fno, self.__fname = tempfile.mkstemp("-mmarray", "pyslm-")
            self.__file = os.fdopen (fno, "w+")
            self.__enlarge(capsize)
        else:
            self.fromfile(fname)

    def fromfile(self, fname):
        if not os.path.exists(fname):
            raise "The file '%s' does not exist!"

        fsize = os.path.getsize(fname)
        if fsize == 0:
            raise "The size of file '%s' is zero!" % fname

        if self.__mem: self.__mem.close()
        if self.__file: self.__file.close()

        self.__file = open (fname, "r+")
        self.__mem = mmap.mmap(self.__file.fileno(), fsize)
        self.__realsize = self.__capsize = fsize/self.__elmsize

    def tofile(self, fname):
        if fname == self.__file.name:
            raise "Can not dump the array to currently mapping file!"
        tf = open(fname, "w+")
        bsize = self.__realsize * self.__elmsize
        tf.write (self.__mem[:bsize])
        tf.close()

    def __enlarge(self, capsize):
        if self.__capsize >= capsize:
            return
        
        self.__capsize = capsize
        self.__file.seek(self.__elmsize * self.__capsize - 1)
        self.__file.write('\0')
        self.__file.flush()

        if (self.__mem): self.__mem.close()
        self.__mem = mmap.mmap(self.__file.fileno(), self.__file.tell())

    def __del__ (self):
        bsize = self.__realsize * self.__elmsize
        self.__file.truncate (bsize)
        self.__file.close()
        if self.__mem: self.__mem.close()
        os.remove(self.__fname)

    def __getitem__(self, idx):
        if idx < -self.__realsize or idx >= self.__realsize:
            raise IndexError
        return self.__access(idx)

    def __setitem__(self, idx, buf):
        if idx < -self.__realsize or idx >= self.__realsize:
            raise IndexError
        if type(buf) != type("") or len(buf) != self.__elmsize:
            raise "Not a string, or the buffer size is incorrect!"
        self.__access(idx, buf)

    def __access (self, idx, buf=None):
        if idx < 0: idx = self.__realsize + idx
        start = idx * self.__elmsize
        end = start + self.__elmsize
        if not buf: return self.__mem[start:end]
        self.__mem[start:end] = buf

    def size(self):
        return self.__realsize

    def append(self, buf):
        if type(buf) != type("") or len(buf) != self.__elmsize:
            raise "Not a string, or the buffer size is incorrect!"

        if self.__realsize >= self.__capsize:
            self.__enlarge(self.__capsize*2)

        self.__access(self.__realsize, buf)
        self.__realsize += 1

    def __iter__(self):
        for i in range(0, self.__realsize):
            yield self.__access(i)

    def truncate(self, tsize):
        if self.__realsize >= tsize:
            self.__realsize = tsize

# -*- indent-tabs-mode: nil -*- vim:et:ts=4

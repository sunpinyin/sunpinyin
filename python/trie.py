#!/usr/bin/python

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

__all__ = ['Trie', 'DATrie', 'match_longest', 'get_ambiguious_length']

from math import log
import struct

class Trie (object):
    class TrieNode:
        def __init__ (self):
            self.val = 0
            self.trans = {}

    def __init__(self):
        self.root = Trie.TrieNode()

    def add(self, word, value=1):
        curr_node = self.root
        for ch in word:
            try: 
                curr_node = curr_node.trans[ch]
            except:
                curr_node.trans[ch] = Trie.TrieNode()
                curr_node = curr_node.trans[ch]

        curr_node.val = value

    def walk (self, trienode, ch):
        if ch in trienode.trans:
            trienode = trienode.trans[ch]
            return trienode, trienode.val
        else:
            return None, 0

class FlexibleList (list):
    def __check_size (self, index):
        if index >= len(self):
            self.extend ([0] * (index-len(self)+1))

    def __getitem__ (self, index):
        self.__check_size (index)
        return list.__getitem__(self, index)

    def __setitem__ (self, index, value):
        self.__check_size (index)
        return list.__setitem__(self, index, value)

def character_based_encoder (ch, range=('a', 'z')):
    ret = ord(ch) - ord(range[0]) + 1
    if ret <= 0: ret = ord(range[1]) + 1
    return ret

class DATrie (object):
    def __init__(self, chr_encoder=character_based_encoder):
        self.root = 0
        self.chr_encoder = chr_encoder
        self.clear()

    def clear (self):
        self.base  = FlexibleList ()
        self.check = FlexibleList ()
        self.value = FlexibleList ()

    def walk (self, s, ch):
        c = self.chr_encoder (ch)
        t = abs(self.base[s]) + c

        if t<len(self.check) and self.check[t] == s and self.base[t]:
            if self.value: 
                v = self.value[t]
            else: 
                v = -1 if self.base[t] < 0 else 0
            return t, v
        else:
            return 0, 0

    def find_base (self, s, children, i=1):
        if s == 0 or not children:
            return s

        i = max (i, 1)
        loop_times = 0
        while True:
            for ch in children:
                k = i + self.chr_encoder (ch)
                if self.base[k] or self.check[k] or k == s:
                    loop_times += 1
                    i += int (log (loop_times, 2)) + 1
                    break
            else:
                break

        return i

    def build (self, words, values=None):
        assert (not values or (len(words) == len(values)))
        itval = iter(values) if values else None

        trie = Trie()
        for w in words:
            trie.add (w, itval.next() if itval else -1)

        self.construct_from_trie (trie, values!=None)

    def construct_from_trie (self, trie, with_value=True, progress_cb=None, progress_cb_thr=100):
        nodes = [(trie.root, 0)]
        find_from = 1
        loop_times = 0

        while nodes:
            trienode, s = nodes.pop(0)
            find_from = b = self.find_base (s, trienode.trans, find_from)
            self.base[s] = -b if trienode.val else b
            if with_value: self.value[s] = trienode.val

            for ch in trienode.trans:
                c = self.chr_encoder (ch)
                t = abs(self.base[s]) + c
                self.check[t] = s if s else -1

                nodes.append ((trienode.trans[ch], t))

            loop_times += 1
            if loop_times == progress_cb_thr:
                loop_times = 0
                if progress_cb:
                    progress_cb ()

        for i in xrange (self.chr_encoder (max(trie.root.trans))+1):
            if self.check[i] == -1:
                self.check[i] = 0

    def save (self, fname):
        f = open (fname, 'w+')
        l = len (self.base)

        using_32bits = l > 2**15
        elm_size = 4 if using_32bits else 2
        fmt_str = '%di'%l if using_32bits else '%dh'%l

        # the data types here should be aligned with those in datrie.h
        f.write (struct.pack ('I', l))
        f.write (struct.pack ('H', elm_size))
        f.write (struct.pack ('H', 1 if self.value else 0))

        f.write (struct.pack (fmt_str, *self.base))
        f.write (struct.pack (fmt_str, *self.check))

        if self.value:
            if len(self.value) < l: self.value[l-1] = 0
            f.write (struct.pack ('%di'%l, *self.value))

        f.close()

    def output_static_c_arrays (self, fname):
        f = open(fname, 'w+')
        l = len (self.base)

        type = "int" if l > 2**15 else "short"

        f.write (self.__to_c_array (self.base,  type,  "base"))
        f.write (self.__to_c_array (self.check, type,  "check"))
        f.write (self.__to_c_array (self.value, "int", "value"))

        f.close()

    def __to_c_array (self, array, type, name):
        return "static %s %s[] = {%s};\n\n" % (type, name, ', '.join (str(i) for i in array))

    def load (self, fname):
        f = open (fname, 'r')

        l = struct.unpack ('I', f.read(4))[0]
        elm_size = struct.unpack ('H', f.read(2))[0]
        has_value = struct.unpack ('H', f.read(2))[0]

        fmt_str = '%di'%l if elm_size == 4 else '%dh'%l
        self.base  = struct.unpack (fmt_str, f.read(l*elm_size))
        self.check = struct.unpack (fmt_str, f.read(l*elm_size))
        self.value = struct.unpack ('%di'%l, f.read(l*4)) if has_value else []

        f.close()

def search (trie, word):
    curr_node = trie.root

    for ch in word:
        curr_node, val = trie.walk (curr_node, ch)
        if not curr_node: 
            break
    else:
        return val

    return 0

def match_longest (trie, word):
    l = ret_l = ret_v = 0
    curr_node = trie.root

    for ch in word:
        curr_node, val = trie.walk (curr_node, ch)
        if not curr_node: 
            break

        l += 1
        if val: 
            ret_l, ret_v = l, val

    return ret_v, ret_l

def get_ambiguious_length (trie, str, word_len):
    i = 1
    while i < word_len and i < len(str):
        wid, l = match_longest(trie, str[i:])
        if word_len < i + l:
            word_len = i + l
        i += 1
    return i

def test ():
    from pinyin_data import valid_syllables

    trie = Trie()
    for s in valid_syllables:
        trie.add (s, valid_syllables[s])

    for s in valid_syllables:
        v, l = match_longest (trie, s+'b')
        assert (len(s) == l and valid_syllables[s] == v)

    datrie = DATrie()
    datrie.construct_from_trie (trie)

    datrie.save ('/tmp/trie_test')
    datrie.load ('/tmp/trie_test')

    for s in valid_syllables:
        v, l = match_longest (datrie, s+'b')
        assert (len(s) == l and valid_syllables[s] == v)

    print 'test executed successfully'

if __name__ == "__main__":
    test ()

#!/usr/bin/python3
# -*- coding: UTF-8 -*-

from pyslm import Slm, SlmState
from pytrie import PinyinTrie, PinyinTrieNode, WordInfo

def test_pyslm ():
    slm = Slm ()
    if not slm.load ("../data/lm_sc.t3g"):
        return

    pr, result = slm.transfer (SlmState(0,0), 58614)
    print("pr =", pr, "\tresult = %s" % result)
    
    pr, result = slm.transfer (result, 75956)
    print("pr =", pr, "\tresult = %s" % result)
    
    pr, result = slm.transfer (result, 84582)
    print("pr =", pr, "\tresult = %s" % result)
    
    his = slm.history_state_of (result)
    print("his = %s" % his)
    
    slm.historify (result)
    print("result = %s" % result)

    print('last_word_id =', slm.last_word_id (result))

    slm.free ()

def test_pytrie ():
    trie = PinyinTrie()
    if not trie.load ("../data/pydict_sc.bin"):
        return

    root = trie.get_root_node ()
    node = trie.transfer (root, 0x1000)
    for w in node.get_words ():
        print(w)

    print(trie.is_valid (node, False, 0))
    print(trie[10000])

    print(trie.get_symbol_id (u'。'))
    trie.free ()

test_pyslm()
test_pytrie()

# -*- indent-tabs-mode: nil -*- vim:et:ts=4

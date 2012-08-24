#include "trie_writer.h"

template <>
bool revert_write<CPinyinTrie::TTransUnit
                  > (const CPinyinTrie::TTransUnit& t, FILE *fp)
{
    return revert_write(t.m_Syllable, fp) && revert_write(t.m_Offset, fp);
}

// -*- indent-tabs-mode: nil -*- vim:et:ts=4

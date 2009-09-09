#include <imi_uiobjects.h>
#include "sunpinyin_lookup_table.h"

SunPinyinLookupTable::SunPinyinLookupTable()
{
    m_lookup_table = ibus_lookup_table_new (9, 0, TRUE, TRUE);
}

SunPinyinLookupTable::~SunPinyinLookupTable()
{
    if (m_lookup_table) {
        g_object_unref (m_lookup_table);
        m_lookup_table = NULL;
    }
}

int
SunPinyinLookupTable::update_candidates(const ICandidateList& cl)
{
    const int size = cl.size();
    if (size <= 0)
        return size;
    
    const int total = cl.total();    
    // expand the array in lookup_table
    // we will fill the missing items in when we have them
    ibus_lookup_table_set_page_size(m_lookup_table, size);
    g_array_set_size(m_lookup_table->candidates, total);
    // XXX: any way to tell ibus the total number of candidates in
    // lookup_table, a.k.a. table->candidates->len ???
    for (int i = 0, begin = 0; i < size; ++i) {
        const int len = append_candidate(cl, i, begin);
        if (len)
            begin += len;
        else
            break;
    }
    return size;
    //ibus_lookup_table_set_cursor_pos (m_lookup_table, index);
}

bool
SunPinyinLookupTable::cursor_up()
{
    ibus_lookup_table_cursor_down(m_lookup_table);
    return true;
}

bool
SunPinyinLookupTable::cursor_down()
{
    ibus_lookup_table_cursor_down(m_lookup_table);
    return true;
}

size_t
SunPinyinLookupTable::get_cursor_pos() const
{
    return ibus_lookup_table_get_cursor_pos(m_lookup_table);
}

IBusLookupTable *
SunPinyinLookupTable::get()
{
    return m_lookup_table;
}

// an alternative to ibus_lookup_table_append_candidate(m_lookup_table, text);
// if we can assume that WinHandler::updateCandiates() is called
// in sequence. we can use ibus_lookup_table_append_candidate()
static void
ibus_lookup_table_set_candidate(IBusLookupTable *table,
                                guint index,
                                IBusText *text)
{
    g_return_if_fail (IBUS_IS_LOOKUP_TABLE (table));
    g_return_if_fail (IBUS_IS_TEXT (text));
    g_assert(index < table->candidates->len);
    
    g_object_ref (text);
    g_array_insert_val (table->candidates, index, text);
}

int
SunPinyinLookupTable::append_candidate(const ICandidateList& cl,
                                       int item,
                                       int begin)
{
    const TWCHAR* cand = 0;
    int len = 0;
    
    cand = cl.candiString(item);
    if (!cand)
        return len;
    len = cl.candiSize(item);
    IBusText *text = ibus_text_new_from_ucs4(cand);
    decorate_candidate(text, cl.candiType(item));
    int index = get_current_page_start() + item;
    ibus_lookup_table_set_candidate(m_lookup_table, index, text);
    g_object_unref(text);       // XXX: shall we unref it?
    return len;
}

void
SunPinyinLookupTable::decorate_candidate(IBusText *text, int type)
{
    switch (type) {
    case ICandidateList::BEST_WORD:
        ibus_text_append_attribute (text, IBUS_ATTR_TYPE_FOREGROUND,
                                    0x00ffffff, 0, -1);
        ibus_text_append_attribute (text, IBUS_ATTR_TYPE_BACKGROUND,
                                    0x00000000, 0, -1);
        break;
    case ICandidateList::USER_SELECTED_WORD:
        break;
    case ICandidateList::NORMAL_WORD:
        break;
    }
}

int
SunPinyinLookupTable::get_current_page_start() const
{
    guint cursor = ibus_lookup_table_get_cursor_pos(m_lookup_table);
    guint cursor_in_page = ibus_lookup_table_get_cursor_in_page(m_lookup_table);
    return cursor - cursor_in_page;
}

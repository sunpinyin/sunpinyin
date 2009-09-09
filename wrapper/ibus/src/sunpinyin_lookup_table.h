#ifndef SUNPINYIN_LOOKUP_TABLE_H
#define SUNPINYIN_LOOKUP_TABLE_H

#include <ibus.h>

class ICandidateList;

/**
 * a helper class to maintain the IBusLookupTable
 */
class SunPinyinLookupTable
{
public:
    SunPinyinLookupTable();
    ~SunPinyinLookupTable();
    /**
     * update lookup table with given candidate list
     * @param cl candidate list
     * @return the number of candidates updated
     */
    int update_candidates(const ICandidateList& cl);
    bool cursor_up();
    bool cursor_down();
    size_t get_cursor_pos() const;
    /**
     * get the internal IBusLookupTable pointer
     */
    IBusLookupTable *get();
    
private:
    /**
     * set the candidate in lookup table
     * @param cl the candidate list including all available candidates in current page
     * @param index the candidate's index in page
     * @param begin the begin position in candidates
     * @return the length of candidate string
     */
    int append_candidate(const ICandidateList& cl, int index, int begin);
 
    /**
     * get the index of current page's start point
     * @return the index where current page starts
     */
    int get_current_page_start() const;

    /**
     * update text's attributes according to the type of candidate
     * @param text
     * @param type
     */
    void decorate_candidate(IBusText *text, int type);

private:
    IBusLookupTable *m_lookup_table;
};

#endif // SUNPINYIN_LOOKUP_TABLE_H

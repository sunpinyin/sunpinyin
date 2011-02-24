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

#include <sunpinyin.h>
#include "ibus_portable.h"
#include "sunpinyin_lookup_table.h"

SunPinyinLookupTable::SunPinyinLookupTable() 
    : Pointer<IBusLookupTable>(ibus_lookup_table_new (9, 0, TRUE, TRUE))
{}

SunPinyinLookupTable::~SunPinyinLookupTable()
{
}

int
SunPinyinLookupTable::update_candidates(const ICandidateList& cl)
{
    const int size = cl.size();
    if (size <= 0)
        return size;
    
    const int total = cl.total();
    // total is the number of items in all pages, while size is the number of items in the current
    // page (only these items are included in the candidate list).
    ibus_lookup_table_set_page_size(*this, size);
    // We are updating the current page only, thus some ugly hacks are needed...
    ibus_lookup_table_clear(*this);
    int page_start = get_current_page_start(), cur_total = 0;
    // Dummy candidates are inserted when they are not in the current page.
    // Maybe we can share these candidates (but what if someone else changes the attributes?)
    for (; cur_total < page_start; ++cur_total)
	ibus_lookup_table_append_candidate(*this, ibus_text_new_from_static_string(""));
    for (int i = 0; i < size; ++i) {
	const TWCHAR* cand = cl.candiString(i);
	if (cand && cl.candiSize(i)) {
	    ibus::Text text(ibus_text_new_from_ucs4(cand));
	    decorate_candidate(text, cl.candiType(i));
	    ibus_lookup_table_append_candidate(*this, text);
	    ++cur_total;
	} else break; // I think this shouldn't happen...
    }
    for (; cur_total < total; ++cur_total)
	ibus_lookup_table_append_candidate(*this, ibus_text_new_from_static_string(""));

    return size;
    //ibus_lookup_table_set_cursor_pos (m_lookup_table, index);
}

bool
SunPinyinLookupTable::cursor_up()
{
    ibus_lookup_table_cursor_down(*this);
    return true;
}

bool
SunPinyinLookupTable::cursor_down()
{
    ibus_lookup_table_cursor_down(*this);
    return true;
}

size_t
SunPinyinLookupTable::get_cursor_pos() const
{
    return ibus_lookup_table_get_cursor_pos(*this);
}

void
SunPinyinLookupTable::decorate_candidate(ibus::Text text, int type)
{
    switch (type) {
    case ICandidateList::BEST_WORD:
        ibus_text_append_attribute (text, IBUS_ATTR_TYPE_FOREGROUND,
                                    0x003E6B8A, 0, -1);
        ibus_text_append_attribute (text, IBUS_ATTR_TYPE_UNDERLINE,
                                    IBUS_ATTR_UNDERLINE_SINGLE, 0, -1);
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
    guint cursor = ibus_lookup_table_get_cursor_pos(*this);
    guint cursor_in_page = ibus_lookup_table_get_cursor_in_page(*this);
    return cursor - cursor_in_page;
}

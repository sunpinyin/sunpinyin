/*
 * Copyright (c) 2007 Kov Chai <tchaikov@gmail.com>
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

#include <cassert>
#include <vector>

#include "portability.h"

#include "sunpinyin_lookup_table.h"
#include "sunpinyin_utils.h"

#include <imi_uiobjects.h>

using namespace scim;

template <class Seg>
class SegVector
{
    typedef typename Seg::value_type T;
    std::vector<T>      m_buffer;
    std::vector<uint32> m_index;
public:
    uint32 length() const {
        return m_index.size();
    }
    
    void push_back(const Seg& s) {
        m_index.push_back(m_buffer.size());
        m_buffer.insert(m_buffer.end(), s.begin(), s.end());
    }
    
    void push_back(const T& t) {
        m_index.push_back(m_buffer.size());
        m_buffer.push_back(t);
    }
    
    Seg operator [] (uint32 index) const {
        const uint32 len = length();
        if (index >= len)
            return Seg();
        typename std::vector<T>::const_iterator begin, end;
        begin = m_buffer.begin() + m_index[index];
        if (index < len - 1)
            end = m_buffer.begin() + m_index[index+1];
        else
            end = m_buffer.end();
        return Seg(begin, end);
    }
    
    void clear() {
        std::vector<T>     ().swap(m_buffer);
        std::vector<uint32>().swap(m_index);
    }
};

class SunLookupTable::SunLookupTableImpl
{
public:
    SegVector<WideString>     m_candidates;
    SegVector<AttributeList>  m_attributes;
};

SunLookupTable::SunLookupTable(int page_size)
    : LookupTable(page_size),
      m_impl( new SunLookupTableImpl ())
{
    std::vector <WideString> labels;
    char buf [2] = { 0, 0 };
    for (int i = 0; i < 9; ++i) {
        buf [0] = '1' + i;
        labels.push_back (utf8_mbstowcs (buf));
    }

    labels.push_back (utf8_mbstowcs ("0"));
    fix_page_size(false);
    set_candidate_labels (labels);
}

SunLookupTable::~SunLookupTable()
{
    delete m_impl;
}

WideString
SunLookupTable::get_candidate (int index) const
{
    // the start point should be synced
    SCIM_DEBUG_IMENGINE  (3) <<  "get_candidate(" << index << ")\n";
    
    index = translate_index(index);
    
    if (index < 0) {
        SCIM_DEBUG_IMENGINE (2) << "index out of bound: " << index << "!\n";
        return WideString();
    }
    else
        return m_impl->m_candidates[index];
}

AttributeList
SunLookupTable::get_attributes (int index) const
{
    index = translate_index(index);
    
    if (index < 0) {
        SCIM_DEBUG_IMENGINE (2) << "index out of bound: " << index << "!\n";
        return AttributeList();
    }
    else
        return m_impl->m_attributes[index];
}

uint32
SunLookupTable::number_of_candidates () const
{
    return m_total;
}

void
SunLookupTable::clear()
{
    LookupTable::clear();
    m_impl->m_candidates.clear();
    m_impl->m_attributes.clear();
}

void
SunLookupTable::update(const ICandidateList& cl)
{
    clear();

    const int sz = cl.size();
    m_total = cl.total();
    
    for (int i = 0, begin = 0; i < sz; ++i) {
        const int len = append_candidate(cl, i, begin);
        if (len)
            begin += len;
        else
            break;
    }
    LookupTable::set_page_size(sz);
    SCIM_DEBUG_IMENGINE (3) << "update() " << sz << "/" << m_total << " candidates updated\n";
}

int
SunLookupTable::translate_index(int index) const
{
    // SunLookupTable *only* provides access to candidates in current page
    assert(index >= get_current_page_start());
    assert(index < (get_current_page_start() + get_current_page_size()));
    return index - get_current_page_start();
}

int
SunLookupTable::append_candidate(const ICandidateList& cl, int i, int begin)
{
    const TWCHAR* cand = 0;
    int len = 0;
    
    if ( (cand = cl.candiString(i)) ) {
        len = cl.candiSize(i);
        m_impl->m_candidates.push_back( wstr_to_widestr(cand, len) );
        // XXX, a better looking decoration should be used
        // to disable this, simply s/push_back(attr)/push_back(AttributeList())/
        Attribute attr(begin, len, SCIM_ATTR_DECORATE);
        switch (cl.candiType(i)) {
        case ICandidateList::BEST_TAIL:
            attr.set_value(SCIM_ATTR_DECORATE_REVERSE);
            break;
        case ICandidateList::BEST_WORD:
            attr.set_value(SCIM_ATTR_DECORATE_UNDERLINE);
            break;
        default:
            // NORMAL_WORD, USER_SELECTED_WORD, ...
            break;
        }
        m_impl->m_attributes.push_back(attr);
    }
    return len;
}

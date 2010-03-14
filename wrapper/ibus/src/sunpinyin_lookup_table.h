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

#ifndef SUNPINYIN_LOOKUP_TABLE_H
#define SUNPINYIN_LOOKUP_TABLE_H

#include <ibus.h>
#include "ibus_common.h"

class ICandidateList;

/**
 * a helper class to maintain the IBusLookupTable
 */
class SunPinyinLookupTable : public ibus::LookupTable
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
    void decorate_candidate(ibus::Text text, int type);
};

#endif // SUNPINYIN_LOOKUP_TABLE_H

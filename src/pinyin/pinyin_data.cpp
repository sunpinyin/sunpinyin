/*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
 *
 * Copyright (c) 2007 Sun Microsystems, Inc. All Rights Reserved.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pinyin_data.h"
#include "pinyin_info.h"

static int
pytab_entry_compare(const char *s, TPyTabEntry *v)
{
    return strcmp(s, v->pystr);
}

TSyllable
CPinyinData::encodeSyllable(const char *pinyin)
{
    typedef int (*bsearch_compare)(const void*, const void*);
    TPyTabEntry *e = (TPyTabEntry*)bsearch(pinyin, pinyin_table,
                                           sizeof(pinyin_table) /
                                           sizeof(pinyin_table[0]),
                                           sizeof(pinyin_table[0]),
                                           (bsearch_compare)pytab_entry_compare);
    if (e)
        return e->id;

    return 0;
}

const char *
CPinyinData::decodeSyllable(TSyllable s, const char **i, const char **f)
{
    if (i) *i = initials[s.initial];
    if (f) *f = finals[s.final];

    static char buf[128];
    snprintf(buf, sizeof(buf), "%s%s", initials[s.initial], finals[s.final]);

    typedef int (*bsearch_compare)(const void*, const void*);
    TPyTabEntry *e = (TPyTabEntry*)bsearch(buf, pinyin_table,
                                           sizeof(pinyin_table) /
                                           sizeof(pinyin_table[0]),
                                           sizeof(pinyin_table[0]),
                                           (bsearch_compare)pytab_entry_compare);

    if (e)
        return e->pystr;

    return NULL;
}

const char **
CPinyinData::getAutoCorrectionPairs(unsigned &num)
{
    num = num_auto_correction_pairs;
    return auto_correction_pairs;
}

const char **
CPinyinData::getFuzzyPairs(unsigned &num)
{
    num = num_fuzzy_pairs;
    return fuzzy_pairs;
}

const char **
CPinyinData::getInitials(unsigned &num)
{
    num = num_initials;
    return initials;
}

const char **
CPinyinData::getFinals(unsigned &num)
{
    num = num_finals;
    return finals;
}

const TPyTabEntry *
CPinyinData::getPinyinTable(unsigned &num)
{
    num = sizeof(pinyin_table) / sizeof(TPyTabEntry);
    return pinyin_table;
}

const unsigned *
CPinyinData::getInnerFuzzyFinalMap(unsigned &num)
{
    num = num_fuzzy_finals;
    return fuzzy_finals_map;
}

void
CPinyinData::getFuzzyPreProSyllables(const unsigned **pre_syls,
                                     const unsigned **pro_syls)
{
    *pre_syls = fuzzy_pre_syllables;
    *pro_syls = fuzzy_pro_syllables;
}

// -*- indent-tabs-mode: nil -*- vim:et:ts=4

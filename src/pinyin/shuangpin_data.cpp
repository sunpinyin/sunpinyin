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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <cassert>
#include "syllable.h"
#include "pinyin_data.h"
#include "shuangpin_data.h"

#define INITIAL_NUM 26
#define FINAL_NUM   35
#define ZEROINITIAL_NUM 12

static const char *initials[INITIAL_NUM] =
{ "", "", "", "b", "p", "m", "f", "d", "t", "n", "l",
  "g", "k", "h", "j", "q", "x", "zh",
  "ch", "sh", "r", "z", "c", "s", "y", "w", };

static const char *finals[FINAL_NUM] = { "", "a", "o", "e", "ai", "ei", "ao",
                                         "ou", "an", "en", "ang", "eng", "er",
                                         "i", "ia", "ie", "iao", "iu", "ian",
                                         "in", "iang", "ing", "u",
                                         "ua", "uo", "uai", "ui", "uan", "un",
                                         "uang", "ong", "v", "ve", "ue", "iong", };


//MS2003 shuangpin plan map table
static const char ms2003_mapinitials[INITIAL_NUM] =
{ '\0', '\0', 'o', 'b', 'p', 'm', 'f', 'd',
  't', 'n', 'l', 'g', 'k',
  'h', 'j', 'q', 'x', 'v', 'i', 'u', 'r', 'z', 'c', 's',
  'y', 'w', };

static const char ms2003_mapfinals[FINAL_NUM] =
{ '\0', 'a', 'o', 'e', 'l', 'z', 'k',
  'b', 'j', 'f', 'h', 'g', 'r',
  'i', 'w', 'x', 'c', 'q', 'm', 'n', 'd', ';', 'u',
  'w', 'o', 'y', 'v', 'r', 'p',
  'd', 's', 'v', 't', 't', 's', };

static TZeroInitial ms2003_zeroinitials[] = {
    { "a", "oa" },
    { "ai", "ol" },
    { "an", "oj" },
    { "ang", "oh" },
    { "ao", "ok" },
    { "e", "oe" },
    { "ei", "oz" },
    { "en", "of" },
    { "eng", "og" },
    { "er", "or" },
    { "o", "oo" },
    { "ou", "ob" },
};


//ZhiNengABC shuangpin plan map table
static const char abc_mapinitials[INITIAL_NUM] =
{ '\0', '\0', 'o', 'b', 'p', 'm', 'f', 'd',
  't', 'n', 'l', 'g', 'k', 'h',
  'j', 'q', 'x', 'a', 'e', 'v', 'r', 'z', 'c', 's',
  'y', 'w', };

static const char abc_mapfinals[FINAL_NUM] =
{ '\0', 'a', 'o', 'e', 'l', 'q', 'k',
  'b', 'j', 'f', 'h', 'g', 'r',
  'i', 'd', 'x', 'z', 'r', 'w', 'c', 't', 'y', 'u',
  'd', 'o', 'c', 'm', 'p', 'n',
  't', 's', 'v', 'm', 'm', 's', };

static TZeroInitial abc_zeroinitials[] = {
    { "a", "oa" },
    { "ai", "ol" },
    { "an", "oj" },
    { "ang", "oh" },
    { "ao", "ok" },
    { "e", "oe" },
    { "ei", "oq" },
    { "en", "of" },
    { "eng", "og" },
    { "er", "or" },
    { "o", "oo" },
    { "ou", "ob" },
};

//ZiGuang shuangpin plan map table
static const char ziguang_mapinitials[INITIAL_NUM] =
{ '\0', '\0', 'o', 'b', 'p', 'm', 'f', 'd',
  't', 'n', 'l', 'g', 'k',
  'h', 'j', 'q', 'x', 'u', 'a', 'i', 'r', 'z', 'c', 's',
  'y', 'w', };

static const char ziguang_mapfinals[FINAL_NUM] =
{ '\0', 'a', 'o', 'e', 'p', 'k', 'q',
  'z', 'r', 'w', 's', 't', 'j',
  'i', 'x', 'd', 'b', 'j', 'f', 'y', 'g', ';', 'u',
  'x', 'o', 'y', 'n', 'l', 'm',
  'g', 'h', 'v', 'n', 'n', 'h', };

static TZeroInitial ziguang_zeroinitials[] = {
    { "a", "oa" },
    { "ai", "op" },
    { "an", "or" },
    { "ang", "os" },
    { "ao", "oq" },
    { "e", "oe" },
    { "ei", "ok" },
    { "en", "ow" },
    { "eng", "ot" },
    { "er", "oj" },
    { "o", "oo" },
    { "ou", "oz" },
};

//PinYinJiaJia shuangpin plan map table
static const char pyjiajia_mapinitials[INITIAL_NUM] =
{ '\0', '\0', 'o', 'b', 'p', 'm', 'f', 'd',
  't', 'n', 'l', 'g', 'k',
  'h', 'j', 'q', 'x', 'v', 'u', 'i', 'r', 'z', 'c', 's',
  'y', 'w', };

static const char pyjiajia_mapfinals[FINAL_NUM] =
{ '\0', 'a', 'o', 'e', 's', 'w', 'd',
  'p', 'f', 'r', 'g', 't',
  'q', 'i', 'b', 'm', 'k', 'n', 'j', 'l', 'h', 'q', 'u',
  'b', 'o', 'x', 'v', 'c',
  'z', 'h', 'y', 'v', 'x', 'x', 'y', };

static TZeroInitial pyjiajia_zeroinitials[] = {
    { "a", "oa" },
    { "ai", "os" },
    { "an", "of" },
    { "ang", "og" },
    { "ao", "od" },
    { "e", "oe" },
    { "ei", "ow" },
    { "en", "or" },
    { "eng", "ot" },
    { "er", "oq" },
    { "o", "oo" },
    { "ou", "op" },
};

//Ziranma shuangpin plan map table
static const char ziranma_mapinitials[INITIAL_NUM] =
{ 'a', 'e', 'o', 'b', 'p', 'm', 'f', 'd',
  't', 'n', 'l', 'g', 'k',
  'h', 'j', 'q', 'x', 'v', 'i', 'u', 'r', 'z', 'c', 's',
  'y', 'w', };

static const char ziranma_mapfinals[FINAL_NUM] =
{ '\0', 'a', 'o', 'e', 'l', 'z', 'k',
  'b', 'j', 'f', 'h', 'g',
  '\0', 'i', 'w', 'x', 'c', 'q', 'm', 'n', 'd', 'y', 'u',
  'w', 'o', 'y', 'v', 'r', 'p',
  'd', 's', 'v', 't', 't', 's', };

static TZeroInitial ziranma_zeroinitials[] = {
    { "a", "aa" },
    { "ai", "ai" },
    { "an", "an" },
    { "ang", "ah" },
    { "ao", "ao" },
    { "e", "ee" },
    { "ei", "ei" },
    { "en", "en" },
    { "eng", "eg" },
    { "er", "er" },
    { "o", "oo" },
    { "ou", "ou" },
};

//Ziranma shuangpin plan map table
static const char xiaohe_mapinitials[INITIAL_NUM] =
{ 'a', 'e', 'o', 'b', 'p', 'm', 'f', 'd',
  't', 'n', 'l', 'g', 'k',
  'h', 'j', 'q', 'x', 'v', 'i', 'u', 'r', 'z', 'c', 's',
  'y', 'w', };

static const char xiaohe_mapfinals[FINAL_NUM] =
{ '\0', 'a', 'o', 'e', 'd', 'w', 'c',
  'z', 'j', 'f', 'h', 'g', '\0',
  'i', 'x', 'p', 'n', 'q', 'm', 'b', 'l', 'k', 'u',
  'x', 'o', 'k', 'v', 'r', 'y',
  'l', 's', 'v', 't', 't', 's', };

static TZeroInitial xiaohe_zeroinitials[] = {
    { "a", "aa" },
    { "ai", "ai" },
    { "an", "an" },
    { "ang", "ah" },
    { "ao", "ao" },
    { "e", "ee" },
    { "ei", "ei" },
    { "en", "en" },
    { "eng", "eg" },
    { "er", "er" },
    { "o", "oo" },
    { "ou", "ou" },
};

CShuangpinData::CShuangpinData (EShuangpinType shpPlan) : m_shuangpinPlan(NULL)
{
    _genCodingMap();
    _genKeyboardMap(shpPlan);
}

CShuangpinData::~CShuangpinData()
{
    delete m_shuangpinPlan;
}

TSyllable
CShuangpinData::encodeSyllable(const char *pinyin)
{
    std::map<std::string, TSyllable>::iterator iter;
    iter = m_codingmap.find(pinyin);
    if (iter != m_codingmap.end()) {
        return iter->second;
    }
    return (TSyllable)0;
}

const char *
CShuangpinData::decodeSyllable(TSyllable s, const char **i, const char **f)
{
    return NULL;
}

int
CShuangpinData::getMapString(const char *shpstr, CMappedYin &syls)
{
    assert(shpstr != NULL);
    assert(m_shuangpinPlan->mapinitials != NULL);

    const char *ch = shpstr;
    int len = strlen(shpstr);
    char buf[32] = "\0";

    syls.clear();
    switch (len) {
    case 1:
        for (int i = 0; i < INITIAL_NUM; i++) {
            if (*ch == m_shuangpinPlan->mapinitials[i]) {
                syls.push_back(std::string(initials[i]));
                return 1;
            }
        }
        break;
    case 2:
        if (m_shuangpinPlan->zeroinitals != NULL) {
            for (int i = 0; i < ZEROINITIAL_NUM; i++) {
                if (!strcmp(shpstr, m_shuangpinPlan->zeroinitals[i].mapshp)) {
                    syls.push_back(std::string(m_shuangpinPlan->zeroinitals[i].
                                               syl));
                    return 1;
                }
            }
        }

        for (int i = 0; i < INITIAL_NUM; i++) {
            if (*ch == m_shuangpinPlan->mapinitials[i]) {
                for (int j = 0; j < FINAL_NUM; j++) {
                    if (*(ch + 1) == m_shuangpinPlan->mapfinals[j]) {
                        sprintf(buf, "%s%s", initials[i], finals[j]);
                        std::map<std::string, TSyllable>::iterator iter;
                        iter = m_codingmap.find(buf);
                        if (iter != m_codingmap.end()) {
                            syls.push_back(std::string(buf));
                        }
                    }
                }
            }
        }
        break;
    default:
        //Invalid shuangpin input character number, return 0;
        break;
    }

    return syls.size();
}

void
CShuangpinData::setShuangpinType(EShuangpinType shpType)
{
    if (shpType == getShuangpinType()) {
        return;
    }
    _genKeyboardMap(shpType);
}


void
CShuangpinData::_genCodingMap()
{
    unsigned len;
    const TPyTabEntry *pinyin_table = CPinyinData::getPinyinTable(len);

    for (size_t i = 0; i < len; ++i) {
        m_codingmap.insert(CEncodingMap::value_type(pinyin_table[i].pystr,
                                                    pinyin_table[i].id));
    }
}

void
CShuangpinData::_genKeyboardMap(EShuangpinType shyType)
{
    if (m_shuangpinPlan == NULL) {
        m_shuangpinPlan = new TShuangpinPlan;
        memset(m_shuangpinPlan, 0, sizeof(TShuangpinPlan));
    }
    m_shuangpinPlan->type = shyType;
    switch (shyType) {
    case MS2003:
        m_shuangpinPlan->mapinitials = (char*)ms2003_mapinitials;
        m_shuangpinPlan->mapfinals = (char*)ms2003_mapfinals;
        m_shuangpinPlan->zeroinitals = ms2003_zeroinitials;
        break;
    case ABC:
        m_shuangpinPlan->mapinitials = (char*)abc_mapinitials;
        m_shuangpinPlan->mapfinals = (char*)abc_mapfinals;
        m_shuangpinPlan->zeroinitals = abc_zeroinitials;
        break;
    case ZIGUANG:
        m_shuangpinPlan->mapinitials = (char*)ziguang_mapinitials;
        m_shuangpinPlan->mapfinals = (char*)ziguang_mapfinals;
        m_shuangpinPlan->zeroinitals = ziguang_zeroinitials;
        break;
    case PINYINJIAJIA:
        m_shuangpinPlan->mapinitials = (char*)pyjiajia_mapinitials;
        m_shuangpinPlan->mapfinals = (char*)pyjiajia_mapfinals;
        m_shuangpinPlan->zeroinitals = pyjiajia_zeroinitials;
        break;
    case ZIRANMA:
        m_shuangpinPlan->mapinitials = (char*)ziranma_mapinitials;
        m_shuangpinPlan->mapfinals = (char*)ziranma_mapfinals;
        m_shuangpinPlan->zeroinitals = ziranma_zeroinitials;
        break;
    case XIAOHE:
        m_shuangpinPlan->mapinitials = (char*)xiaohe_mapinitials;
        m_shuangpinPlan->mapfinals = (char*)xiaohe_mapfinals;
        m_shuangpinPlan->zeroinitals = xiaohe_zeroinitials;
        break;
    default:
        break;
    }
}

// -*- indent-tabs-mode: nil -*- vim:et:ts=4

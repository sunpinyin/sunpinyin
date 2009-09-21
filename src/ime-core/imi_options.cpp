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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <algorithm>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "imi_option_keys.h"
#include "imi_keys.h"
#include "imi_options.h"
#include "imi_view_classic.h"

CSimplifiedChinesePolicy::CSimplifiedChinesePolicy()
    : m_bLoaded(false), m_bTried(false), m_csLevel(3),
      m_bEnableFullSymbol(false), m_bEnableFullPunct(true)
{}

bool
CSimplifiedChinesePolicy::loadResources()
{
    if (m_bLoaded || m_bTried)
        return m_bLoaded;

    bool suc = true;
    suc &= m_coreData.loadResource (SUNPINYIN_DATA_DIR"/lm_sc.t3g", SUNPINYIN_DATA_DIR"/pydict_sc.bin");

    char path[256];
    const char *home = getenv ("HOME");
    snprintf (path, sizeof(path), "%s/%s", home, SUNPINYIN_USERDATA_DIR_PREFIX);
    suc &= createDirectory(path);
    
    CBigramHistory::initClass();
    snprintf (path, sizeof(path), "%s/%s/history", home, SUNPINYIN_USERDATA_DIR_PREFIX);
    suc &= m_historyCache.loadFromFile (path);
    
    snprintf (path, sizeof(path), "%s/%s/userdict", home, SUNPINYIN_USERDATA_DIR_PREFIX);
    suc &= m_userDict.load (path);
    
    m_bTried = true;
    return m_bLoaded = suc;
}

CIMIContext *
CSimplifiedChinesePolicy::createContext()
{
    CIMIContext* pic = new CIMIContext ();
    pic->setCoreData (&m_coreData);
    pic->setHistoryMemory (&m_historyCache);
    pic->setUserDict (&m_userDict);

    pic->setFullSymbolForwarding (m_bEnableFullSymbol);
    pic->setGetFullSymbolOp (&m_getFullSymbolOp);

    pic->setFullPunctForwarding (m_bEnableFullPunct);
    pic->setGetFullPunctOp (&m_getFullPunctOp);
    return pic;
}

void
CSimplifiedChinesePolicy::destroyContext (CIMIContext *context)
{
    char path[256];
    const char *home = getenv ("HOME");
    snprintf (path, sizeof(path), "%s/%s/history", home, SUNPINYIN_USERDATA_DIR_PREFIX);
    m_historyCache.saveToFile(path);
}
    
CShuangpinSchemePolicy::CShuangpinSchemePolicy()
    : m_shuangpinType(MS2003)
{}

bool
CSimplifiedChinesePolicy::createDirectory(const char *path) {
    struct stat stat_buf;
    
    if (stat (path, &stat_buf)) {
        if (mkdir (path, S_IRUSR | S_IWUSR | S_IXUSR)) {
            perror("unabled to mkdir() for user history.\n");
            return false;
        }
    } else if (!S_ISDIR(stat_buf.st_mode)) {
        fprintf(stderr, "%s is not a directory.\n", path);
        return false;
    }
    return true;
}

size_t init_pinyin_pairs(const char** pinyins, size_t len, const COptionEvent& event)
{
    std::vector<std::string> pairs = event.get_string_list();
    size_t num = std::min(len, pairs.size()) / 2;
    for (int i = 0; i  < num*2; ++i) {
        pinyins[i] = pairs[i].c_str();
    }
    return num;
}

bool
CQuanpinSchemePolicy::onConfigChanged(const COptionEvent& event)
{
    if (event.name == QUANPIN_FUZZY_ENABLED) {
        setFuzzyForwarding(event.get_bool());
    } else if (event.name == QUANPIN_FUZZY_PINYINS) {
        const char* pinyins[MAX_FUZZY_PINYINS];
        size_t num = init_pinyin_pairs(pinyins, MAX_FUZZY_PINYINS, event);
        setFuzzyPinyinPairs(pinyins, num);
    } else if (event.name == QUANPIN_AUTOCORRECTING_ENABLED) {
        setAutoCorrecting(event.get_bool());
    } else if (event.name == QUANPIN_AUTOCORRECTING_PINYINS) {
        const char* pinyins[MAX_AUTOCORRECTION_PINYINS];
        size_t num = init_pinyin_pairs(pinyins, MAX_AUTOCORRECTION_PINYINS,
                                       event);
        setAutoCorrectionPairs(pinyins, num);
    } else {
        return false;
    }
    return true;
}

bool
CShuangpinSchemePolicy::onConfigChanged(const COptionEvent& event)
{
    if (event.name == SHUANGPIN_TYPE) {
        setShuangpinType( (EShuangpinType) event.get_unsigned());
        return true;
    }
    return false;
}

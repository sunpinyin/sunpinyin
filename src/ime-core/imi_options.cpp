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
#include <cassert>
#include <errno.h>

#include "imi_option_keys.h"
#include "imi_keys.h"
#include "imi_options.h"
#include "imi_view_classic.h"

CSimplifiedChinesePolicy::CSimplifiedChinesePolicy()
    : m_bLoaded(false), m_bTried(false), m_csLevel(3),
      m_bEnableFullSymbol(false), m_bEnableFullPunct(true)
{
    m_getFullPunctOp.initPunctMap(getDefaultPunctMapping());
}

bool
CSimplifiedChinesePolicy::loadResources()
{
    if (m_bLoaded || m_bTried)
        return m_bLoaded;

    bool suc = true;
    std::string data_dir  = m_data_dir.size()? m_data_dir: SUNPINYIN_DATA_DIR;
    std::string lm_path   = data_dir + "/lm_sc.t3g";
    std::string dict_path = data_dir + "/pydict_sc.bin";

    suc &= m_coreData.loadResource (lm_path.c_str(), dict_path.c_str());

    if (!m_user_data_dir.size()) {
        char path[256];
        const char *home = getenv ("HOME");
        snprintf (path, sizeof(path), "%s/%s", home, SUNPINYIN_USERDATA_DIR_PREFIX);
        m_user_data_dir = path;
    }

    char * tmp = strdup (m_user_data_dir.c_str());
    suc &= createDirectory (tmp);
    free (tmp);
    
    std::string history_path = m_user_data_dir + "/history";
    suc &= m_historyCache.loadFromFile (history_path.c_str());
    
    std::string user_dict_path = m_user_data_dir + "/userdict";
    suc &= m_userDict.load (user_dict_path.c_str());
    
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

    pic->setCharsetLevel (m_csLevel);

    pic->setFullSymbolForwarding (m_bEnableFullSymbol);
    pic->setGetFullSymbolOp (&m_getFullSymbolOp);

    pic->setFullPunctForwarding (m_bEnableFullPunct);
    pic->setGetFullPunctOp (&m_getFullPunctOp);
    return pic;
}

void
CSimplifiedChinesePolicy::destroyContext (CIMIContext *context)
{
    assert(context != NULL);
    saveUserHistory();
    delete context;
}

string_pairs
CSimplifiedChinesePolicy::getDefaultPunctMapping() const
{
    static const char* punc_map [] = { 
        " ",    "　",
        ",",    "，",
        ";",    "；",
        "!",    "！",
        "?",    "？",
        ".",    "。",
        ":",    "：",
        "^",    "……",
        "\\",   "、",
        "\"",   "“",
        "\"",   "”",
        "'",    "‘",
        "'",    "’",
        "_",    "——",
        "<",    "《",
        ">",    "》",
        "(",    "（",
        ")",    "）",
        "[",    "【",
        "]",    "】",
        "{",    "『",
        "}",    "』",
        "$",    "￥",
        NULL,
    };

    string_pairs default_punc_map;

    const char *const *p = punc_map;
    while (*p) {
        std::string k = *p++;
        std::string v = *p++;
        default_punc_map.push_back (std::make_pair(k, v));
    }
    return default_punc_map;
}

bool
CSimplifiedChinesePolicy::onConfigChanged (const COptionEvent& event)
{
    if (event.name == SYSTEM_DATA_DIR) {
        setDataDir(event.get_string());
    } else if (event.name == USER_DATA_DIR) {
        setUserDataDir(event.get_string());
    } else if (event.name == CONFIG_GENERAL_CHARSET_LEVEL) {
        m_csLevel = event.get_int() & 3;
    }

    return false;
}

bool
CSimplifiedChinesePolicy::saveUserHistory ()
{
    return m_historyCache.saveToFile();
}

bool
CSimplifiedChinesePolicy::createDirectory(char *path) {
    char *p = path;
    while ((p = strchr(p+1, '/'))) {
        *p = 0;
        if (access(path, F_OK) != 0 && mkdir(path, S_IRWXU) != 0) {
            fprintf(stderr, "mkdir %s: %s\n", path, strerror(errno));
            return false;
        }
        *p = '/';
    }
    return !(access(path, F_OK) != 0 && mkdir(path, S_IRWXU) != 0);
}

CShuangpinSchemePolicy::CShuangpinSchemePolicy()
    : m_shuangpinType(MS2003)
{}

bool
CQuanpinSchemePolicy::onConfigChanged(const COptionEvent& event)
{
    if (event.name == QUANPIN_FUZZY_ENABLED) {
        setFuzzyForwarding(event.get_bool());
        return true;
    } else if (event.name == QUANPIN_FUZZY_PINYINS) {
        setFuzzyPinyinPairs(event.get_string_pair_list());
        return true;
    } else if (event.name == QUANPIN_AUTOCORRECTION_ENABLED) {
        setAutoCorrecting(event.get_bool());
        return true;
    } else if (event.name == QUANPIN_AUTOCORRECTION_PINYINS) {
        setAutoCorrectionPairs(event.get_string_pair_list());
        return true;
    }

    return false;
}

bool
CShuangpinSchemePolicy::onConfigChanged(const COptionEvent& event)
{
    if (event.name == SHUANGPIN_TYPE) {
        setShuangpinType( (EShuangpinType) event.get_int());
        return true;
    } else if (event.name == QUANPIN_FUZZY_ENABLED) {
        setFuzzyForwarding(event.get_bool());
        return true;
    } else if (event.name == QUANPIN_FUZZY_PINYINS) {
        setFuzzyPinyinPairs(event.get_string_pair_list());
        return true;
    }

    return false;
}

bool
CHunpinSchemePolicy::onConfigChanged(const COptionEvent& event)
{
    if (event.name == SHUANGPIN_TYPE) {
        setShuangpinType( (EShuangpinType) event.get_int());
		//刷新segmentor狀態
		COptionEventBus& event_bus = AOptionEventBus::instance();
		event_bus.publishEvent(COptionEvent(PINYIN_SCHEME, -1));//第二個參數沒有用
        return true;
    } else if (event.name == QUANPIN_FUZZY_ENABLED) {
        setFuzzyForwarding(event.get_bool());
        return true;
    } else if (event.name == QUANPIN_FUZZY_PINYINS) {
        setFuzzyPinyinPairs(event.get_string_pair_list());
        return true;
    }
	
    return false;
}


CHunpinSchemePolicy::CHunpinSchemePolicy()
: m_shuangpinType(MS2003)
{}

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

#ifndef _IMI_OPTIONS_H
#define _IMI_OPTIONS_H

#include "utils.h"
#include "portability.h"
#include "imi_view.h"
#include "imi_view_classic.h"
#include "imi_funcobjs.h"
#include "imi_data.h"
#include "userdict.h"
#include "ic_history.h"
#include "shuangpin_seg.h"

struct CSimplifiedChinesePolicy
{
    static bool loadResources ()
    {
        if (s_bLoaded || s_bTried)
            return s_bLoaded;

        bool suc = true;
        suc &= s_coreData.loadResource (SUNPINYIN_DATA_DIR"/lm_sc.t3g", SUNPINYIN_DATA_DIR"/pydict_sc.bin");

        char path[256];
        const char *home = getenv ("HOME");

        CBigramHistory::initClass();
        snprintf (path, sizeof(path), "%s/%s/history", home, s_userDataDirPrefix);
        suc &= s_historyCache.loadFromFile (path);

        snprintf (path, sizeof(path), "%s/%s/userdict", home, s_userDataDirPrefix);
        suc &= s_userDict.load (path);

        s_bTried = true;
        return s_bLoaded = suc;
    }

    static CIMIContext* createContext () 
    {
        CIMIContext* pic = new CIMIContext ();
        pic->setCoreData (&s_coreData);
        pic->setHistoryMemory (&s_historyCache);
        pic->setUserDict (&s_userDict);

        pic->setFullSymbolForwarding (s_bEnableFullSymbol);
        pic->setGetFullSymbolOp (&s_getFullSymbolOp);

        pic->setFullPunctForwarding (s_bEnableFullPunct);
        pic->setGetFullPunctOp (&s_getFullPunctOp);
        return pic;
    }

    static void setPunctMapping (const char **map)
        {s_getFullPunctOp.initPunctMap (map);}

    static void setUserDataPrefix (const char *prefix) {s_userDataDirPrefix = prefix;}
    static void enableFullSymbol (bool v=true) {s_bEnableFullSymbol = v;}
    static void enableFullPunct (bool v=true) {s_bEnableFullPunct = v;}

protected:
    ~CSimplifiedChinesePolicy () {}

    static CIMIData             s_coreData;
    static CBigramHistory       s_historyCache;
    static CUserDict            s_userDict;
    static bool                 s_bLoaded;
    static bool                 s_bTried;
    static unsigned             s_csLevel;
    static bool                 s_bEnableFullSymbol;
    static CGetFullSymbolOp     s_getFullSymbolOp;
    static bool                 s_bEnableFullPunct;
    static CGetFullPunctOp      s_getFullPunctOp;
    static const char          *s_userDataDirPrefix;
};

struct CQuanpinSchemePolicy
{
public:
    static IPySegmentor* createPySegmentor () 
    {
        CQuanpinSegmentor *pseg = new CQuanpinSegmentor (SUNPINYIN_DATA_DIR"/quanpin.dat");
        pseg->setGetFuzzySyllablesOp (&s_getFuzzySyllablesOp);
        pseg->setGetCorrectionPairOp (&s_getCorrectionPairOp);
        return pseg;
    }

    static void setFuzzyForwarding (bool v=true)
        {s_getFuzzySyllablesOp.setEnable (v);}

    static void setFuzzyPinyinPairs (const char **pairs, unsigned num)
        {s_getFuzzySyllablesOp.initFuzzyMap (pairs, num);}

    static void setAutoCorrecting (bool v=true)
        {s_getCorrectionPairOp.setEnable (v);}

    static void setAutoCorrectionPairs (const char **pairs, unsigned num) 
        {s_getCorrectionPairOp.setCorrectionPairs (pairs, num);}

protected:
    ~CQuanpinSchemePolicy () {}

    static CGetFuzzySyllablesOp s_getFuzzySyllablesOp;
    static CGetCorrectionPairOp s_getCorrectionPairOp;
};

struct CShuangpinSchemePolicy
{
public:
    static IPySegmentor* createPySegmentor () 
    {
        CShuangpinSegmentor *pseg = new CShuangpinSegmentor (s_shuangpinType);
        return pseg;
    }

    static void setShuangpinType (EShuangpinType t) {s_shuangpinType = t;}

protected:
    ~CShuangpinSchemePolicy () {}
    static EShuangpinType s_shuangpinType;
};


struct CClassicStylePolicy
{
    static CIMIView* createView () {return new CIMIClassicView ();}

protected:
    ~CClassicStylePolicy () {}
};

struct ISunpinyinProfile
{
    virtual CIMIView* createProfile () = 0;
    virtual ~ISunpinyinProfile () {};
};

template <class LanguagePolicy, class PinyinSchemePolicy, class InputStylePolicy>
class CSunpinyinProfile : public ISunpinyinProfile, LanguagePolicy, PinyinSchemePolicy, InputStylePolicy
{
public:
    CSunpinyinProfile () {};

    CIMIView* createProfile ()
    {
        if (LanguagePolicy::loadResources ()) {
            CIMIContext *pic = LanguagePolicy::createContext ();
            CIMIView* pview = InputStylePolicy::createView ();
            pview->attachIC (pic);

            IPySegmentor* pseg = PinyinSchemePolicy::createPySegmentor ();
            pview->setPySegmentor (pseg);

            return pview;
        }

        return NULL;
    }
};

class CSunpinyinSessionFactory : private CNonCopyable
{
public:
    typedef enum {
        QUANPIN,
        SHUANGPIN,
        YUEPIN,
        ZHUYIN,
    } EPyScheme;
    
    typedef enum {
        MSPY_STYLE,
        CLASSIC_STYLE,
    } EInputStyle;

    typedef enum {
        SIMPLIFIED_CHINESE,
        TRADITIONAL_CHINESE,
    } ELanguage;

public:
    static CSunpinyinSessionFactory& getFactory ()
    {
        static CSunpinyinSessionFactory inst;
        return inst;
    }

    void setLanguage (ELanguage lang) {m_lang = lang;}
    void setInputStyle (EInputStyle inputStyle) {m_inputStyle = inputStyle;}
    void setPinyinScheme (EPyScheme pyScheme) {m_pyScheme = pyScheme;}
    void setCandiWindowSize (unsigned size) {m_candiWindowSize = size;}

    CIMIView* createSession ()
    {
        unsigned key = _policiesToKey (m_lang, m_pyScheme, m_inputStyle);
        std::map <unsigned, ISunpinyinProfile*>::iterator it = m_profiles.find (key);
        if (it != m_profiles.end()) {
            CIMIView *pview = it->second->createProfile ();
            pview->setHotkeyProfile (&m_hotkeyProfile);
            pview->setCandiWindowSize (m_candiWindowSize);
            return pview;
        }

        return NULL;
    }

    void destroySession (CIMIView *pview)
    {
        if (pview) {
            delete pview->getIC();
            delete pview->getPySegmentor();
            delete pview;
        }
    }

    void updateToken () {m_tokenNum ++;}
    unsigned getToken () {return m_tokenNum;}

private:
    CSunpinyinSessionFactory ()
        : m_pyScheme (QUANPIN), m_inputStyle(CLASSIC_STYLE), m_lang(SIMPLIFIED_CHINESE),
          m_candiWindowSize(10), m_tokenNum(0) 
    {
        m_profiles [_policiesToKey (SIMPLIFIED_CHINESE, QUANPIN, CLASSIC_STYLE)] = 
                new CSunpinyinProfile <CSimplifiedChinesePolicy, CQuanpinSchemePolicy, CClassicStylePolicy> ();

        m_profiles [_policiesToKey (SIMPLIFIED_CHINESE, SHUANGPIN, CLASSIC_STYLE)] = 
                new CSunpinyinProfile <CSimplifiedChinesePolicy, CShuangpinSchemePolicy, CClassicStylePolicy> ();
    }

    ~CSunpinyinSessionFactory ()
    {
        std::map <unsigned, ISunpinyinProfile*>::iterator it  = m_profiles.begin ();
        std::map <unsigned, ISunpinyinProfile*>::iterator ite = m_profiles.end ();
        
        for (; it != ite; ++it)
            delete it->second;
    }

    unsigned _policiesToKey (ELanguage lang, EPyScheme pyScheme, EInputStyle inputStyle)
        {return (lang<<16) + (pyScheme<<8) + inputStyle;}

    std::map <unsigned, ISunpinyinProfile*> m_profiles;

    EPyScheme           m_pyScheme;
    EInputStyle         m_inputStyle;
    ELanguage           m_lang;
    unsigned            m_candiWindowSize;
    CHotkeyProfile      m_hotkeyProfile;

    unsigned            m_tokenNum;
};
#endif

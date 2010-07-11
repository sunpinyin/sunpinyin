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
#include "imi_option_event.h"
#include "userdict.h"
#include "ic_history.h"
#include "pinyin/shuangpin_seg.h"

#ifndef SUNPINYIN_USERDATA_DIR_PREFIX
#define SUNPINYIN_USERDATA_DIR_PREFIX ".sunpinyin"
#endif

struct CSimplifiedChinesePolicy : public IConfigurable
{
    CSimplifiedChinesePolicy ();
    
    bool loadResources ();

    CIMIContext* createContext ();
    void destroyContext (CIMIContext *context);
    
    void setPunctMapping (string_pairs punc_map)
        {m_getFullPunctOp.initPunctMap (punc_map);}

    string_pairs getDefaultPunctMapping () const;
    
    void enableFullSymbol (bool v=true) {m_bEnableFullSymbol = v;}
    void enableFullPunct  (bool v=true) {m_bEnableFullPunct = v;}

    void setDataDir (const std::string& data_dir)
        {m_data_dir = data_dir;}

    void setUserDataDir (const std::string& user_data_dir)
        {m_user_data_dir = user_data_dir;}

    virtual bool onConfigChanged (const COptionEvent& event);
    
    template<class> friend class SingletonHolder;
    
protected:
    ~CSimplifiedChinesePolicy () {}

    bool createDirectory (char *path);
    bool saveUserHistory ();
    
    CIMIData             m_coreData;
    CBigramHistory       m_historyCache;
    CUserDict            m_userDict;
    bool                 m_bLoaded;
    bool                 m_bTried;
    unsigned             m_csLevel;
    bool                 m_bEnableFullSymbol;
    CGetFullSymbolOp     m_getFullSymbolOp;
    bool                 m_bEnableFullPunct;
    CGetFullPunctOp      m_getFullPunctOp;
    std::string          m_data_dir;
    std::string          m_user_data_dir;
};

typedef SingletonHolder<CSimplifiedChinesePolicy> ASimplifiedChinesePolicy;

struct CQuanpinSchemePolicy : IConfigurable
{
public:
    
    IPySegmentor* createPySegmentor () 
    {
        CQuanpinSegmentor *pseg = new CQuanpinSegmentor ();
        pseg->setGetFuzzySyllablesOp (&m_getFuzzySyllablesOp);
        pseg->setGetCorrectionPairOp (&m_getCorrectionPairOp);
        pseg->setGetFuzzySegmentsOp  (&m_getFuzzySegmentsOp);
        return pseg;
    }

    void setFuzzyForwarding (bool enable_fuzzies=true, bool enable_simpler_initials=true)
    {
        m_getFuzzySyllablesOp.setEnableFuzzies (enable_fuzzies);
        m_getFuzzySyllablesOp.setEnableSimplerInitials (enable_simpler_initials);
    }

    void clearFuzzyPinyinPairs ()
        {m_getFuzzySyllablesOp.clearFuzzyMap();}

    void setFuzzyPinyinPairs (const string_pairs& pairs, bool duplex = true)
        {m_getFuzzySyllablesOp.initFuzzyMap (pairs, duplex);}

    void setAutoCorrecting (bool v=true)
        {m_getCorrectionPairOp.setEnable (v);}

    void setAutoCorrectionPairs (const string_pairs& pairs) 
        {m_getCorrectionPairOp.setCorrectionPairs (pairs);}

    void setFuzzySegmentation (bool v=true)
        {m_getFuzzySegmentsOp.setEnable (v);}

    void setInnerFuzzySegmentation (bool v=true)
        {m_getFuzzySegmentsOp.setInnerFuzzyEnable (v);}

    virtual bool onConfigChanged(const COptionEvent& event);
    
    template<class> friend class SingletonHolder;
    
protected:
    ~CQuanpinSchemePolicy () {}

    CGetFuzzySyllablesOp<CPinyinData>   m_getFuzzySyllablesOp;
    CGetCorrectionPairOp                m_getCorrectionPairOp;
    CGetFuzzySegmentsOp                 m_getFuzzySegmentsOp;
};

typedef SingletonHolder<CQuanpinSchemePolicy> AQuanpinSchemePolicy;

struct CShuangpinSchemePolicy : public IConfigurable
{
public:
    CShuangpinSchemePolicy();
    
    IPySegmentor* createPySegmentor () 
    {
        CShuangpinSegmentor *pseg = new CShuangpinSegmentor (m_shuangpinType);
        pseg->setGetFuzzySyllablesOp (&m_getFuzzySyllablesOp);
        return pseg;
    }

    void setShuangpinType (EShuangpinType t) {m_shuangpinType = t;}

    void setFuzzyForwarding (bool enable_fuzzies=true, bool enable_simpler_initials=true)
    {
        m_getFuzzySyllablesOp.setEnableFuzzies (enable_fuzzies);
        m_getFuzzySyllablesOp.setEnableSimplerInitials (enable_simpler_initials);
    }

    void clearFuzzyPinyinPairs ()
        {m_getFuzzySyllablesOp.clearFuzzyMap();}

    void setFuzzyPinyinPairs (const string_pairs& pairs, bool duplex = true)
        {m_getFuzzySyllablesOp.initFuzzyMap (pairs, duplex);}

    virtual bool onConfigChanged(const COptionEvent& event);
    
    template<class> friend class SingletonHolder;
protected:
    ~CShuangpinSchemePolicy () {}

    EShuangpinType                      m_shuangpinType;
    CGetFuzzySyllablesOp<CPinyinData>   m_getFuzzySyllablesOp;
};

typedef SingletonHolder<CShuangpinSchemePolicy> AShuangpinSchemePolicy;

struct CClassicStylePolicy : public IConfigurable
{
    CIMIView* createView () {return new CIMIClassicView ();}

    template<class> friend class SingletonHolder;
protected:
    ~CClassicStylePolicy () {}
};

typedef SingletonHolder<CClassicStylePolicy> AClassicStylePolicy;

struct ISunpinyinProfile
{
    virtual CIMIView* createProfile () = 0;
    virtual void destroyProfile (CIMIView *) = 0;
    virtual ~ISunpinyinProfile () {};
};

template <class LanguagePolicy, class PinyinSchemePolicy, class InputStylePolicy>
class CSunpinyinProfile : public ISunpinyinProfile
{
public:
    CSunpinyinProfile () : m_langPolicy(LanguagePolicy::instance()), 
                           m_pySchemePolicy(PinyinSchemePolicy::instance()),
                           m_inputStylePolicy(InputStylePolicy::instance()) 
    {}

    /* profile by itself is a profile, so we are creating a session here? */
    virtual CIMIView* createProfile ()
    {
        if (!m_langPolicy.loadResources ())
            return NULL;

        IPySegmentor* pseg = m_pySchemePolicy.createPySegmentor ();
        if (pseg == NULL)
            return NULL;
        
        CIMIContext *pic = m_langPolicy.createContext ();
        CIMIView* pview = m_inputStylePolicy.createView ();
        pview->attachIC (pic);
        pview->setPySegmentor (pseg);
        
        return pview;
    }

    virtual void destroyProfile(CIMIView* pview)
    {
        if (pview) {
            LanguagePolicy::instance().destroyContext(pview->getIC());
            delete pview->getPySegmentor();
            delete pview;
        }
    }
private:
    typename LanguagePolicy::Type&      m_langPolicy;
    typename PinyinSchemePolicy::Type&  m_pySchemePolicy;
    typename InputStylePolicy::Type&    m_inputStylePolicy;
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
        ISunpinyinProfile *profile = _getProfile(key);
        if (!profile)
            return NULL;
        
        CIMIView *pview = profile->createProfile ();
        if (!pview)
            return NULL;
        
        pview->setHotkeyProfile (&m_hotkeyProfile);
        pview->setCandiWindowSize (m_candiWindowSize);
        return pview;
    }

    void destroySession (CIMIView* pview)
    {
        unsigned key = _policiesToKey (m_lang, m_pyScheme, m_inputStyle);
        ISunpinyinProfile *profile = _getProfile(key);
        if (!profile)
            return;
        profile->destroyProfile(pview);
    }

private:
    CSunpinyinSessionFactory ()
        : m_pyScheme (QUANPIN), m_inputStyle(CLASSIC_STYLE), m_lang(SIMPLIFIED_CHINESE),
          m_candiWindowSize(10)
    {
        m_profiles [_policiesToKey (SIMPLIFIED_CHINESE, QUANPIN, CLASSIC_STYLE)] = 
                new CSunpinyinProfile <ASimplifiedChinesePolicy, AQuanpinSchemePolicy, AClassicStylePolicy> ();

        m_profiles [_policiesToKey (SIMPLIFIED_CHINESE, SHUANGPIN, CLASSIC_STYLE)] = 
                new CSunpinyinProfile <ASimplifiedChinesePolicy, AShuangpinSchemePolicy, AClassicStylePolicy> ();
    }

    ~CSunpinyinSessionFactory ()
    {
        std::map <unsigned, ISunpinyinProfile*>::iterator it  = m_profiles.begin ();
        std::map <unsigned, ISunpinyinProfile*>::iterator ite = m_profiles.end ();
        
        for (; it != ite; ++it)
            delete it->second;
    }

    ISunpinyinProfile* _getProfile(unsigned key)
    {
        std::map <unsigned, ISunpinyinProfile*>::iterator it = m_profiles.find (key);
        if (it != m_profiles.end()) {
            return it->second;
        } else {
            return NULL;
        }
    }

    unsigned _policiesToKey (ELanguage lang, EPyScheme pyScheme, EInputStyle inputStyle)
        {return (lang<<16) + (pyScheme<<8) + inputStyle;}
    
    std::map <unsigned, ISunpinyinProfile*> m_profiles;

    EPyScheme           m_pyScheme;
    EInputStyle         m_inputStyle;
    ELanguage           m_lang;
    unsigned            m_candiWindowSize;
    CHotkeyProfile      m_hotkeyProfile;
};

#endif

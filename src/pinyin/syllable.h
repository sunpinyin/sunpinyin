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

#ifndef __SUNPINYIN_SYLLABLE_H__
#define __SUNPINYIN_SYLLABLE_H__

#include <vector>
#include <string>
#include <map>
#include <cstdio>

#include "ime-core/utils.h"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

struct TSyllable {
#ifdef WORDS_BIGENDIAN
    unsigned other    : 12;
    unsigned initial  : 8;
    unsigned final    : 8;
    unsigned tone     : 4;
#else
    unsigned tone     : 4;
    unsigned final    : 8;
    unsigned initial  : 8;
    unsigned other    : 12;
#endif
    TSyllable (unsigned int s=0)
        { *((unsigned*)this) = s; }

    TSyllable (int i, int f, int t) 
        : initial(i), final(f), tone(t), other(0) { }

    operator unsigned int() const 
        { return *((unsigned *)this); }
    
    bool isFullSyllable () const
        { return final != 0;}

    bool operator == (const TSyllable & syl) const {
        return (unsigned int )*this == (unsigned int)(syl);
    }
    
    bool operator != (const TSyllable & syl) const {
        return !(*this == syl);
    }
    
    bool operator == (const unsigned s) const {
        return (unsigned int)*this == s;
    }
};

typedef struct _TPyTabEntry {
    const char *pystr;
    unsigned    id;
} TPyTabEntry;

typedef std::vector<TSyllable> CSyllables;

template <class PinyinDataPolicy>
class CGetFuzzySyllablesOp : private CNonCopyable
{
public: 
    typedef std::multimap<const std::string, std::string> CFuzzyMap;

    CGetFuzzySyllablesOp () : m_bEnableFuzzies(false) {}

    void setEnableFuzzies         (bool value=true) {m_bEnableFuzzies = value;}
    void setEnableSimplerInitials (bool value=true) {m_bEnableSimplerInitials = value;}
    bool isEnabled () {return m_bEnableFuzzies || m_bEnableSimplerInitials;}

    void clearFuzzyMap ()
        {m_fuzzyMap.clear();}

    void initFuzzyMap (const string_pairs& fuzzyPairs, bool duplex = true)
        {
            string_pairs::const_iterator it =  fuzzyPairs.begin();
            string_pairs::const_iterator ite = fuzzyPairs.end();

            for (; it != ite; ++it) {
                const std::string i = it->first;
                const std::string j = it->second;
               
                if (m_fuzzyMap.find(i) == m_fuzzyMap.end())
                    m_fuzzyMap.insert (std::pair<const std::string, std::string> (i, j));

                if (duplex && m_fuzzyMap.find(j) == m_fuzzyMap.end())
                    m_fuzzyMap.insert (std::pair<const std::string, std::string> (j, i));
            }
        }

    CSyllables operator () (TSyllable s)
        {
            CSyllables ret;
            static char buf[128];

            const char *i, *f;
            PinyinDataPolicy::decodeSyllable (s, &i, &f);

            if (m_bEnableSimplerInitials && !m_bEnableFuzzies && *f != '\0')
                return ret;

            std::vector<const char *> iset;
            std::vector<const char *> fset;

            iset.push_back (i);
            fset.push_back (f);

            CFuzzyMap::const_iterator it;
            for (it = m_fuzzyMap.lower_bound(i); it != m_fuzzyMap.upper_bound(i); ++it) 
                iset.push_back ((it->second).c_str());

            for (it = m_fuzzyMap.lower_bound(f); it != m_fuzzyMap.upper_bound(f); ++it)
                fset.push_back ((it->second).c_str());

            std::vector<const char *>::const_iterator iset_it = iset.begin();
            for (; iset_it != iset.end(); ++iset_it) {
                std::vector<const char *>::const_iterator fset_it = fset.begin();
                for (; fset_it != fset.end(); ++ fset_it) {
                    snprintf (buf, sizeof(buf), "%s%s", *iset_it, *fset_it);
                    TSyllable ts = PinyinDataPolicy::encodeSyllable (buf);
                    if (ts && ts != s)
                        ret.push_back (ts);
                }
            }

            return ret;
        }

private:
    CFuzzyMap   m_fuzzyMap;
    bool        m_bEnableFuzzies;
    bool        m_bEnableSimplerInitials;
};

#endif

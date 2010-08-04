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

#ifndef _SHUANGPIN_DATA_H_
#define _SHUANGPIN_DATA_H_

#include <string>
#include <map>

#include "ime-core/utils.h"
#include "syllable.h"

enum EShuangpinType {
    MS2003,
    ABC,
    ZIRANMA,
    PINYINJIAJIA,
    ZIGUANG,
    XIAOHE,
    USERDEFINE,
};

typedef struct _TZeroinitial {
    const char  *syl;
    const char  *mapshp;
} TZeroInitial;

typedef struct _TShungpinPlan {
    EShuangpinType type;
    char *mapinitials;
    char *mapfinals;
    TZeroInitial *zeroinitals;
} TShuangpinPlan;


typedef std::vector<std::string>         CMappedYin;
typedef std::map<std::string, TSyllable> CEncodingMap;
typedef std::map <char, CMappedYin*>     CKeyMap;

class CShuangpinData : private CNonCopyable
{
public:
    CShuangpinData (EShuangpinType shpPlan = MS2003);
    ~CShuangpinData ();
    
    
    
    TSyllable encodeSyllable (const char *);
    const char *decodeSyllable (TSyllable, const char **i=NULL, const char **f=NULL);

    int getMapString(const char *ch, CMappedYin & syls);
    EShuangpinType getShuangpinType () const {return m_shuangpinPlan->type;}
    void setShuangpinType(EShuangpinType shpPlan);
    
protected:
    void  _genCodingMap(); 
    void  _genKeyboardMap(EShuangpinType shyType);

    CEncodingMap    m_codingmap; 
    TShuangpinPlan *m_shuangpinPlan;
    CKeyMap         m_keymap;
};

#endif /* _PINYIN_DATA_H_ */

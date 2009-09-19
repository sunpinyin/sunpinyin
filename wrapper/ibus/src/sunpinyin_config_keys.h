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

#ifndef SUNPINYIN_CONFIG_KEYS_H
#define SUNPINYIN_CONFIG_KEYS_H

#define CONFIG_VIEW_CANDIDATE_WIN_SIZE      "engine/SunPinyin/View/CandidateWindowSize"
#define CONFIG_PINYIN_MEMORY_POWER          "engine/SunPinyin/Pinyin/MemoryPower"

#define CONFIG_PINYIN_PUNCT_MAPPING         "engine/SunPinyin/Pinyin/PunctMapping"
#define CONFIG_PINYIN_SCHEME                "engine/SunPinyin/Pinyin/Scheme"
#define CONFIG_PINYIN_SHUANGPIN_TYPE        "engine/SunPinyin/Pinyin/ShuangPinType"
#define CONFIG_PINYIN_AMBIGUITY_ANY         "engine/SunPinyin/Pinyin/Ambiguity/Any"
#define CONFIG_PINYIN_AMBIGUITY_ZhiZi       "engine/SunPinyin/Pinyin/Ambiguity/ZhiZi"
#define CONFIG_PINYIN_AMBIGUITY_ChiCi       "engine/SunPinyin/Pinyin/Ambiguity/ChiCi"
#define CONFIG_PINYIN_AMBIGUITY_ShiSi       "engine/SunPinyin/Pinyin/Ambiguity/ShiSi"
#define CONFIG_PINYIN_AMBIGUITY_AnAng       "engine/SunPinyin/Pinyin/Ambiguity/AnAng"
#define CONFIG_PINYIN_AMBIGUITY_OnOng       "engine/SunPinyin/Pinyin/Ambiguity/OnOng"
#define CONFIG_PINYIN_AMBIGUITY_EnEng       "engine/SunPinyin/Pinyin/Ambiguity/EnEng"
#define CONFIG_PINYIN_AMBIGUITY_InIng       "engine/SunPinyin/Pinyin/Ambiguity/InIng"
#define CONFIG_PINYIN_AMBIGUITY_EngOng      "engine/SunPinyin/Pinyin/Ambiguity/EngOng"
#define CONFIG_PINYIN_AMBIGUITY_IanIang     "engine/SunPinyin/Pinyin/Ambiguity/IanIang"
#define CONFIG_PINYIN_AMBIGUITY_UanUang     "engine/SunPinyin/Pinyin/Ambiguity/UanUang"
#define CONFIG_PINYIN_AMBIGUITY_NeLe        "engine/SunPinyin/Pinyin/Ambiguity/NeLe"
#define CONFIG_PINYIN_AMBIGUITY_FoHe        "engine/SunPinyin/Pinyin/Ambiguity/FoHe"
#define CONFIG_PINYIN_AMBIGUITY_LeRi        "engine/SunPinyin/Pinyin/Ambiguity/LeRi"
#define CONFIG_PINYIN_AMBIGUITY_KeGe        "engine/SunPinyin/Pinyin/Ambiguity/KeGe"

#define CONFIG_KEYBOARD_MODE_SWITCH_SHIFT   "engine/SunPinyin/Keyboard/ModeSwitch/Shift"
#define CONFIG_KEYBOARD_MODE_SWITCH_CONTROL "engine/SunPinyin/Keyboard/ModeSwitch/Control"
#define CONFIG_KEYBOARD_PAGE_COMMA          "engine/SunPinyin/Keyboard/Page/CommaPeriod"
#define CONFIG_KEYBOARD_PAGE_MINUS          "engine/SunPinyin/Keyboard/Page/MinusEquals"

#endif // SUNPINYIN_CONFIG_KEYS_H

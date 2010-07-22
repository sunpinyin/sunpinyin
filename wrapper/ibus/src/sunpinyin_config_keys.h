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

#define CONFIG_GENERAL_PAGE_SIZE            "General/PageSize"
#define CONFIG_GENERAL_MEMORY_POWER         "General/MemoryPower"
#define CONFIG_GENERAL_CHARSET_LEVEL        "General/Charset"
#define CONFIG_GENERAL_INITIAL_MODE         "General/InitialStatus/Mode"
#define CONFIG_GENERAL_INITIAL_PUNCT        "General/InitialStatus/Punct"
#define CONFIG_GENERAL_INITIAL_LETTER       "General/InitialStatus/Letter"

#define CONFIG_QUANPIN_FUZZY_ANY         "QuanPin/Fuzzy/Any"
#define CONFIG_QUANPIN_FUZZY_ZhiZi       "QuanPin/Fuzzy/ZhiZi"
#define CONFIG_QUANPIN_FUZZY_ChiCi       "QuanPin/Fuzzy/ChiCi"
#define CONFIG_QUANPIN_FUZZY_ShiSi       "QuanPin/Fuzzy/ShiSi"
#define CONFIG_QUANPIN_FUZZY_AnAng       "QuanPin/Fuzzy/AnAng"
#define CONFIG_QUANPIN_FUZZY_OnOng       "QuanPin/Fuzzy/OnOng"
#define CONFIG_QUANPIN_FUZZY_EnEng       "QuanPin/Fuzzy/EnEng"
#define CONFIG_QUANPIN_FUZZY_InIng       "QuanPin/Fuzzy/InIng"
#define CONFIG_QUANPIN_FUZZY_EngOng      "QuanPin/Fuzzy/EngOng"
#define CONFIG_QUANPIN_FUZZY_IanIang     "QuanPin/Fuzzy/IanIang"
#define CONFIG_QUANPIN_FUZZY_UanUang     "QuanPin/Fuzzy/UanUang"
#define CONFIG_QUANPIN_FUZZY_NeLe        "QuanPin/Fuzzy/NeLe"
#define CONFIG_QUANPIN_FUZZY_FoHe        "QuanPin/Fuzzy/FoHe"
#define CONFIG_QUANPIN_FUZZY_LeRi        "QuanPin/Fuzzy/LeRi"
#define CONFIG_QUANPIN_FUZZY_KeGe        "QuanPin/Fuzzy/KeGe"

#define CONFIG_QUANPIN_FUZZYSEGS_ENABLED  "QuanPin/FuzzySegs/Enabled"
#define CONFIG_QUANPIN_INNERFUZZY_ENABLED "QuanPin/InnerFuzzy/Enabled"

#define CONFIG_QUANPIN_CORRECTION_IgnIng "QuanPin/AutoCorrecting/IgnIng"
#define CONFIG_QUANPIN_CORRECTION_OgnOng "QuanPin/AutoCorrecting/OgnOng"
#define CONFIG_QUANPIN_CORRECTION_UenUn  "QuanPin/AutoCorrecting/UenUn"
#define CONFIG_QUANPIN_CORRECTION_ImgIng "QuanPin/AutoCorrecting/ImgIng"
#define CONFIG_QUANPIN_CORRECTION_IouIu  "QuanPin/AutoCorrecting/IouIu"
#define CONFIG_QUANPIN_CORRECTION_UeiUi  "QuanPin/AutoCorrecting/UeiUi"

#define CONFIG_KEYBOARD_MODE_SWITCH      "Keyboard/ModeSwitch"
#define CONFIG_KEYBOARD_PUNCT_SWITCH     "Keyboard/PunctSwitch"
#define CONFIG_KEYBOARD_PAGE_COMMA       "Keyboard/Page/CommaPeriod"
#define CONFIG_KEYBOARD_PAGE_MINUS       "Keyboard/Page/MinusEquals"
#define CONFIG_KEYBOARD_PAGE_BRACKET     "Keyboard/Page/Brackets"
#define CONFIG_KEYBOARD_CANCEL_BACKSPACE "Keyboard/CancelBackspace"

#endif // SUNPINYIN_CONFIG_KEYS_H

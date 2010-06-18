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

static const char *initials[] = {"", "b", "p", "m", "f", "d", "t", "n", "l", "g", "k", "h", "j", "q", "x", "zh", "ch", "sh", "r", "z", "c", "s", "y", "w", };
static const unsigned num_initials = sizeof(initials)/sizeof(*initials);

static const char *finals[] = {"", "a", "o", "e", "ai", "ei", "ao", "ou", "an", "en", "ang", "eng", "er", "i", "ia", "ie", "iao", "iu", "ian", "in", "iang", "ing", "u", "ua", "uo", "uai", "ui", "uan", "un", "uang", "ong", "v", "ue", "iong", };
static const unsigned num_finals = sizeof(finals)/sizeof(*finals);

static const char *fuzzy_finals[] = {"ia", "iao", "ian", "iang", "ie", "ua", "uai", "uan", "uang", "ue"};
static const unsigned num_fuzzy_finals = sizeof(fuzzy_finals)/sizeof(*fuzzy_finals);

static const unsigned fuzzy_finals_map[] = {
    0x0e,        0x10,        1,           /* ia   -> a    len 1 */
    0x10,        0x60,        2,           /* iao  -> ao   len 2 */
    0x12,        0x80,        2,           /* ian  -> an   len 2 */
    0x14,        0xa0,        3,           /* iang -> ang  len 3 */
    0x0f,        0x30,        1,           /* ie   -> e    len 1 */
    0x17,        0x10,        1,           /* ua   -> a    len 1 */
    0x19,        0x40,        2,           /* uai  -> ai   len 2 */
    0x1b,        0x80,        2,           /* uan  -> an   len 2 */
    0x1d,        0xa0,        3,           /* uang -> ang  len 3 */
    0x20,        0x30,        1,           /* ue   -> e    len 1 */
};

static const unsigned fuzzy_pre_syllables [] = {
    0x0d0e0,     'n',         0x0d120,     /* qian */
    0x09080,     'g',         0x090a0,     /* gang */
    0x080e0,     'n',         0x08120,     /* lian */
    0x15090,     'g',         0x150b0,     /* seng */
    0x04010,     'n',         0x04080,     /* fan */
    0x10030,     'n',         0x10090,     /* chen */
    0x050e0,     'n',         0x05120,     /* dian */
    0x15160,     'n',         0x151c0,     /* sun */
    0x07080,     'g',         0x070a0,     /* nang */
    0x0a160,     'n',         0x0a1c0,     /* kun */
    0x05030,     'n',         0x05090,     /* den */
    0x07090,     'g',         0x070b0,     /* neng */
    0x03030,     'n',         0x03090,     /* men */
    0x09090,     'g',         0x090b0,     /* geng */
    0x10080,     'g',         0x100a0,     /* chang */
    0x0f010,     'n',         0x0f080,     /* zhan */
    0x14010,     'n',         0x14080,     /* can */
    0x07130,     'g',         0x07150,     /* ning */
    0x17080,     'g',         0x170a0,     /* wang */
    0x01090,     'g',         0x010b0,     /* beng */
    0x0f1b0,     'g',         0x0f1d0,     /* zhuang */
    0x06010,     'n',         0x06080,     /* tan */
    0x00090,     'g',         0x000b0,     /* eng */
    0x0f080,     'g',         0x0f0a0,     /* zhang */
    0x02130,     'g',         0x02150,     /* ping */
    0x08010,     'n',         0x08080,     /* lan */
    0x0e160,     'n',         0x0e1c0,     /* xun */
    0x03010,     'n',         0x03080,     /* man */
    0x0c120,     'g',         0x0c140,     /* jiang */
    0x0a1b0,     'g',         0x0a1d0,     /* kuang */
    0x01130,     'g',         0x01150,     /* bing */
    0x13010,     'n',         0x13080,     /* zan */
    0x13030,     'n',         0x13090,     /* zen */
    0x02080,     'g',         0x020a0,     /* pang */
    0x0c0d0,     'n',         0x0c130,     /* jin */
    0x14030,     'n',         0x14090,     /* cen */
    0x05010,     'n',         0x05080,     /* dan */
    0x0f030,     'n',         0x0f090,     /* zhen */
    0x01080,     'g',         0x010a0,     /* bang */
    0x17090,     'g',         0x170b0,     /* weng */
    0x00030,     'n',         0x00090,     /* en */
    0x0a080,     'g',         0x0a0a0,     /* kang */
    0x09160,     'n',         0x091c0,     /* gun */
    0x00030,     'r',         0x000c0,     /* er */
    0x0a090,     'g',         0x0a0b0,     /* keng */
    0x15080,     'g',         0x150a0,     /* sang */
    0x12030,     'n',         0x12090,     /* ren */
    0x11160,     'n',         0x111c0,     /* shun */
    0x0d160,     'n',         0x0d1c0,     /* qun */
    0x16160,     'n',         0x161c0,     /* yun */
    0x0e120,     'g',         0x0e140,     /* xiang */
    0x12080,     'g',         0x120a0,     /* rang */
    0x09170,     'n',         0x091b0,     /* guan */
    0x16130,     'g',         0x16150,     /* ying */
    0x0a170,     'n',         0x0a1b0,     /* kuan */
    0x10010,     'n',         0x10080,     /* chan */
    0x160d0,     'n',         0x16130,     /* yin */
    0x0e0d0,     'n',         0x0e130,     /* xin */
    0x07120,     'g',         0x07140,     /* niang */
    0x0b160,     'n',         0x0b1c0,     /* hun */
    0x11170,     'n',         0x111b0,     /* shuan */
    0x05080,     'g',         0x050a0,     /* dang */
    0x00080,     'g',         0x000a0,     /* ang */
    0x15010,     'n',         0x15080,     /* san */
    0x12090,     'g',         0x120b0,     /* reng */
    0x03130,     'g',         0x03150,     /* ming */
    0x030d0,     'n',         0x03130,     /* min */
    0x07030,     'n',         0x07090,     /* nen */
    0x0a010,     'n',         0x0a080,     /* kan */
    0x16080,     'g',         0x160a0,     /* yang */
    0x05090,     'g',         0x050b0,     /* deng */
    0x101b0,     'g',         0x101d0,     /* chuang */
    0x04090,     'g',         0x040b0,     /* feng */
    0x03090,     'g',         0x030b0,     /* meng */
    0x10090,     'g',         0x100b0,     /* cheng */
    0x09030,     'n',         0x09090,     /* gen */
    0x01010,     'n',         0x01080,     /* ban */
    0x07160,     'n',         0x071c0,     /* nun */
    0x15030,     'n',         0x15090,     /* sen */
    0x04080,     'g',         0x040a0,     /* fang */
    0x08160,     'n',         0x081c0,     /* lun */
    0x0a030,     'n',         0x0a090,     /* ken */
    0x0b1b0,     'g',         0x0b1d0,     /* huang */
    0x03080,     'g',         0x030a0,     /* mang */
    0x06160,     'n',         0x061c0,     /* tun */
    0x0d0d0,     'n',         0x0d130,     /* qin */
    0x02090,     'g',         0x020b0,     /* peng */
    0x05160,     'n',         0x051c0,     /* dun */
    0x10160,     'n',         0x101c0,     /* chun */
    0x09010,     'n',         0x09080,     /* gan */
    0x13090,     'g',         0x130b0,     /* zeng */
    0x06080,     'g',         0x060a0,     /* tang */
    0x14080,     'g',         0x140a0,     /* cang */
    0x0b090,     'g',         0x0b0b0,     /* heng */
    0x0e0e0,     'n',         0x0e120,     /* xian */
    0x0f160,     'n',         0x0f1c0,     /* zhun */
    0x111b0,     'g',         0x111d0,     /* shuang */
    0x11010,     'n',         0x11080,     /* shan */
    0x02010,     'n',         0x02080,     /* pan */
    0x070d0,     'n',         0x07130,     /* nin */
    0x0b080,     'g',         0x0b0a0,     /* hang */
    0x0f170,     'n',         0x0f1b0,     /* zhuan */
    0x080d0,     'n',         0x08130,     /* lin */
    0x091b0,     'g',         0x091d0,     /* guang */
    0x0b010,     'n',         0x0b080,     /* han */
    0x14160,     'n',         0x141c0,     /* cun */
    0x010d0,     'n',         0x01130,     /* bin */
    0x11030,     'n',         0x11090,     /* shen */
    0x0e130,     'g',         0x0e150,     /* xing */
    0x0d120,     'g',         0x0d140,     /* qiang */
    0x12160,     'n',         0x121c0,     /* run */
    0x11090,     'g',         0x110b0,     /* sheng */
    0x10170,     'n',         0x101b0,     /* chuan */
    0x0d130,     'g',         0x0d150,     /* qing */
    0x0c0e0,     'n',         0x0c120,     /* jian */
    0x17010,     'n',         0x17080,     /* wan */
    0x0c130,     'g',         0x0c150,     /* jing */
    0x16010,     'n',         0x16080,     /* yan */
    0x08120,     'g',         0x08140,     /* liang */
    0x0b170,     'n',         0x0b1b0,     /* huan */
    0x0b030,     'n',         0x0b090,     /* hen */
    0x11080,     'g',         0x110a0,     /* shang */
    0x0c160,     'n',         0x0c1c0,     /* jun */
    0x08130,     'g',         0x08150,     /* ling */
    0x14090,     'g',         0x140b0,     /* ceng */
    0x020d0,     'n',         0x02130,     /* pin */
    0x00010,     'n',         0x00080,     /* an */
    0x13080,     'g',         0x130a0,     /* zang */
    0x07010,     'n',         0x07080,     /* nan */
    0x0f090,     'g',         0x0f0b0,     /* zheng */
    0x13160,     'n',         0x131c0,     /* zun */
    0x08080,     'g',         0x080a0,     /* lang */
    0x0,
};

static const unsigned fuzzy_pro_syllables [] = {
    0x09030,     'g',         0x00030,     /* ge */
    0x090a0,     'g',         0x000a0,     /* gang */
    0x09010,     'g',         0x00010,     /* ga */
    0x12070,     'r',         0x00070,     /* rou */
    0x07050,     'n',         0x00050,     /* nei */
    0x070a0,     'n',         0x000a0,     /* nang */
    0x070b0,     'n',         0x000b0,     /* neng */
    0x090b0,     'g',         0x000b0,     /* geng */
    0x07070,     'n',         0x00070,     /* nou */
    0x12030,     'r',         0x00030,     /* re */
    0x12090,     'r',         0x00090,     /* ren */
    0x09070,     'g',         0x00070,     /* gou */
    0x120a0,     'r',         0x000a0,     /* rang */
    0x120b0,     'r',         0x000b0,     /* reng */
    0x12080,     'r',         0x00080,     /* ran */
    0x12060,     'r',         0x00060,     /* rao */
    0x07090,     'n',         0x00090,     /* nen */
    0x09050,     'g',         0x00050,     /* gei */
    0x09090,     'g',         0x00090,     /* gen */
    0x09060,     'g',         0x00060,     /* gao */
    0x09080,     'g',         0x00080,     /* gan */
    0x09040,     'g',         0x00040,     /* gai */
    0x07060,     'n',         0x00060,     /* nao */
    0x07010,     'n',         0x00010,     /* na */
    0x07040,     'n',         0x00040,     /* nai */
    0x07080,     'n',         0x00080,     /* nan */
    0x07030,     'n',         0x00030,     /* ne */
    0x0,
};

static const char * fuzzy_pairs[] = {
    "z",       "zh", 
    "c",       "ch", 
    "s",       "sh", 
    "an",      "ang", 
    "on",      "ong", 
    "en",      "eng", 
    "in",      "ing", 
    "eng",     "ong", 
    "ian",     "iang", 
    "uan",     "uang", 
    "l",       "n", 
    "f",       "h", 
    "r",       "l", 
    "k",       "g",
};
static const unsigned num_fuzzy_pairs = sizeof(fuzzy_pairs)/sizeof(*fuzzy_pairs)/2;

static const char * auto_correction_pairs[] = {
    "ign",      "ing",
    "img",      "ing",
    "uei",      "ui",
    "uen",      "un",
    "iou",      "iu",
};
static const unsigned num_auto_correction_pairs = sizeof(auto_correction_pairs)/sizeof(*auto_correction_pairs)/2;

static const TPyTabEntry 
pinyin_table[] = {
    {"a",	0x00010},
    {"ai",	0x00040},
    {"an",	0x00080},
    {"ang",	0x000a0},
    {"ao",	0x00060},
    {"b",	0x01000},
    {"ba",	0x01010},
    {"bai",	0x01040},
    {"ban",	0x01080},
    {"bang",	0x010a0},
    {"bao",	0x01060},
    {"bei",	0x01050},
    {"ben",	0x01090},
    {"beng",	0x010b0},
    {"bi",	0x010d0},
    {"bian",	0x01120},
    {"biao",	0x01100},
    {"bie",	0x010f0},
    {"bin",	0x01130},
    {"bing",	0x01150},
    {"bo",	0x01020},
    {"bu",	0x01160},
    {"c",	0x14000},
    {"ca",	0x14010},
    {"cai",	0x14040},
    {"can",	0x14080},
    {"cang",	0x140a0},
    {"cao",	0x14060},
    {"ce",	0x14030},
    {"cei",	0x14050},
    {"cen",	0x14090},
    {"ceng",	0x140b0},
    {"ch",	0x10000},
    {"cha",	0x10010},
    {"chai",	0x10040},
    {"chan",	0x10080},
    {"chang",	0x100a0},
    {"chao",	0x10060},
    {"che",	0x10030},
    {"chen",	0x10090},
    {"cheng",	0x100b0},
    {"chi",	0x100d0},
    {"chong",	0x101e0},
    {"chou",	0x10070},
    {"chu",	0x10160},
    {"chua",	0x10170},
    {"chuai",	0x10190},
    {"chuan",	0x101b0},
    {"chuang",	0x101d0},
    {"chui",	0x101a0},
    {"chun",	0x101c0},
    {"chuo",	0x10180},
    {"ci",	0x140d0},
    {"cong",	0x141e0},
    {"cou",	0x14070},
    {"cu",	0x14160},
    {"cuan",	0x141b0},
    {"cui",	0x141a0},
    {"cun",	0x141c0},
    {"cuo",	0x14180},
    {"d",	0x05000},
    {"da",	0x05010},
    {"dai",	0x05040},
    {"dan",	0x05080},
    {"dang",	0x050a0},
    {"dao",	0x05060},
    {"de",	0x05030},
    {"dei",	0x05050},
    {"den",	0x05090},
    {"deng",	0x050b0},
    {"di",	0x050d0},
    {"dia",	0x050e0},
    {"dian",	0x05120},
    {"diao",	0x05100},
    {"die",	0x050f0},
    {"ding",	0x05150},
    {"diu",	0x05110},
    {"dong",	0x051e0},
    {"dou",	0x05070},
    {"du",	0x05160},
    {"duan",	0x051b0},
    {"dui",	0x051a0},
    {"dun",	0x051c0},
    {"duo",	0x05180},
    {"e",	0x00030},
    {"ei",	0x00050},
    {"en",	0x00090},
    {"eng",	0x000b0},
    {"er",	0x000c0},
    {"f",	0x04000},
    {"fa",	0x04010},
    {"fan",	0x04080},
    {"fang",	0x040a0},
    {"fei",	0x04050},
    {"fen",	0x04090},
    {"feng",	0x040b0},
    {"fiao",	0x04100},
    {"fo",	0x04020},
    {"fou",	0x04070},
    {"fu",	0x04160},
    {"g",	0x09000},
    {"ga",	0x09010},
    {"gai",	0x09040},
    {"gan",	0x09080},
    {"gang",	0x090a0},
    {"gao",	0x09060},
    {"ge",	0x09030},
    {"gei",	0x09050},
    {"gen",	0x09090},
    {"geng",	0x090b0},
    {"gong",	0x091e0},
    {"gou",	0x09070},
    {"gu",	0x09160},
    {"gua",	0x09170},
    {"guai",	0x09190},
    {"guan",	0x091b0},
    {"guang",	0x091d0},
    {"gui",	0x091a0},
    {"gun",	0x091c0},
    {"guo",	0x09180},
    {"h",	0x0b000},
    {"ha",	0x0b010},
    {"hai",	0x0b040},
    {"han",	0x0b080},
    {"hang",	0x0b0a0},
    {"hao",	0x0b060},
    {"he",	0x0b030},
    {"hei",	0x0b050},
    {"hen",	0x0b090},
    {"heng",	0x0b0b0},
    {"hong",	0x0b1e0},
    {"hou",	0x0b070},
    {"hu",	0x0b160},
    {"hua",	0x0b170},
    {"huai",	0x0b190},
    {"huan",	0x0b1b0},
    {"huang",	0x0b1d0},
    {"hui",	0x0b1a0},
    {"hun",	0x0b1c0},
    {"huo",	0x0b180},
    {"j",	0x0c000},
    {"ji",	0x0c0d0},
    {"jia",	0x0c0e0},
    {"jian",	0x0c120},
    {"jiang",	0x0c140},
    {"jiao",	0x0c100},
    {"jie",	0x0c0f0},
    {"jin",	0x0c130},
    {"jing",	0x0c150},
    {"jiong",	0x0c210},
    {"jiu",	0x0c110},
    {"ju",	0x0c160},
    {"juan",	0x0c1b0},
    {"jue",	0x0c200},
    {"jun",	0x0c1c0},
    {"k",	0x0a000},
    {"ka",	0x0a010},
    {"kai",	0x0a040},
    {"kan",	0x0a080},
    {"kang",	0x0a0a0},
    {"kao",	0x0a060},
    {"ke",	0x0a030},
    {"kei",	0x0a050},
    {"ken",	0x0a090},
    {"keng",	0x0a0b0},
    {"kong",	0x0a1e0},
    {"kou",	0x0a070},
    {"ku",	0x0a160},
    {"kua",	0x0a170},
    {"kuai",	0x0a190},
    {"kuan",	0x0a1b0},
    {"kuang",	0x0a1d0},
    {"kui",	0x0a1a0},
    {"kun",	0x0a1c0},
    {"kuo",	0x0a180},
    {"l",	0x08000},
    {"la",	0x08010},
    {"lai",	0x08040},
    {"lan",	0x08080},
    {"lang",	0x080a0},
    {"lao",	0x08060},
    {"le",	0x08030},
    {"lei",	0x08050},
    {"leng",	0x080b0},
    {"li",	0x080d0},
    {"lia",	0x080e0},
    {"lian",	0x08120},
    {"liang",	0x08140},
    {"liao",	0x08100},
    {"lie",	0x080f0},
    {"lin",	0x08130},
    {"ling",	0x08150},
    {"liu",	0x08110},
    {"lo",	0x08020},
    {"long",	0x081e0},
    {"lou",	0x08070},
    {"lu",	0x08160},
    {"luan",	0x081b0},
    {"lue",	0x08200},
    {"lun",	0x081c0},
    {"luo",	0x08180},
    {"lv",	0x081f0},
    {"m",	0x03000},
    {"ma",	0x03010},
    {"mai",	0x03040},
    {"man",	0x03080},
    {"mang",	0x030a0},
    {"mao",	0x03060},
    {"me",	0x03030},
    {"mei",	0x03050},
    {"men",	0x03090},
    {"meng",	0x030b0},
    {"mi",	0x030d0},
    {"mian",	0x03120},
    {"miao",	0x03100},
    {"mie",	0x030f0},
    {"min",	0x03130},
    {"ming",	0x03150},
    {"miu",	0x03110},
    {"mo",	0x03020},
    {"mou",	0x03070},
    {"mu",	0x03160},
    {"n",	0x07000},
    {"na",	0x07010},
    {"nai",	0x07040},
    {"nan",	0x07080},
    {"nang",	0x070a0},
    {"nao",	0x07060},
    {"ne",	0x07030},
    {"nei",	0x07050},
    {"nen",	0x07090},
    {"neng",	0x070b0},
    {"ni",	0x070d0},
    {"nian",	0x07120},
    {"niang",	0x07140},
    {"niao",	0x07100},
    {"nie",	0x070f0},
    {"nin",	0x07130},
    {"ning",	0x07150},
    {"niu",	0x07110},
    {"nong",	0x071e0},
    {"nou",	0x07070},
    {"nu",	0x07160},
    {"nuan",	0x071b0},
    {"nue",	0x07200},
    {"nun",	0x071c0},
    {"nuo",	0x07180},
    {"nv",	0x071f0},
    {"o",	0x00020},
    {"ou",	0x00070},
    {"p",	0x02000},
    {"pa",	0x02010},
    {"pai",	0x02040},
    {"pan",	0x02080},
    {"pang",	0x020a0},
    {"pao",	0x02060},
    {"pei",	0x02050},
    {"pen",	0x02090},
    {"peng",	0x020b0},
    {"pi",	0x020d0},
    {"pian",	0x02120},
    {"piao",	0x02100},
    {"pie",	0x020f0},
    {"pin",	0x02130},
    {"ping",	0x02150},
    {"po",	0x02020},
    {"pou",	0x02070},
    {"pu",	0x02160},
    {"q",	0x0d000},
    {"qi",	0x0d0d0},
    {"qia",	0x0d0e0},
    {"qian",	0x0d120},
    {"qiang",	0x0d140},
    {"qiao",	0x0d100},
    {"qie",	0x0d0f0},
    {"qin",	0x0d130},
    {"qing",	0x0d150},
    {"qiong",	0x0d210},
    {"qiu",	0x0d110},
    {"qu",	0x0d160},
    {"quan",	0x0d1b0},
    {"que",	0x0d200},
    {"qun",	0x0d1c0},
    {"r",	0x12000},
    {"ran",	0x12080},
    {"rang",	0x120a0},
    {"rao",	0x12060},
    {"re",	0x12030},
    {"ren",	0x12090},
    {"reng",	0x120b0},
    {"ri",	0x120d0},
    {"rong",	0x121e0},
    {"rou",	0x12070},
    {"ru",	0x12160},
    {"ruan",	0x121b0},
    {"rui",	0x121a0},
    {"run",	0x121c0},
    {"ruo",	0x12180},
    {"s",	0x15000},
    {"sa",	0x15010},
    {"sai",	0x15040},
    {"san",	0x15080},
    {"sang",	0x150a0},
    {"sao",	0x15060},
    {"se",	0x15030},
    {"sen",	0x15090},
    {"seng",	0x150b0},
    {"sh",	0x11000},
    {"sha",	0x11010},
    {"shai",	0x11040},
    {"shan",	0x11080},
    {"shang",	0x110a0},
    {"shao",	0x11060},
    {"she",	0x11030},
    {"shei",	0x11050},
    {"shen",	0x11090},
    {"sheng",	0x110b0},
    {"shi",	0x110d0},
    {"shou",	0x11070},
    {"shu",	0x11160},
    {"shua",	0x11170},
    {"shuai",	0x11190},
    {"shuan",	0x111b0},
    {"shuang",	0x111d0},
    {"shui",	0x111a0},
    {"shun",	0x111c0},
    {"shuo",	0x11180},
    {"si",	0x150d0},
    {"song",	0x151e0},
    {"sou",	0x15070},
    {"su",	0x15160},
    {"suan",	0x151b0},
    {"sui",	0x151a0},
    {"sun",	0x151c0},
    {"suo",	0x15180},
    {"t",	0x06000},
    {"ta",	0x06010},
    {"tai",	0x06040},
    {"tan",	0x06080},
    {"tang",	0x060a0},
    {"tao",	0x06060},
    {"te",	0x06030},
    {"tei",	0x06050},
    {"teng",	0x060b0},
    {"ti",	0x060d0},
    {"tian",	0x06120},
    {"tiao",	0x06100},
    {"tie",	0x060f0},
    {"ting",	0x06150},
    {"tong",	0x061e0},
    {"tou",	0x06070},
    {"tu",	0x06160},
    {"tuan",	0x061b0},
    {"tui",	0x061a0},
    {"tun",	0x061c0},
    {"tuo",	0x06180},
    {"w",	0x17000},
    {"wa",	0x17010},
    {"wai",	0x17040},
    {"wan",	0x17080},
    {"wang",	0x170a0},
    {"wei",	0x17050},
    {"wen",	0x17090},
    {"weng",	0x170b0},
    {"wo",	0x17020},
    {"wu",	0x17160},
    {"x",	0x0e000},
    {"xi",	0x0e0d0},
    {"xia",	0x0e0e0},
    {"xian",	0x0e120},
    {"xiang",	0x0e140},
    {"xiao",	0x0e100},
    {"xie",	0x0e0f0},
    {"xin",	0x0e130},
    {"xing",	0x0e150},
    {"xiong",	0x0e210},
    {"xiu",	0x0e110},
    {"xu",	0x0e160},
    {"xuan",	0x0e1b0},
    {"xue",	0x0e200},
    {"xun",	0x0e1c0},
    {"y",	0x16000},
    {"ya",	0x16010},
    {"yai",	0x16040},
    {"yan",	0x16080},
    {"yang",	0x160a0},
    {"yao",	0x16060},
    {"ye",	0x16030},
    {"yi",	0x160d0},
    {"yin",	0x16130},
    {"ying",	0x16150},
    {"yo",	0x16020},
    {"yong",	0x161e0},
    {"you",	0x16070},
    {"yu",	0x16160},
    {"yuan",	0x161b0},
    {"yue",	0x16200},
    {"yun",	0x161c0},
    {"z",	0x13000},
    {"za",	0x13010},
    {"zai",	0x13040},
    {"zan",	0x13080},
    {"zang",	0x130a0},
    {"zao",	0x13060},
    {"ze",	0x13030},
    {"zei",	0x13050},
    {"zen",	0x13090},
    {"zeng",	0x130b0},
    {"zh",	0x0f000},
    {"zha",	0x0f010},
    {"zhai",	0x0f040},
    {"zhan",	0x0f080},
    {"zhang",	0x0f0a0},
    {"zhao",	0x0f060},
    {"zhe",	0x0f030},
    {"zhei",	0x0f050},
    {"zhen",	0x0f090},
    {"zheng",	0x0f0b0},
    {"zhi",	0x0f0d0},
    {"zhong",	0x0f1e0},
    {"zhou",	0x0f070},
    {"zhu",	0x0f160},
    {"zhua",	0x0f170},
    {"zhuai",	0x0f190},
    {"zhuan",	0x0f1b0},
    {"zhuang",	0x0f1d0},
    {"zhui",	0x0f1a0},
    {"zhun",	0x0f1c0},
    {"zhuo",	0x0f180},
    {"zi",	0x130d0},
    {"zong",	0x131e0},
    {"zou",	0x13070},
    {"zu",	0x13160},
    {"zuan",	0x131b0},
    {"zui",	0x131a0},
    {"zun",	0x131c0},
    {"zuo",	0x13180},
};

static int 
pytab_entry_compare (const char *s, TPyTabEntry *v)
  {return strcmp (s, v->pystr);}

TSyllable
CPinyinData::encodeSyllable (const char *pinyin)
{
    typedef int (*bsearch_compare) (const void*, const void*);
    TPyTabEntry *e = (TPyTabEntry*) bsearch (pinyin, pinyin_table, 
                                           sizeof(pinyin_table)/sizeof(pinyin_table[0]), 
                                           sizeof(pinyin_table[0]), 
                                           (bsearch_compare) pytab_entry_compare);
    if (e)
        return e->id;

    return 0;
}

const char * 
CPinyinData::decodeSyllable (TSyllable s, const char **i, const char **f)
{
    if (i) *i = initials[s.initial];
    if (f) *f = finals[s.final];

    static char buf[128];
    snprintf (buf, sizeof(buf), "%s%s", initials[s.initial], finals[s.final]);

    typedef int (*bsearch_compare) (const void*, const void*);
    TPyTabEntry *e = (TPyTabEntry*) bsearch (buf, pinyin_table, 
                                           sizeof(pinyin_table)/sizeof(pinyin_table[0]), 
                                           sizeof(pinyin_table[0]), 
                                           (bsearch_compare) pytab_entry_compare);

    if (e)
        return e->pystr;

    return NULL;
}

const char **
CPinyinData::getAutoCorrectionPairs (unsigned &num)
{
    num = num_auto_correction_pairs;
    return auto_correction_pairs;
}

const char **
CPinyinData::getFuzzyPairs (unsigned &num)
{
    num = num_fuzzy_pairs;
    return fuzzy_pairs;
}

const char **
CPinyinData::getInitials (unsigned &num)
{
    num = num_initials;
    return initials;
}

const char **
CPinyinData::getFinals (unsigned &num)
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
CPinyinData::getInnerFuzzyFinalMap (unsigned &num)
{
    num = num_fuzzy_finals;
    return fuzzy_finals_map;
}

void
CPinyinData::getFuzzyPreProSyllables (const unsigned **pre_syls, const unsigned **pro_syls)
{
    *pre_syls = fuzzy_pre_syllables;
    *pro_syls = fuzzy_pro_syllables;
}

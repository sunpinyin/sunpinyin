#!/usr/bin/python 

# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
# 
# Copyright (c) 2007 Sun Microsystems, Inc. All Rights Reserved.
# 
# The contents of this file are subject to the terms of either the GNU Lesser
# General Public License Version 2.1 only ("LGPL") or the Common Development and
# Distribution License ("CDDL")(collectively, the "License"). You may not use this
# file except in compliance with the License. You can obtain a copy of the CDDL at
# http://www.opensource.org/licenses/cddl1.php and a copy of the LGPLv2.1 at
# http://www.opensource.org/licenses/lgpl-license.php. See the License for the 
# specific language governing permissions and limitations under the License. When
# distributing the software, include this License Header Notice in each file and
# include the full text of the License in the License file as well as the
# following notice:
# 
# NOTICE PURSUANT TO SECTION 9 OF THE COMMON DEVELOPMENT AND DISTRIBUTION LICENSE
# (CDDL)
# For Covered Software in this distribution, this License shall be governed by the
# laws of the State of California (excluding conflict-of-law provisions).
# Any litigation relating to this License shall be subject to the jurisdiction of
# the Federal Courts of the Northern District of California and the state courts
# of the State of California, with venue lying in Santa Clara County, California.
# 
# Contributor(s):
# 
# If you wish your version of this file to be governed by only the CDDL or only
# the LGPL Version 2.1, indicate your decision by adding "[Contributor]" elects to
# include this software in this distribution under the [CDDL or LGPL Version 2.1]
# license." If you don't indicate a single choice of license, a recipient has the
# option to distribute your version of this file under either the CDDL or the LGPL
# Version 2.1, or to extend the choice of license to its licensees as provided
# above. However, if you add LGPL Version 2.1 code and therefore, elected the LGPL
# Version 2 license, then the option applies only if the new code is made subject
# to such option by the copyright holder. 

initials = ["", "b", "p", "m", "f", "d", "t", "n", "l", "g", "k", "h", "j", "q", "x", "zh", "ch", "sh", "r", "z", "c", "s", "y", "w", ]

finals = ["", "a", "o", "e", "ai", "ei", "ao", "ou", "an", "en", "ang", "eng", "er", "i", "ia", "ie", "iao", "iu", "ian", "in", "iang", "ing", "u", "ua", "uo", "uai", "ui", "uan", "un", "uang", "ong", "v", "ue", "iong", ]

inner_fuzzy_finals = ['ia', 'iao', 'ian', 'iang', 'ie', 'ua', 'uai', 'uan', 'uang', 'ue']

fuzzy_pairs = [
    ('z',       'zh'), 
    ('c',       'ch'), 
    ('s',       'sh'), 
    ('an',      'ang'), 
    ('on',      'ong'), 
    ('en',      'eng'), 
    ('in',      'ing'), 
    ('eng',     'ong'), 
    ('ian',     'iang'), 
    ('uan',     'uang'), 
    ('l',       'n'), 
    ('f',       'h'), 
    ('r',       'l'), 
    ('k',       'g'),
]

auto_correction_pairs = {
    'ign':      'ing',
    'img':      'ing',
    'uei':      'ui',
    'uen':      'un',
    'iou':      'iu',
}

valid_syllables = {
    "a":        0x00010,
    "ai":       0x00040,
    "an":       0x00080,
    "ang":      0x000a0,
    "ao":       0x00060,
    "b":        0x01000,
    "ba":       0x01010,
    "bai":      0x01040,
    "ban":      0x01080,
    "bang":     0x010a0,
    "bao":      0x01060,
    "bei":      0x01050,
    "ben":      0x01090,
    "beng":     0x010b0,
    "bi":       0x010d0,
    "bian":     0x01120,
    "biao":     0x01100,
    "bie":      0x010f0,
    "bin":      0x01130,
    "bing":     0x01150,
    "bo":       0x01020,
    "bu":       0x01160,
    "c":        0x14000,
    "ca":       0x14010,
    "cai":      0x14040,
    "can":      0x14080,
    "cang":     0x140a0,
    "cao":      0x14060,
    "ce":       0x14030,
    "cei":      0x14050,
    "cen":      0x14090,
    "ceng":     0x140b0,
    "ch":       0x10000,
    "cha":      0x10010,
    "chai":     0x10040,
    "chan":     0x10080,
    "chang":    0x100a0,
    "chao":     0x10060,
    "che":      0x10030,
    "chen":     0x10090,
    "cheng":    0x100b0,
    "chi":      0x100d0,
    "chong":    0x101e0,
    "chou":     0x10070,
    "chu":      0x10160,
    "chua":     0x10170,
    "chuai":    0x10190,
    "chuan":    0x101b0,
    "chuang":   0x101d0,
    "chui":     0x101a0,
    "chun":     0x101c0,
    "chuo":     0x10180,
    "ci":       0x140d0,
    "cong":     0x141e0,
    "cou":      0x14070,
    "cu":       0x14160,
    "cuan":     0x141b0,
    "cui":      0x141a0,
    "cun":      0x141c0,
    "cuo":      0x14180,
    "d":        0x05000,
    "da":       0x05010,
    "dai":      0x05040,
    "dan":      0x05080,
    "dang":     0x050a0,
    "dao":      0x05060,
    "de":       0x05030,
    "dei":      0x05050,
    "den":      0x05090,
    "deng":     0x050b0,
    "di":       0x050d0,
    "dia":      0x050e0,
    "dian":     0x05120,
    "diao":     0x05100,
    "die":      0x050f0,
    "ding":     0x05150,
    "diu":      0x05110,
    "dong":     0x051e0,
    "dou":      0x05070,
    "du":       0x05160,
    "duan":     0x051b0,
    "dui":      0x051a0,
    "dun":      0x051c0,
    "duo":      0x05180,
    "e":        0x00030,
    "ei":       0x00050,
    "en":       0x00090,
    "eng":      0x000b0,
    "er":       0x000c0,
    "f":        0x04000,
    "fa":       0x04010,
    "fan":      0x04080,
    "fang":     0x040a0,
    "fei":      0x04050,
    "fen":      0x04090,
    "feng":     0x040b0,
    "fiao":     0x04100,
    "fo":       0x04020,
    "fou":      0x04070,
    "fu":       0x04160,
    "g":        0x09000,
    "ga":       0x09010,
    "gai":      0x09040,
    "gan":      0x09080,
    "gang":     0x090a0,
    "gao":      0x09060,
    "ge":       0x09030,
    "gei":      0x09050,
    "gen":      0x09090,
    "geng":     0x090b0,
    "gong":     0x091e0,
    "gou":      0x09070,
    "gu":       0x09160,
    "gua":      0x09170,
    "guai":     0x09190,
    "guan":     0x091b0,
    "guang":    0x091d0,
    "gui":      0x091a0,
    "gun":      0x091c0,
    "guo":      0x09180,
    "h":        0x0b000,
    "ha":       0x0b010,
    "hai":      0x0b040,
    "han":      0x0b080,
    "hang":     0x0b0a0,
    "hao":      0x0b060,
    "he":       0x0b030,
    "hei":      0x0b050,
    "hen":      0x0b090,
    "heng":     0x0b0b0,
    "hong":     0x0b1e0,
    "hou":      0x0b070,
    "hu":       0x0b160,
    "hua":      0x0b170,
    "huai":     0x0b190,
    "huan":     0x0b1b0,
    "huang":    0x0b1d0,
    "hui":      0x0b1a0,
    "hun":      0x0b1c0,
    "huo":      0x0b180,
    "j":        0x0c000,
    "ji":       0x0c0d0,
    "jia":      0x0c0e0,
    "jian":     0x0c120,
    "jiang":    0x0c140,
    "jiao":     0x0c100,
    "jie":      0x0c0f0,
    "jin":      0x0c130,
    "jing":     0x0c150,
    "jiong":    0x0c210,
    "jiu":      0x0c110,
    "ju":       0x0c160,
    "juan":     0x0c1b0,
    "jue":      0x0c200,
    "jun":      0x0c1c0,
    "k":        0x0a000,
    "ka":       0x0a010,
    "kai":      0x0a040,
    "kan":      0x0a080,
    "kang":     0x0a0a0,
    "kao":      0x0a060,
    "ke":       0x0a030,
    "kei":      0x0a050,
    "ken":      0x0a090,
    "keng":     0x0a0b0,
    "kong":     0x0a1e0,
    "kou":      0x0a070,
    "ku":       0x0a160,
    "kua":      0x0a170,
    "kuai":     0x0a190,
    "kuan":     0x0a1b0,
    "kuang":    0x0a1d0,
    "kui":      0x0a1a0,
    "kun":      0x0a1c0,
    "kuo":      0x0a180,
    "l":        0x08000,
    "la":       0x08010,
    "lai":      0x08040,
    "lan":      0x08080,
    "lang":     0x080a0,
    "lao":      0x08060,
    "le":       0x08030,
    "lei":      0x08050,
    "leng":     0x080b0,
    "li":       0x080d0,
    "lia":      0x080e0,
    "lian":     0x08120,
    "liang":    0x08140,
    "liao":     0x08100,
    "lie":      0x080f0,
    "lin":      0x08130,
    "ling":     0x08150,
    "liu":      0x08110,
    "lo":       0x08020,
    "long":     0x081e0,
    "lou":      0x08070,
    "lu":       0x08160,
    "luan":     0x081b0,
    "lue":      0x08200,
    "lun":      0x081c0,
    "luo":      0x08180,
    "lv":       0x081f0,
    "m":        0x03000,
    "ma":       0x03010,
    "mai":      0x03040,
    "man":      0x03080,
    "mang":     0x030a0,
    "mao":      0x03060,
    "me":       0x03030,
    "mei":      0x03050,
    "men":      0x03090,
    "meng":     0x030b0,
    "mi":       0x030d0,
    "mian":     0x03120,
    "miao":     0x03100,
    "mie":      0x030f0,
    "min":      0x03130,
    "ming":     0x03150,
    "miu":      0x03110,
    "mo":       0x03020,
    "mou":      0x03070,
    "mu":       0x03160,
    "n":        0x07000,
    "na":       0x07010,
    "nai":      0x07040,
    "nan":      0x07080,
    "nang":     0x070a0,
    "nao":      0x07060,
    "ne":       0x07030,
    "nei":      0x07050,
    "nen":      0x07090,
    "neng":     0x070b0,
    "ni":       0x070d0,
    "nian":     0x07120,
    "niang":    0x07140,
    "niao":     0x07100,
    "nie":      0x070f0,
    "nin":      0x07130,
    "ning":     0x07150,
    "niu":      0x07110,
    "nong":     0x071e0,
    "nou":      0x07070,
    "nu":       0x07160,
    "nuan":     0x071b0,
    "nue":      0x07200,
    "nun":      0x071c0,
    "nuo":      0x07180,
    "nv":       0x071f0,
    "o":        0x00020,
    "ou":       0x00070,
    "p":        0x02000,
    "pa":       0x02010,
    "pai":      0x02040,
    "pan":      0x02080,
    "pang":     0x020a0,
    "pao":      0x02060,
    "pei":      0x02050,
    "pen":      0x02090,
    "peng":     0x020b0,
    "pi":       0x020d0,
    "pian":     0x02120,
    "piao":     0x02100,
    "pie":      0x020f0,
    "pin":      0x02130,
    "ping":     0x02150,
    "po":       0x02020,
    "pou":      0x02070,
    "pu":       0x02160,
    "q":        0x0d000,
    "qi":       0x0d0d0,
    "qia":      0x0d0e0,
    "qian":     0x0d120,
    "qiang":    0x0d140,
    "qiao":     0x0d100,
    "qie":      0x0d0f0,
    "qin":      0x0d130,
    "qing":     0x0d150,
    "qiong":    0x0d210,
    "qiu":      0x0d110,
    "qu":       0x0d160,
    "quan":     0x0d1b0,
    "que":      0x0d200,
    "qun":      0x0d1c0,
    "r":        0x12000,
    "ran":      0x12080,
    "rang":     0x120a0,
    "rao":      0x12060,
    "re":       0x12030,
    "ren":      0x12090,
    "reng":     0x120b0,
    "ri":       0x120d0,
    "rong":     0x121e0,
    "rou":      0x12070,
    "ru":       0x12160,
    "ruan":     0x121b0,
    "rui":      0x121a0,
    "run":      0x121c0,
    "ruo":      0x12180,
    "s":        0x15000,
    "sa":       0x15010,
    "sai":      0x15040,
    "san":      0x15080,
    "sang":     0x150a0,
    "sao":      0x15060,
    "se":       0x15030,
    "sen":      0x15090,
    "seng":     0x150b0,
    "sh":       0x11000,
    "sha":      0x11010,
    "shai":     0x11040,
    "shan":     0x11080,
    "shang":    0x110a0,
    "shao":     0x11060,
    "she":      0x11030,
    "shei":     0x11050,
    "shen":     0x11090,
    "sheng":    0x110b0,
    "shi":      0x110d0,
    "shou":     0x11070,
    "shu":      0x11160,
    "shua":     0x11170,
    "shuai":    0x11190,
    "shuan":    0x111b0,
    "shuang":   0x111d0,
    "shui":     0x111a0,
    "shun":     0x111c0,
    "shuo":     0x11180,
    "si":       0x150d0,
    "song":     0x151e0,
    "sou":      0x15070,
    "su":       0x15160,
    "suan":     0x151b0,
    "sui":      0x151a0,
    "sun":      0x151c0,
    "suo":      0x15180,
    "t":        0x06000,
    "ta":       0x06010,
    "tai":      0x06040,
    "tan":      0x06080,
    "tang":     0x060a0,
    "tao":      0x06060,
    "te":       0x06030,
    "tei":      0x06050,
    "teng":     0x060b0,
    "ti":       0x060d0,
    "tian":     0x06120,
    "tiao":     0x06100,
    "tie":      0x060f0,
    "ting":     0x06150,
    "tong":     0x061e0,
    "tou":      0x06070,
    "tu":       0x06160,
    "tuan":     0x061b0,
    "tui":      0x061a0,
    "tun":      0x061c0,
    "tuo":      0x06180,
    "w":        0x17000,
    "wa":       0x17010,
    "wai":      0x17040,
    "wan":      0x17080,
    "wang":     0x170a0,
    "wei":      0x17050,
    "wen":      0x17090,
    "weng":     0x170b0,
    "wo":       0x17020,
    "wu":       0x17160,
    "x":        0x0e000,
    "xi":       0x0e0d0,
    "xia":      0x0e0e0,
    "xian":     0x0e120,
    "xiang":    0x0e140,
    "xiao":     0x0e100,
    "xie":      0x0e0f0,
    "xin":      0x0e130,
    "xing":     0x0e150,
    "xiong":    0x0e210,
    "xiu":      0x0e110,
    "xu":       0x0e160,
    "xuan":     0x0e1b0,
    "xue":      0x0e200,
    "xun":      0x0e1c0,
    "y":        0x16000,
    "ya":       0x16010,
    "yai":      0x16040,
    "yan":      0x16080,
    "yang":     0x160a0,
    "yao":      0x16060,
    "ye":       0x16030,
    "yi":       0x160d0,
    "yin":      0x16130,
    "ying":     0x16150,
    "yo":       0x16020,
    "yong":     0x161e0,
    "you":      0x16070,
    "yu":       0x16160,
    "yuan":     0x161b0,
    "yue":      0x16200,
    "yun":      0x161c0,
    "z":        0x13000,
    "za":       0x13010,
    "zai":      0x13040,
    "zan":      0x13080,
    "zang":     0x130a0,
    "zao":      0x13060,
    "ze":       0x13030,
    "zei":      0x13050,
    "zen":      0x13090,
    "zeng":     0x130b0,
    "zh":       0x0f000,
    "zha":      0x0f010,
    "zhai":     0x0f040,
    "zhan":     0x0f080,
    "zhang":    0x0f0a0,
    "zhao":     0x0f060,
    "zhe":      0x0f030,
    "zhei":     0x0f050,
    "zhen":     0x0f090,
    "zheng":    0x0f0b0,
    "zhi":      0x0f0d0,
    "zhong":    0x0f1e0,
    "zhou":     0x0f070,
    "zhu":      0x0f160,
    "zhua":     0x0f170,
    "zhuai":    0x0f190,
    "zhuan":    0x0f1b0,
    "zhuang":   0x0f1d0,
    "zhui":     0x0f1a0,
    "zhun":     0x0f1c0,
    "zhuo":     0x0f180,
    "zi":       0x130d0,
    "zong":     0x131e0,
    "zou":      0x13070,
    "zu":       0x13160,
    "zuan":     0x131b0,
    "zui":      0x131a0,
    "zun":      0x131c0,
    "zuo":      0x13180,
}

def decode_syllable (s):
    return initials[(s>>12)], finals[(s&0x00ff0)>>4]

def init_fuzzy_map (fuzzy_pairs):
    fuzzy_map = {}
    for i, j in fuzzy_pairs:
        fuzzy_map.setdefault (i, []).append (j)
        fuzzy_map.setdefault (j, []).append (i)

    return fuzzy_map

fuzzy_map = init_fuzzy_map (fuzzy_pairs)

def get_fuzzy_syllables (syllable):
    i, f = decode_syllable (syllable)
    iset = fuzzy_map.setdefault(i, []) + [i]
    fset = fuzzy_map.setdefault(f, []) + [f]
    sset = [valid_syllables[i+f] for i in iset for f in fset if i+f in valid_syllables]
    sset.remove (syllable)
    return sset

def gen_suffix_trie (fname):
    from trie import Trie, DATrie
    
    trie = Trie ()
    pytrie = DATrie ()

    for s in valid_syllables:
        trie.add (s[::-1], valid_syllables[s])
    
    pytrie.construct_from_trie (trie)
    pytrie.output_static_c_arrays (fname)

def gen_fuzzy_syllable_pairs_tables ():
    fuzzy_pro_syllables = [s for s in valid_syllables if s[1:] in valid_syllables and s[0] in initials and s not in initials]
    fuzzy_pre_syllables = [s for s in valid_syllables if s[:-1] in valid_syllables and s[-1] in initials and s not in initials]

    initial_sets = set([s[0] for s in fuzzy_pro_syllables]) & set([s[-1] for s in fuzzy_pre_syllables])

    fuzzy_pro_syllables  = [s for s in fuzzy_pro_syllables if s[0] in initial_sets]
    fuzzy_pre_syllables  = [s for s in fuzzy_pre_syllables if s[-1] in initial_sets]

    print "static const unsigned fuzzy_pre_syllables [] = {"
    for s in fuzzy_pre_syllables:
        print "    %-12s %-12s %-12s /* %s */" % ("0x%05x," % valid_syllables[s[:-1]], "'%s'," % s[-1], "0x%05x," % valid_syllables[s], s)
    print "    0x0,"
    print "};\n"

    print "static const unsigned fuzzy_pro_syllables [] = {"
    for s in fuzzy_pro_syllables:
        print "    %-12s %-12s %-12s /* %s */" % ("0x%05x," % valid_syllables[s], "'%s'," % s[0], "0x%05x," % valid_syllables[s[1:]], s)
    print "    0x0,"
    print "};\n"

def gen_inner_fuzzy_syllable_tables ():
    print "static const unsigned fuzzy_finals_map[] = {"
    for s in inner_fuzzy_finals:
        print "    %-12s %-12s %-12s /* %-4s -> %-4s len %d */" % ("0x%02x," % finals.index(s), "0x%02x," % valid_syllables[s[1:]], "%d," % (len(s)-1,),  s, s[1:], len(s)-1)
    print "};\n"

if __name__ == "__main__":
    gen_suffix_trie ("../src/pinyin/quanpin_trie.h")
    gen_inner_fuzzy_syllable_tables ()
    gen_fuzzy_syllable_pairs_tables ()

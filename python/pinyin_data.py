#!/usr/bin/python3

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

initials = ["", "b", "p", "m", "f", "d", "t", "n", "l", "g", "k", "h", "j", "q", "x", "zh", "ch", "sh", "r", "z", "c", "s", "y", "w", "hm", ]

finals = ["", "a", "o", "e", "ai", "ei", "ao", "ou", "an", "en", "ang", "eng", "er", "i", "ia", "ie", "iao", "iu", "ian", "in", "iang", "ing", "u", "ua", "uo", "uai", "ui", "uan", "un", "uang", "ong", "v", "ve", "ue", "iong", "ng" ]

inner_fuzzy_finals = ['ia', 'iao', 'ian', 'iang', 'ie', 'ua', 'uai', 'uan', 'uang', 'ue', 've']

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

valid_init_fin_pairs = [
    ([""],      ["a", "ai", "an", "ang", "ao", "e", "ei", "en", "eng", "er", "o", "ou", "ng"]),
    (["b"],     ["", "a", "ai", "an", "ang", "ao", "ei", "en", "eng", "i", "ian", "iao", "ie", "in", "ing", "o", "u"]),
    (["p"],     ["", "a", "ai", "an", "ang", "ao", "ei", "en", "eng", "i", "ian", "iao", "ie", "in", "ing", "o", "ou", "u"]),
    (["m"],     ["", "a", "ai", "an", "ang", "ao", "e", "ei", "en", "eng", "i", "ian", "iao", "ie", "in", "ing", "iu", "o", "ou", "u"]),
    (["f"],     ["", "a", "an", "ang", "ei", "en", "eng", "iao", "o", "ou", "u"]),
    (["d"],     ["", "a", "ai", "an", "ang", "ao", "e", "ei", "en", "eng", "i", "ia", "ian", "iao", "ie", "ing", "iu", "ong", "ou", "u", "uan", "ui", "un", "uo"]),
    (["t"],     ["", "a", "ai", "an", "ang", "ao", "e", "ei", "eng", "i", "ian", "iao", "ie", "ing", "ong", "ou", "u", "uan", "ui", "un", "uo"]),
    (["n"],     ["", "a", "ai", "an", "ang", "ao", "e", "ei", "en", "eng", "i", "ian", "iang", "iao", "ie", "in", "ing", "iu", "ong", "ou", "u", "uan", "un", "uo", "v", "ve|ue"]),
    (["l"],     ["", "a", "ai", "an", "ang", "ao", "e", "ei", "eng", "i", "ia", "ian", "iang", "iao", "ie", "in", "ing", "iu", "o", "ong", "ou", "u", "uan", "un", "uo", "v", "ve|ue"]),
    (["g", "k", "h"],   ["", "a", "ai", "an", "ang", "ao", "e", "ei", "en", "eng", "ong", "ou", "u", "ua", "uai", "uan", "uang", "ui", "un", "uo"]),
    (["j", "q", "x"],    ["", "i", "ia", "ian", "iang", "iao", "ie", "in", "ing", "iong", "iu", "u", "uan", "ue", "un"]),
    (["zh"],    ["", "a", "ai", "an", "ang", "ao", "e", "ei", "en", "eng", "i", "ong", "ou", "u", "ua", "uai", "uan", "uang", "ui", "un", "uo"]),
    (["ch"],    ["", "a", "ai", "an", "ang", "ao", "e", "en", "eng", "i", "ong", "ou", "u", "ua", "uai", "uan", "uang", "ui", "un", "uo"]),
    (["sh"],    ["", "a", "ai", "an", "ang", "ao", "e", "ei", "en", "eng", "i", "ou", "u", "ua", "uai", "uan", "uang", "ui", "un", "uo"]),
    (["r"],     ["", "an", "ang", "ao", "e", "en", "eng", "i", "ong", "ou", "u", "ua", "uan", "ui", "un", "uo"]),
    (["z"],     ["", "a", "ai", "an", "ang", "ao", "e", "ei", "en", "eng", "i", "ong", "ou", "u", "uan", "ui", "un", "uo"]),
    (["c"],     ["", "a", "ai", "an", "ang", "ao", "e", "ei", "en", "eng", "i", "ong", "ou", "u", "uan", "ui", "un", "uo"]),
    (["s"],     ["", "a", "ai", "an", "ang", "ao", "e", "en", "eng", "i", "ong", "ou", "u", "uan", "ui", "un", "uo"]),
    (["y"],     ["", "a", "an", "ang", "ao", "e", "i", "in", "ing", "o", "ong", "ou", "u", "uan", "ue", "un"]),
    (["w"],     ["", "a", "ai", "an", "ang", "ei", "en", "eng", "o", "u"]),
    (["hm"],    [""]),
]

valid_syllables = {}

for (ii, ff) in valid_init_fin_pairs:
    for i in ii:
        for f in ff:
            fv = f.split('|')
            for fi in fv:
                valid_syllables[i+fi] = (initials.index(i) << 12) + (finals.index(fv[0]) << 4)

fuzzy_map = {}
for i, j in fuzzy_pairs:
    fuzzy_map.setdefault (i, []).append(j)
    fuzzy_map.setdefault (j, []).append(i)

fuzzy_pro_syllables = [s for s in valid_syllables if s[1:] in valid_syllables and s[0] in initials and s not in initials]
fuzzy_pre_syllables = [s for s in valid_syllables if s[:-1] in valid_syllables and s[-1] in initials and s not in initials]
initial_sets = {s[0] for s in fuzzy_pro_syllables} & {s[-1] for s in fuzzy_pre_syllables}
fuzzy_pro_syllables  = [s for s in fuzzy_pro_syllables if s[0] in initial_sets]
fuzzy_pre_syllables  = [s for s in fuzzy_pre_syllables if s[-1] in initial_sets]

def decode_syllable (s):
    return initials[(s >> 12)], finals[(s & 0x00ff0) >> 4]

def get_fuzzy_syllables (syllable):
    i, f = decode_syllable (syllable)
    ii = fuzzy_map.setdefault (i, []) + [i]
    ff = fuzzy_map.setdefault (f, []) + [f]
    sset = [valid_syllables[init + fin] for i in ii for f in ff if i + f in valid_syllables]
    sset.remove (syllable)
    return sset

# -*- indent-tabs-mode: nil -*- vim:et:ts=4

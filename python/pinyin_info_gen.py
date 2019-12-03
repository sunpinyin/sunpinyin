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

from pinyin_data import *

def fmt_str_array (name, var) :
    print('static const char *%s[] = { %s };' % (name, ', '.join ('"%s"' % s for s in var)))

def fmt_array_size (name):
    print('static const unsigned num_%s = sizeof(%s) / sizeof(*%s);' % (name, name, name))

def fmt_str_pair_array (name, var) :
    print('static const char *%s[] = {' % name)
    for s1, s2 in var:
        print('    %-7s %s' % ('"%s",' % s1, '"%s",' % s2))
    print('};')

def fmt_pair_array_size (name):
    print('static const unsigned num_%s = sizeof(%s) / sizeof(*%s) / 2;' % (name, name, name))

fmt_str_array ('initials', initials)
fmt_array_size ('initials')
print('')

fmt_str_array('finals', finals)
fmt_array_size ('finals')
print('')

fmt_str_array('fuzzy_finals', inner_fuzzy_finals)
fmt_array_size ('fuzzy_finals')
print('')

fmt_str_pair_array ('fuzzy_pairs', fuzzy_pairs)
fmt_pair_array_size ('fuzzy_pairs')
print('')

fmt_str_pair_array ('auto_correction_pairs', sorted(auto_correction_pairs.items()))
fmt_pair_array_size ('auto_correction_pairs')
print('')

print('static const unsigned fuzzy_finals_map [] = {')
for s in inner_fuzzy_finals:
    print('    %-7s %-7s %-7s /* %-4s -> %-4s len %d */' % ('0x%02x,' % finals.index(s), '0x%02x,' % valid_syllables[s[1:]], '%d,' % (len(s)-1,),  s, s[1:], len(s)-1))
print('};\n')

print('static const TPyTabEntry pinyin_table[] = {')
for syllable, hex_syllable in sorted(valid_syllables.items()):
    print('    { %-9s %s },' % ('"%s",' % syllable, '0x%05x' % hex_syllable))
print('};\n')

print('static const unsigned fuzzy_pre_syllables [] = {')
for s in fuzzy_pre_syllables:
    print('    %-11s %-7s %-11s /* %s */' % ('0x%05x,' % valid_syllables[s[:-1]], "'%s'," % s[-1], '0x%05x,' % valid_syllables[s], s))
print('    0x0,')
print('};\n')

print('static const unsigned fuzzy_pro_syllables [] = {')
for s in fuzzy_pro_syllables:
    print('    %-11s %-7s %-11s /* %s */' % ('0x%05x,' % valid_syllables[s], "'%s'," % s[0], '0x%05x,' % valid_syllables[s[1:]], s))
print('    0x0,')
print('};\n')

# -*- indent-tabs-mode: nil -*- vim:et:ts=4

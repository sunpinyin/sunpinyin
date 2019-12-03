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

import sys
import getopt
import codecs
import struct

from imdict import IMDict
from trie import match_longest, get_ambiguious_length
from utils import read_ch_sentences

def usage():
    print('''
Usage:
mmseg.py -d dict_file [-f (text|bin)] [-i] [-s STOK_ID] [-a AMBI_ID] corpus_file

  -d --dict:
    The dictionary file (in UTF-8 encoding) to be used.
  -f --format:
    Output format, can be 'text' or 'bin'. Default is 'bin'.
    Normally, in text mode, word text are output, while in binary mode,
    the integer of the word-ids are writed to stdout.
  -i --show-id:
    Show Id info. In text output format, attach id after known words. 
  -s --stok-id:
    Sentence token id. Default 10.
    It will be write to output in binary mode after every sentence.
  -a --ambi-id:
    Ambiguious means ABC => A BC or AB C. If specified (AMBI-ID != 0), 
    The sequence ABC will not be segmented, in binary mode, the AMBI-ID 
    is written out; in text mode, <ambi>ABC</ambi> will be output. Default 
    is 9.
''')

options={'show-id':       False, 
         'format' :       'bin', 
         'stok-id':       10,
         'ambi-id':       9}

def parse_options(args):
    try:
        opts, args = getopt.getopt(args, "hid:f:s:a:", ["help", "show-id", "dict=", "format=", "stok-id=", "ambi-id="])
    except getopt.GetoptError as err:
        print(str(err))
        sys.exit(1)

    for opt,val in opts:
        if opt in ('-h', '--help'):
            usage()
            sys.exit()
        elif opt in ('-d', '--dict'):
            options['dict'] = val
        elif opt in ('-i', '--show-id'):
            options['show-id'] = True
        elif opt in ('-f', '--format'):
            if val in ('bin', 'text'):
                options['format'] = val
        elif opt in ('-s', '--stok-id'):
            options['stok-id'] = int(val)
        elif opt in ('-val', '--ambi-id'):
            options['ambi-id'] = int(val)

    if 'dict' not in options:
        usage()
        sys.exit(1)

    if args:
        options['corpus'] = args[0]

def output_word(wid, word):
    if options['format'] == 'text':
        if wid == options['ambi-id']:
            word = '<ambi>'+word+'</ambi>'
        if options['show-id']:
            word = word+'('+str(wid)+')'
        sys.stdout.write('%s ' % word.encode('UTF-8'))
    else:
        sys.stdout.write(struct.pack('I', wid))

def process_file(file, dict):
    for line in read_ch_sentences(file):
        print(line.encode('UTF-8'), file=sys.stderr)
        length = len(line)
        i = 0
        while (i < length):
            strbuf = line[i:]
            wid, l = match_longest(dict, strbuf)
            if wid == 0:
                l = 1
            else:
                ambi_len = get_ambiguious_length(dict, strbuf, l)
                if ambi_len > l:
                    wid, l = options['ambi-id'], ambi_len

            output_word (wid, strbuf[:l])
            i += l

        output_word (options['stok-id'], '\n')

if __name__ == "__main__":
    parse_options(sys.argv[1:])

    dict = IMDict(options['dict'])
    
    try:    file = codecs.open(options['corpus'], "r", "UTF-8")
    except: file = codecs.getreader('UTF-8')(sys.stdin)

    process_file (file, dict)
    file.close()

# -*- indent-tabs-mode: nil -*- vim:et:ts=4

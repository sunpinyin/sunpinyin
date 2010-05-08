#!/usr/bin/python

import os, sys
import codecs
from importer import import_to_sunpinyin_user_dict

def load_google_user_dict (fname):
    result = []
    f = codecs.open (fname, "r", "GB18030")
    for l in f:
        v = l.strip().split()
        utf8str = v[0]

        try:
            freq = int(v[1])
            pystr = '\''.join (v[2:])
        except:
            pystr = '\''.join (v[1:])

        result.append ((pystr, utf8str))

    return result

def main ():
    if len (sys.argv) != 2:
        print "Please specify the Google Pinyin exported user dict file!"
        exit (1)

    google_user_dict = load_google_user_dict(sys.argv[1])
    import_to_sunpinyin_user_dict (google_user_dict)

if __name__ == "__main__":
    main()

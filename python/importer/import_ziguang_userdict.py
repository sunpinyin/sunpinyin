#!/usr/bin/python

import os, sys
import codecs
from importer import import_to_sunpinyin_user_dict

def load_ziguang_user_dict (fname):
    result = []
    f = codecs.open (fname, "r", "UTF-16")
    for l in f:
        if l[0] in ('\n', '\r'):
            continue

        try:
            utf8str, pystr, _ = l.strip().split("\t")
            result.append ((pystr, utf8str))
        except:
            pass

    return result

def main ():
    if len (sys.argv) != 2:
        print "Please specify the ZiGuang PinYin exported user dict file!"
        exit (1)

    ziguang_user_dict = load_ziguang_user_dict(sys.argv[1])
    import_to_sunpinyin_user_dict (ziguang_user_dict)

if __name__ == "__main__":
    main()

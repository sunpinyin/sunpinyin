#!/usr/bin/python3

import os, sys
import codecs
from importer import import_to_sunpinyin_user_dict

def load_qq_user_dict (fname):
    result = []
    f = codecs.open (fname, "r", "UTF-16")
    for l in f:
        if l[0] in ('\n', '\r'):
            continue

        try:
            pystr, utf8str, _ = l.strip().split(",")[0].split(" ")
            result.append ((pystr[1:], utf8str))
        except:
            pass

    return result

def main ():
    if len (sys.argv) != 2:
        print("Please specify the QQ PinYin exported user dict file!")
        exit (1)

    qq_user_dict = load_qq_user_dict(sys.argv[1])
    import_to_sunpinyin_user_dict (qq_user_dict)

if __name__ == "__main__":
    main()

# -*- indent-tabs-mode: nil -*- vim:et:ts=4

#!/usr/bin/python

import os, sys
import codecs
from importer import import_to_sunpinyin_user_dict

def load_sogou_user_dict (fname):
    result = []
    f = codecs.open (fname, "r", "UTF-16")

    for l in f:
        if l[0] in (';', '\n', '\r'):
            continue

        pystr, utf8str = l.strip().split(" ")
        result.append ((pystr[1:], utf8str))

    return result

def main ():
    if len (sys.argv) != 2:
        print "Please specify the Sogou PinYin exported user dict file!"
        exit (1)

    sogou_user_dict = load_sogou_user_dict(sys.argv[1])
    import_to_sunpinyin_user_dict (sogou_user_dict)

if __name__ == "__main__":
    main()

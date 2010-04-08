#!/usr/bin/python

import os, sys
import codecs
from importer import import_to_sunpinyin_user_dict

def load_qim_user_dict (fname):
    result = []
    f = codecs.open (fname, "r", "UTF-8")
    for l in f:
        pystr, utf8str = l.strip().split(" ")
        if 'P' in pystr:
            continue

        result.append ((pystr, utf8str))

    return result

def main ():
    if len (sys.argv) != 2:
        print "Please specify the QIM exported user dict file!"
        exit (1)

    qim_user_dict = load_qim_user_dict(sys.argv[1])
    import_to_sunpinyin_user_dict (qim_user_dict)

if __name__ == "__main__":
    main()

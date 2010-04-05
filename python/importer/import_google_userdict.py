#!/usr/bin/python

import os, sys
import codecs
from importer import import_to_sunpinyin_user_dict

homedir = os.environ.get("HOME")

def load_google_user_dict (fname):
    result = []
    f = codecs.open (fname, "r", "GB18030")
    for l in f:
        utf8str, _, pystr = l.strip().split("\t")
        pystr = '\''.join (pystr.split(' '))
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

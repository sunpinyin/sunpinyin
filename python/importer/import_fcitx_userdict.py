#!/usr/bin/python

import os, sys
import codecs
from importer import import_to_sunpinyin_user_dict

def load_fcitx_user_dict (fname):
    result = []
    f = codecs.open (fname, "r", "GB18030")
    for l in f:
        if l[0] in ('\n'):
            continue

        pystr, utf8str = l.strip().split(" ")
        result.append ((pystr, utf8str))

    return result

def main ():
    if len (sys.argv) != 2:
        os.system ("/usr/bin/mb2org ~/.fcitx/pyusrphrase.mb > /tmp/fcitx_userdict_gb.txt")
        fcitx_user_dict = load_fcitx_user_dict("/tmp/fcitx_userdict_gb.txt")
        os.system ("rm -rf /tmp/fcitx_userdict_gb.txt")
    else:
        fcitx_user_dict = load_fcitx_user_dict(sys.argv[1])

    import_to_sunpinyin_user_dict (fcitx_user_dict)

if __name__ == "__main__":
    main()

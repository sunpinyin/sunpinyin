#!/usr/bin/python3

import os
import sqlite3 as sqlite
from importer import import_to_sunpinyin_user_dict

def load_fit_user_dict ():
    homedir = os.environ.get("HOME")
    db = sqlite.connect (homedir+"/Library/FunInputToy/py.sqlite")
    sqlstring = "SELECT code, string FROM words WHERE is_new==1"
    result = list (db.execute (sqlstring).fetchall ())
    db.close ()
    return result

def main ():
    fit_user_dict = load_fit_user_dict()
    import_to_sunpinyin_user_dict (fit_user_dict)

if __name__ == "__main__":
    main()

# -*- indent-tabs-mode: nil -*- vim:et:ts=4

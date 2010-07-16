#!/usr/bin/python
import os, sys
import sqlite3 as sqlite
import imdict, trie
from pinyin_data import valid_syllables, decode_syllable, initials, finals

def get_userdict_path ():
    homedir = os.environ.get("HOME")

    if sys.platform == "darwin":
        return homedir+"/Library/Application Support/SunPinyin/userdict"

    # FIXME: not sure how to get the ibus version or wrapper type (xim or ibus)
    if os.path.exists (homedir+"/.cache/ibus/sunpinyin"):
        return homedir+"/.cache/ibus/sunpinyin/userdict"
        
    if os.path.exists (homedir+"/.ibus/sunpinyin"):
        return homedir+"/.ibus/sunpinyin/userdict"
    
    if os.path.exists (homedir+"/.sunpinyin"):
        return homedir+"/.sunpinyin/userdict"

    raise "Can not detect sunpinyin's userdict!"

def import_to_sunpinyin_user_dict (records, userdict_path=''):
    userdict_path = userdict_path if userdict_path else get_userdict_path()
    db = sqlite.connect (userdict_path)

    sysdict = imdict.IMDict("dict.utf8")

    sqlstring = """
            CREATE TABLE IF NOT EXISTS dict(
            id INTEGER PRIMARY KEY, len INTEGER,
            i0 INTEGER, i1 INTEGER, i2 INTEGER, i3 INTEGER, i4 INTEGER, i5 INTEGER,
            f0 INTEGER, f1 INTEGER, f2 INTEGER, f3 INTEGER, f4 INTEGER, f5 INTEGER,
            utf8str TEXT, UNIQUE (utf8str));
            """
    db.executescript (sqlstring)

    batch_count = 0

    for (pystr, utf8str) in records:
        try:
            syllables = [valid_syllables[s] for s in pystr.split("'")]
        except:
            print "[%s] has un-recognized syllables, ignoring this record!" % pystr
            continue

        if len (syllables) < 2 or len (syllables) > 6:
            print "[%s] is too long or too short for sunpinyin userdict" % utf8str
            continue

        if sysdict and trie.search (sysdict, utf8str):
            #print "[%s] is already in sunpinyin's sysdict" % utf8str
            continue

        record = [0]*14
        record[0] = len (syllables)
        record[13] = utf8str

        c = 1
        for s in syllables:
            i, f = s>>12, (s&0x00ff0)>>4
            if i and not f:
                break; 
            record[c] = i
            record[c+1] = f
            c += 2
        else:
            sqlstring = """
                    INSERT INTO dict (len, i0, f0, i1, f1, i2, f2, i3, f3, i4, f4, i5, f5, utf8str)
                    VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);
                    """
            try:
                db.execute (sqlstring, record)
                #print "[%s] is imported into sunpinyin's userdict" % utf8str

                batch_count += 1
                if batch_count == 100:
                    db.commit ()
                    batch_count = 0

            except:
                #print "[%s] is already in sunpinyin's userdict" % utf8str
                pass

    db.commit()
    db.close()

def export_sunpinyin_user_dict (userdict_path=''):
    userdict_path = userdict_path if userdict_path else get_userdict_path()
    db = sqlite.connect (userdict_path)

    sqlstring = "SELECT * FROM dict"
    result = list (db.execute (sqlstring).fetchall ())

    for record in result:
        id   = record[0]
        l    = record[1]
        i    = record[2:8]
        f    = record[8:14]
        str  = record[-1]
        syls = [initials[i[x]] + finals[f[x]] for x in range(l)]
        print str.encode ('UTF-8'), id, "'".join(syls) 
        
if __name__ == "__main__":
    export_sunpinyin_user_dict ('/Users/yongsun/.sunpinyin/userdict')

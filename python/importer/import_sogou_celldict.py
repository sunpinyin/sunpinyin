#!/usr/bin/python

# thanks for the reverse engineering efforts of following projects/peoples:
# http://code.google.com/p/imewlconverter
# http://code.google.com/p/ibus-cloud-pinyin
# http://forum.ubuntu.org.cn/viewtopic.php?f=8&t=250136&start=0

from importer import import_to_sunpinyin_user_dict
import struct
import os, sys

def read_utf16_str (f, offset=-1, len=2):
    if offset >= 0:
        f.seek(offset)
    str = f.read(len)
    return str.decode('UTF-16LE')

def read_uint16 (f):
    return struct.unpack ('<H', f.read(2))[0]

def get_word_from_sogou_cell_dict (fname):
    f = open (fname, 'rb')
    file_size = os.path.getsize (fname)
    
    hz_offset = 0
    mask = struct.unpack ('B', f.read(128)[4])[0]
    if mask == 0x44:
        hz_offset = 0x2628
    elif mask == 0x45:
        hz_offset = 0x26c4
    else:
        sys.exit(1)
    
    title   = read_utf16_str (f, 0x130, 0x338  - 0x130)
    type    = read_utf16_str (f, 0x338, 0x540  - 0x338)
    desc    = read_utf16_str (f, 0x540, 0xd40  - 0x540)
    samples = read_utf16_str (f, 0xd40, 0x1540 - 0xd40)
    
    py_map = {}
    f.seek(0x1540+4)
    
    while 1:
        py_code = read_uint16 (f)
        py_len  = read_uint16 (f)
        py_str  = read_utf16_str (f, -1, py_len)
    
        if py_code not in py_map:
            py_map[py_code] = py_str
    
        if py_str == 'zuo':
            break
    
    f.seek(hz_offset)
    while f.tell() != file_size:
        word_count   = read_uint16 (f)
        pinyin_count = read_uint16 (f) / 2
    
        py_set = []
        for i in range(pinyin_count):
            py_id = read_uint16(f)
            py_set.append(py_map[py_id])
        py_str = "'".join (py_set)
    
        for i in range(word_count):
            word_len = read_uint16(f)
            word_str = read_utf16_str (f, -1, word_len)
            f.read(12)  # simply ignore word frequence info
            yield py_str, word_str

    f.close()

def main ():
    if len (sys.argv) != 2:
        print "Please specify the Sogou PinYin Cell dict file!"
        exit (1)

    generator = get_word_from_sogou_cell_dict (sys.argv[1])
    import_to_sunpinyin_user_dict (generator)

if __name__ == "__main__":
    main()

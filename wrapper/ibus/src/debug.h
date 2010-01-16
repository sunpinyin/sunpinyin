#ifndef SUNPINYIN_DEBUG_H
#define SUNPINYIN_DEBUG_H

#include <iostream>
#include <iomanip>
#include <fstream>
#include <cerrno>

namespace ibus
{
    extern std::fstream log;
}

using std::cout;
using std::hex;
using std::endl;

#define PRINT_VAL(val)                          \
    do {                                        \
        cout << "  "#val" = " << val << endl;   \
    } while (0)
 
#endif // SUNPINYIN_DEBUG_H

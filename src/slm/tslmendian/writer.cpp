#include <arpa/inet.h>
#include <cassert>
#include <cstring>

#include "writer.h"

int
get_host_endian()
{
    return htons(0x0001) == 0x0100 ? LITTLE_ENDIAN : BIG_ENDIAN;
}

int
parse_endian(const char* arg)
{
    if (!strcmp(arg, "le")) {
        return LITTLE_ENDIAN;
    } else if (!strcmp(arg, "be")) {
        return BIG_ENDIAN;
    } else {
        return -1;
    }
}

const char*
endian2str(int endian)
{
    static const char le[] = "little-endian";
    static const char be[] = "big-endian";

    switch (endian) {
    case LITTLE_ENDIAN:
        return le;
    case BIG_ENDIAN:
        return be;
    default:
        assert(0);
    }
    return NULL;
}

// -*- indent-tabs-mode: nil -*- vim:et:ts=4

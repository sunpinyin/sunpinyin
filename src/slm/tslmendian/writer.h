#ifndef WRITER_H
#define WRITER_H

#include <stdio.h>
#include <stdint.h>
#include <algorithm>

#ifndef LITTLE_ENDIAN
enum {
    BIG_ENDIAN = 4321,
    LITTLE_ENDIAN = 1234,
    UNKNOWN_ENDIAN = 0x0000
};
#else
#define UNKNOWN_ENDIAN (0x0000)
#endif
int get_host_endian();
int parse_endian(const char* arg);
const char* endian2str(int endian);

// change the byte order of given variable
template <typename Type>
Type change_byte_order(const Type& v)
{
    Type t = v;
    const size_t size = sizeof(v);
    uint8_t* first = (uint8_t*)(&t);
    uint8_t* last  = first+size-1;
    while (first < last) {
        std::swap(*first++, *last--);
    }
    return t;
}
    
template <typename T>
class OtherEndian
{
public:
    typedef T TargetType;
    static TargetType create(const T& from)
    {
        return from;
    }
};

#if WORDS_BIGENDIAN
#define DEFINE_OTHER_TYPE(__T__) typedef __T__##_BE TargetType
#else
#define DEFINE_OTHER_TYPE(__T__) typedef __T__##_LE TargetType
#endif

//
// we always defined a reversed layout of big-endian and little-endian 
// bit-field struct, so such kind of struct need to be reverted if host
// arch is different from build arch.
//
template <typename T>
bool revert_write(const T& t, FILE *fp)
{
    T reverted = change_byte_order(t);
    typename OtherEndian<T>::TargetType o =
        OtherEndian<T>::create(reverted);
    return fwrite(&o, sizeof(o), 1, fp) == 1;
}

//
// if the struct has non-bit-field member(s), TTransUnit, among others,
// the order of members is the same as how they are defined.
//

class Writer
{
public:
    Writer(FILE *fp, bool doRevert)
        : m_fp (fp), m_doRevert(doRevert)
    {}
    
    template <typename T>
    bool write(const T& t)
    {
        if (m_doRevert)
            return revert_write(t, m_fp);
        else
            return fwrite(&t, sizeof(t), 1, m_fp) == 1;
    }
    
    
    template <typename T>
    bool write(const T* t, size_t len)
    {
        for (unsigned i = 0; i < len; i++) {
            if (!write(t[i]))
                return false;
        }
        return true;
    }
private:
    FILE *m_fp;
    const bool m_doRevert;
};



#endif // WRITER_H

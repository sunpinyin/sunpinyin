// -*- mode: c++ -*-
/*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
 *
 * Copyright (c) 2007 Sun Microsystems, Inc. All Rights Reserved.
 *
 * The contents of this file are subject to the terms of either the GNU Lesser
 * General Public License Version 2.1 only ("LGPL") or the Common Development and
 * Distribution License ("CDDL")(collectively, the "License"). You may not use this
 * file except in compliance with the License. You can obtain a copy of the CDDL at
 * http://www.opensource.org/licenses/cddl1.php and a copy of the LGPLv2.1 at
 * http://www.opensource.org/licenses/lgpl-license.php. See the License for the
 * specific language governing permissions and limitations under the License. When
 * distributing the software, include this License Header Notice in each file and
 * include the full text of the License in the License file as well as the
 * following notice:
 *
 * NOTICE PURSUANT TO SECTION 9 OF THE COMMON DEVELOPMENT AND DISTRIBUTION LICENSE
 * (CDDL)
 * For Covered Software in this distribution, this License shall be governed by the
 * laws of the State of California (excluding conflict-of-law provisions).
 * Any litigation relating to this License shall be subject to the jurisdiction of
 * the Federal Courts of the Northern District of California and the state courts
 * of the State of California, with venue lying in Santa Clara County, California.
 *
 * Contributor(s):
 *
 * If you wish your version of this file to be governed by only the CDDL or only
 * the LGPL Version 2.1, indicate your decision by adding "[Contributor]" elects to
 * include this software in this distribution under the [CDDL or LGPL Version 2.1]
 * license." If you don't indicate a single choice of license, a recipient has the
 * option to distribute your version of this file under either the CDDL or the LGPL
 * Version 2.1, or to extend the choice of license to its licensees as provided
 * above. However, if you add LGPL Version 2.1 code and therefore, elected the LGPL
 * Version 2 license, then the option applies only if the new code is made subject
 * to such option by the copyright holder.
 */

#ifndef ___SUN_SLM_PORTABILITY_H___
#define ___SUN_SLM_PORTABILITY_H___

#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string>
#include <cstring>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#ifdef __cplusplus
#include <algorithm>
#else
#include <sys/ddi.h>
#endif //__cpluscplus
#endif //ifdef HAVE_UNISTD_H

#ifndef HOST_OS_GNUC_2
    #if defined(DEBUG) && !defined(NDEBUG)
        #define DEBUG_print(fmt, ...)   fprintf(stderr, fmt, ...)
    #else
        #define DEBUG_print(fmt, ...)   (int(0))
    #endif
#else // HOST_OS_GNUC_2
    #if defined(DEBUG) && !defined(NDEBUG)
        #define DEBUG_print(fmt, a ...)   fprintf(stderr, fmt, a ...)
    #else
        #define DEBUG_print(fmt, a ...)   (int(0))
    #endif
#endif // !HOST_OS_GNUC_2

#ifndef HAVE_LOG2
inline double log2(double x) { return log(x) / M_LN2; }
#endif

#if defined(sun) || defined(__NetBSD__) // Solaris/HP-UX/NetBSD 's iconv is const char**
typedef const char* TIConvSrcPtr;
#else
typedef char* TIConvSrcPtr;
#endif

union TDoubleAnatomy {
public:
    TDoubleAnatomy(double d) : m_value(d){
    }

    int getExp(void) const
    { return anony.m_exp - 0x3FF; }

    double getValue(void) const
    { return m_value; }

    void clearExp(void)
    { anony.m_exp = 0x3FF; }

public:
    double m_value;
    struct TAnony {
#ifndef WORDS_BIGENDIAN
        unsigned m_other2 : 32;
        unsigned m_other1 : 20;
        unsigned m_exp    : 11;
        unsigned m_neg    : 1;
#else
        unsigned m_neg    : 1;
        unsigned m_exp    : 11;
        unsigned m_other1 : 20;
        unsigned m_other2 : 32;
#endif
    } anony;
};

struct TLongExpFloat {
public:
    TLongExpFloat(const TLongExpFloat& b) : m_base(b.m_base), m_exp(b.m_exp) { }

    TLongExpFloat(int exp = 0, double base = 0.0) : m_base(base), m_exp(exp) { }

    TLongExpFloat(double d);

    TLongExpFloat
    operator*(const TLongExpFloat& b) const;

    TLongExpFloat
    operator/(const TLongExpFloat& b) const;

    bool
    operator<(const TLongExpFloat& b) const;

    bool
    operator<=(const TLongExpFloat& b) const;

    bool
    operator==(const TLongExpFloat& b) const;

    void
    toString(std::string& str) const;

    void toString(char* buf) const
    { if (buf) sprintf(buf, "%10lf*2^%d", m_base, m_exp); }

    double log2() const { return ::log2(m_base) + m_exp; }

private:
    double m_base;
    int m_exp;
};

/**
 * UCS4 wide character type, system dependent
 * Multibytes string in this program is UTF-8 only
 */
typedef unsigned int TWCHAR;

#if !defined(WORDS_BIGENDIAN)
    #define  TWCHAR_ICONV_NAME  "UCS-4LE"
#else
    #define  TWCHAR_ICONV_NAME  "UCS-4BE"
#endif

typedef TWCHAR TSIMWordId;

const TSIMWordId SIM_ID_NOT_WORD = (0x0);
const TSIMWordId SIM_ID_UNKNOWN_CN = (2);
const TSIMWordId SIM_ID_SEN_TOKEN = (10);
const TSIMWordId SIM_ID_DIGIT = (20);
const TSIMWordId SIM_ID_SIMBOL = (21);
const TSIMWordId SIM_ID_DUMMY_WORD = (69);
const TSIMWordId SIM_ID_NONWORD = (69);
const TSIMWordId SIM_ID_ALLWORD = (69);
const TSIMWordId SIM_ID_REALWORD_START = (70);
const TSIMWordId SIM_ID_CNWORD_START = (100);

const TWCHAR WCH_NULL = 0;
const TWCHAR WCH_RETURN = '\n';
const TWCHAR WCH_TAB = '\t';
const TWCHAR WCH_SPACE = ' ';
const TWCHAR WCH_LESSTHAN = '<';
const TWCHAR WCH_GREATERTHAN = '>';

const TWCHAR WCH_DOUBLESPACE = 0x3000;
const TWCHAR WCH_JUHAO = 0x3002;
const TWCHAR WCH_WENHAO = 0xFF1F;
const TWCHAR WCH_TANHAO = 0xFF01;
const TWCHAR WCH_FENHAO = 0xFF1B;
const TWCHAR WCH_MAOHAO = 0xFF1A;
const TWCHAR WCH_DOUHAO = 0xFF0C;
const TWCHAR WCH_ZUOKUOHAO = 0x201C;
const TWCHAR WCH_YOUKUOHAO = 0x201D;
const TWCHAR WCH_SHENGLUEHAO = 0x2026;


size_t MBSTOWCS(TWCHAR *pwcs, const char* s, size_t n);

size_t WCSTOMBS(char* s, const TWCHAR* pwcs, size_t n);

size_t WCSLEN(const TWCHAR* ws);

namespace std {
#ifdef HOST_OS_GNUC_2
struct string_char_traits<TWCHAR>
#else // !HOST_OS_GNUC_2
template<>
struct char_traits<TWCHAR>
#endif // HOST_OS_GNUC_2
{
    typedef TWCHAR char_type;
    typedef unsigned int int_type;
#ifndef HOST_OS_GNUC_2
    typedef streamoff off_type;
    typedef wstreampos pos_type;
    typedef mbstate_t state_type;
#endif // !HOST_OS_GNUC_2

    static void assign(char_type& __c1, const char_type& __c2)
    { __c1 = __c2; }

    static bool eq(const char_type& __c1, const char_type& __c2)
    { return __c1 == __c2; }

    static bool lt(const char_type& __c1, const char_type& __c2)
    { return __c1 < __c2; }

    static int compare(const char_type* __s1, const char_type* __s2,
                       size_t __n)           {
        for (size_t i = 0; i < __n; ++i) {
            if (*__s1 < *__s2)
                return -1;
            else if (*__s1++ == *__s2++)
                continue;
            else
                return 1;
        }
        return 0;
    }

    static size_t length(const char_type* __s)
    { return WCSLEN(__s); }

    static char_type*copy(char_type* __s1, const char_type* __s2,
                          size_t __n)                 {
        return static_cast<char_type*>(memcpy(__s1, __s2, __n *
                                              sizeof(char_type)));
    }

    static char_type*move(char_type* __s1, const char_type* __s2,
                          int_type __n)                 {
        return static_cast<char_type*>(memmove(__s1, __s2, __n *
                                               sizeof(char_type)));
    }

#ifndef HOST_OS_GNUC_2
    static const char_type*find(const char_type* __s,
                                size_t __n,
                                const char_type& __a)                       {
        while (__n--) {
            if (*__s++ == __a)
                return __s;
        }
        return NULL;
    }

    static char_type to_char_type(const int_type& __c) { return char_type(__c); }

    static int_type to_int_type(const char_type& __c) { return int_type(__c); }

    static bool eq_int_type(const int_type& __c1, const int_type& __c2)
    { return __c1 == __c2; }

    static int_type eof() { return static_cast<int_type>(WEOF); }

    static int_type not_eof(const int_type& __c)
    { return eq_int_type(__c, eof()) ? 0 : __c; }

    static char_type*assign(char_type* __s, size_t __n,
                            char_type __a)                 {
        for (char_type *p = __s; __n--; )
            *p++ = __a;
        return __s;
    }
#else // HOST_OS_GNUC_2
    static bool ne(const char_type& __c1, const char_type& __c2)
    { return __c1 != __c2; }

    static char_type eos()
    { return 0; }

    static bool is_del(char_type a)
    { return a == WCH_SPACE; }

    static char_type* set(char_type* __s, char_type __a, size_t __n){
        for (char_type *p = __s; __n--; )
            *p++ = __a;
        return __s;
    }
#endif // !HOST_OS_GNUC_2
};
}; // namespace std

#ifndef HOST_OS_GNUC_2
typedef std::basic_string<TWCHAR>   wstring;
#else // HOST_OS_GNUC_2
class wstring : public std::basic_string<TWCHAR>
{
public:
    inline wstring() : std::basic_string<TWCHAR>((TWCHAR) 0) {}
    inline wstring(const TWCHAR* c) : std::basic_string<TWCHAR>(c) {}
    inline wstring(const TWCHAR* c, size_t n) : std::basic_string<TWCHAR>(c,
                                                                          n) {}
    inline void push_back(TWCHAR c) { this->append(1, c); }
    inline void clear(void) { this->resize(0); }
    inline const TWCHAR* c_str(void) const {
        static TWCHAR null_s = 0;
        if (this->length() == 0) return &null_s;
        *(const_cast<TWCHAR*>(this->data()) + this->length()) = 0;
        return this->data();
    }
};
#endif // !HOST_OS_GNUC_2

#ifdef _RW_STD_STL
template <class Iterator>
inline long distance(Iterator pos1, Iterator pos2){
    long d = 0;
    distance(pos1, pos2, d);
    return d;
}
#endif

#endif

// -*- indent-tabs-mode: nil -*- vim:et:ts=4

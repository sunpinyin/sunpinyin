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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_ASSERT_H
#include <assert.h>
#endif

#include "portability.h"

#include <stdlib.h>

TLongExpFloat::TLongExpFloat(double d)
{
    if (d != 0.0 && d != -0.0) {
        TDoubleAnatomy da(d);
        m_exp = da.getExp();
        da.clearExp();
        m_base = da.getValue();
    } else {
        m_base = d;
        m_exp = 0;
    }
}

TLongExpFloat
TLongExpFloat::operator*(const TLongExpFloat& b) const
{
    double d = this->m_base * b.m_base;
    TLongExpFloat reda(d);
    reda.m_exp += this->m_exp + b.m_exp;
    return reda;
}

TLongExpFloat
TLongExpFloat::operator/(const TLongExpFloat& b) const
{
    double d = this->m_base / b.m_base;
    TLongExpFloat reda(d);
    reda.m_exp += (this->m_exp - b.m_exp);
    return reda;
}

bool
TLongExpFloat::operator<(const TLongExpFloat& b) const
{
    if (m_base >= 0.0 && b.m_base >= 0.0) {
        return(m_exp < b.m_exp || (m_exp == b.m_exp && m_base < b.m_base));
    } else if (m_base < 0.0 && b.m_base < 0.0) {
        return(m_exp > b.m_exp || (m_exp == b.m_exp && m_base < b.m_base));
    } else if (m_base < 0.0 && b.m_base >= 0.0)
        return true;
    else
        return false;
}

bool
TLongExpFloat::operator<=(const TLongExpFloat& b) const
{
    if (m_base >= 0.0 && b.m_base >= 0.0) {
        return(m_exp < b.m_exp || (m_exp == b.m_exp && m_base <= b.m_base));
    } else if (m_base < 0.0 && b.m_base < 0.0) {
        return(m_exp > b.m_exp || (m_exp == b.m_exp && m_base <= b.m_base));
    } else if (m_base < 0.0 && b.m_base >= 0.0)
        return true;
    else
        return false;
}

bool
TLongExpFloat::operator==(const TLongExpFloat& b) const
{
    return(m_base == b.m_base && m_exp == b.m_exp);
}

void
TLongExpFloat::toString(std::string& str) const
{
    char buf[256];
    toString(buf);
    str = buf;
}

#ifdef HAVE_ICONV_H
#include <iconv.h>

/**
 * convert UTF-8 string pointed by s into UCS-4 Wide String at pwcs.
 * No more than n wide char could be converted into target buffer.
 * return -1 means error.
 * other means the converted wide char number do not count the end 0.
 */
size_t
MBSTOWCS(TWCHAR *pwcs, const char* s, size_t n)
{
#ifndef WORDS_BIGENDIAN
    static iconv_t ic = iconv_open("UCS-4LE", "UTF-8");
#else
    static iconv_t ic = iconv_open("UCS-4BE", "UTF-8");
#endif

    assert(ic != (iconv_t)-1);

    // To eliminate the const char* and char* diffirence in differnt system
    TIConvSrcPtr src = (TIConvSrcPtr)s;
    size_t srclen = std::strlen(s) + 1;
    char* dst = (char*)pwcs;
    size_t dstlen = n * sizeof(TWCHAR);

    size_t res = iconv(ic, &src, &srclen, &dst, &dstlen);

    if (res != size_t(-1) && srclen == 0) {
        n -= dstlen / sizeof(TWCHAR);
        return (n > 0) ? (n - 1) : 0;
    } else {
        return size_t(-1);
    }
}

/**
 * convert UCS-4 string pointed by pwcs into UTF-8 String at s.
 * No more than n byte could be converted into target buffer.
 * return -1 means error.
 * Other means the converted byte number do not count the end 0.
 */
size_t
WCSTOMBS(char* s, const TWCHAR* pwcs, size_t n)
{
#ifndef WORDS_BIGENDIAN
    static iconv_t ic = iconv_open("UTF-8", "UCS-4LE");
#else
    static iconv_t ic = iconv_open("UTF-8", "UCS-4BE");
#endif

    assert(ic != (iconv_t)-1);

    TIConvSrcPtr src = (TIConvSrcPtr)pwcs;
    size_t srclen = (WCSLEN(pwcs) + 1) * sizeof(TWCHAR);
    char* dst = (char*)s;
    size_t dstlen = n;

    size_t res = iconv(ic, &src, &srclen, &dst, &dstlen);

    if (res != size_t(-1) && srclen == 0) {
        n -= dstlen;
        return (n > 0) ? (n - 1) : 0;
    } else {
        return size_t(-1);
    }
}

#else // !HAVE_ICONV_H

size_t
MBSTOWCS(TWCHAR *pwcs, const char* s, size_t n)
{
    const unsigned char *src = (const unsigned char*)s;
    TWCHAR* dst = pwcs;

    while (dst - pwcs < (ssize_t)n) {
        if (*src < 0xc0 || *src >= 0xfe) {
            if (*src < 0x80) *dst++ = *src;
            if (*src++ == 0) break;
            continue;
        }

        for (int bytes = 2; bytes <= 6; bytes++) {
            if ((*src & ~(0xff >> (bytes + 1))) !=
                (((1 << bytes) - 1) << (8 - bytes))) continue;
            if (bytes > 4) {
                src += bytes;
            } else {
                *dst =
                    TWCHAR(*src++ & (0xff >> (bytes + 1))) << (6 * (bytes - 1));
                for (; bytes-- > 1; src++) *dst |=
                        TWCHAR(*src & 0x3f) << (6 * (bytes - 1));
                dst++;
            }
            break;
        }
    }

    return(dst - pwcs);
}

size_t
WCSTOMBS(char* s, const TWCHAR* pwcs, size_t n)
{
    char* dst = s;

    while (dst - s < n) {
        if (*pwcs < 0x80 || *pwcs > 0x10ffff) {
            if (*pwcs < 0x80) *dst++ = *pwcs;
            if (*pwcs++ == 0) break;
            continue;
        }

        int bytes = *pwcs < 0x800 ? 2 : (*pwcs < 0xffff ? 3 : 4);
        dst += bytes;
        if (dst - s > n) return -1;

        TWCHAR c = *pwcs++;
        int nbyte = bytes;
        char *tmp = dst - 1;

        for (; nbyte > 0; c >>= 6, nbyte--)
            *tmp-- =
                (nbyte ==
                 1 ? (((1 << bytes) - 1) << (8 - bytes)) : 0x80) | (c & 0x3f);
    }

    return(dst - s);
}

#endif // HAVE_ICONV_H

/**
 * return the wide string len at pwcs, not count the end 0.
 */
size_t
WCSLEN(const TWCHAR* pwcs)
{
    size_t sz = 0;
    if (pwcs) {
        while (*pwcs++)
            ++sz;
    }
    return sz;
}

// -*- indent-tabs-mode: nil -*- vim:et:ts=4

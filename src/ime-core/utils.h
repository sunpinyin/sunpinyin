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

#ifndef SUNPY_UTILS_H
#define SUNPY_UTILS_H

#include <string>
#include <utility>
#include <vector>

typedef std::pair<std::string, std::string> string_pair;
typedef std::vector<string_pair> string_pairs;

class CNonCopyable
{
protected:
    CNonCopyable () {}
    ~CNonCopyable () {}

private:
    CNonCopyable (const CNonCopyable&);
    CNonCopyable& operator =(const CNonCopyable&);
};

template <typename T, bool isArray = false>
class CResourceHandler
{
public:
    CResourceHandler(T* p) : m_p(p) {}
    ~CResourceHandler() { delete m_p; }
protected:
    T *m_p;
};

template <typename T>
class CResourceHandler <T, true>
{
public:
    CResourceHandler(T* p) : m_p(p) {}
    ~CResourceHandler() { delete [] m_p; }
protected:
    T *m_p;
};


template <typename T>
class CSharedPtr
{
private:
    typedef CSharedPtr<T> this_type;

    T*          ptr;
    unsigned*   ref;

public:
    explicit CSharedPtr (T* p = 0) : ptr(p), ref(new unsigned (1)) {}
    CSharedPtr (const this_type& p) : ptr(p.ptr), ref(p.ref) { ++(*ref); }
    ~CSharedPtr () { dispose(); }

    CSharedPtr<T>& operator =(const this_type& p){
        if (this != &p) {
            dispose();
            ptr = p.ptr, ref = p.ref;
            ++(*ref);
        }
        return *this;
    }

    bool equal(const this_type p) const
    { return *ptr == *p.ptr; }

    bool operator ==(const this_type& p) const
    { return ptr == p.ptr && ref == p.ref; }

    T* get()         const { return ptr; }

    operator bool() const { return ptr != 0; }
    operator T&() const { return *ptr; }
    T& operator *() const { return *ptr; }
    T* operator ->() const { return ptr; }

private:
    void dispose(){
        if (--(*ref) == 0) {
            delete ref;
            delete ptr;
        }
    }
};

template <class T>
class SingletonHolder
{
public:
    typedef T Type;
    static T& instance(){
        static T instance_;
        return instance_;
    }
};

#endif /* SUNPY_UTILS_H */

// -*- indent-tabs-mode: nil -*- vim:et:ts=4

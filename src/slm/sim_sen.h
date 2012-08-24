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

#ifndef _SIM_SENTENCIZER_H
#define _SIM_SENTENCIZER_H

#include "../portability.h"

#include <stdlib.h>
#include <deque>

class CSIMCharReader;

class SIMCharReaderIterator {
public:
    SIMCharReaderIterator(CSIMCharReader* the_reader = NULL, int the_idx = 0)
        : reader(the_reader), idx(the_idx) { }
    SIMCharReaderIterator& operator++();
    SIMCharReaderIterator operator+(int i);
    TWCHAR & operator*() const;

private:
    CSIMCharReader* reader;
    int idx;
};

class CSIMCharReader {
    friend class SIMCharReaderIterator;

public:
    typedef SIMCharReaderIterator iterator;

public:
    CSIMCharReader(FILE* afp) : fp(afp), buf() { }
    iterator begin() { return iterator(this, 0); }

private:
    FILE *fp;
    std::deque<TWCHAR> buf;
};

template <class iterator>
bool ReadSentence(wstring& wstr, iterator& first, bool bIgnoreCRLF = false){
    wstr.clear();
    TWCHAR ch, chnext, ch2;
    int brk = 0;
    for (; (ch = *first) != WCH_NULL; ++first) {
        chnext = *(first + 1);
        ch2 = *(first + 2);
        if ((ch == WCH_RETURN) && bIgnoreCRLF && (chnext != WCH_RETURN))
            continue;
        if (ch == WCH_JUHAO || ch == WCH_WENHAO || ch == WCH_TANHAO ||
            ch == WCH_TANHAO || ch == WCH_SHENGLUEHAO)
            brk = 1;
        else if (ch == WCH_DOUHAO && chnext == WCH_DOUHAO)
            brk = 1;
        else if (ch == WCH_RETURN || ch == WCH_TAB)
            brk = 2;
        else if (ch == WCH_SPACE) {
            if (chnext == WCH_RETURN) {
                if (ch2 == WCH_SPACE || ch2 == WCH_TAB || ch2 == WCH_RETURN)
                    brk = 2;
            } else if (chnext == WCH_SPACE || chnext == WCH_TAB)
                brk = 2;
        }
        if (brk != 0)
            break;
        wstr.push_back(ch);
    }
    if (brk == 2 && wstr.size() == 0) {
        do {
            wstr.push_back(*first);
            ch = *(++first);
        } while (ch == WCH_SPACE || ch == WCH_TAB || ch == WCH_RETURN);
    } else if (brk == 1) {
        do {
            wstr.push_back(*first);
            ch = *(++first);
        } while (ch == WCH_JUHAO || ch == WCH_WENHAO || ch == WCH_TANHAO ||
                 ch == WCH_FENHAO || ch == WCH_MAOHAO || ch == WCH_SHENGLUEHAO);
    }
    return(wstr.size() > 0);
}

#endif

// -*- indent-tabs-mode: nil -*- vim:et:ts=4

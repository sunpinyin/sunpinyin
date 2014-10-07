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

#ifndef SUNPY_SEGMENTOR_H
#define SUNPY_SEGMENTOR_H

#include "portability.h"
#include "syllable.h"
#include <vector>

struct IPySegmentor {
    enum ESegmentType
    { SYLLABLE, SYLLABLE_SEP, INVALID, STRING };

    struct TSegment {
        TSegment (ESegmentType type = SYLLABLE) : m_type(type),
                                                  m_inner_fuzzy(0) {}
        TSegment (unsigned syllable,
                  unsigned start,
                  unsigned length,
                  ESegmentType type = SYLLABLE)
            : m_start(start), m_len(length), m_type(type), m_inner_fuzzy(0)
        { m_syllables.push_back(syllable); }

        bool operator <(const TSegment& other) const {
            if (m_start < other.m_start)
                return true;

            if (m_start == other.m_start)
                return m_len < other.m_len;

            return false;
        }

        // if segment is a STRING type, m_syllables may contain the string buffer without the '\0'
        std::vector<unsigned>           m_syllables;
        std::vector<unsigned>           m_fuzzy_syllables;
        unsigned m_start        : 16;
        unsigned m_len          : 8;
        ESegmentType m_type         : 7;
        bool m_inner_fuzzy  : 1;
    };

    // it requires the segments are sorted by its m_start field
    typedef std::vector<TSegment>  TSegmentVec;

    virtual ~IPySegmentor () {}
    virtual TSegmentVec& getSegments(bool req_aux_segs = true) = 0;
    virtual const wstring& getInputBuffer() = 0;
    virtual const char* getSylSeps() = 0;

    virtual unsigned push(unsigned ch) = 0;
    virtual unsigned pop() = 0;
    virtual unsigned insertAt(unsigned idx, unsigned ch) = 0;
    virtual unsigned deleteAt(unsigned idx, bool backward = true) = 0;
    virtual unsigned clear(unsigned from = 0) = 0;
    virtual void     notify_best_segpath(std::vector<unsigned>& seg_path) {}

    virtual unsigned updatedFrom() = 0;
};

#endif

// -*- indent-tabs-mode: nil -*- vim:et:ts=4

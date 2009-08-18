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

#ifndef _SIM_IDNGRAM_H
#define _SIM_IDNGRAM_H

#include "../../portability.h"
#include "../sim_fmerge.h"

template<int N>
class CSIM_Idngram {
public:
        TSIMWordId ids[N];
public:
        CSIM_Idngram()
                { for (int i=0; i<N; ++i) ids[i] = 0; }
        CSIM_Idngram & operator= (const CSIM_Idngram& r)
                {
                        for (int i=0; i<N; ++i)
                                ids[i] = r.ids[i];
                        return *this;
                }
        bool operator< (const CSIM_Idngram& r) const
                {
                        for (int i=0; i<N; ++i) {
                                if (ids[i] < r.ids[i])
                                         return true;
                                else if (ids[i] > r.ids[i])
                                        return false;
                        }
                        return false;
                }

        bool operator> (const CSIM_Idngram& r) const
        {
                        for (int i=0; i<N; ++i) {
                                if (ids[i] > r.ids[i])
                                        return true;
                                else if (ids[i] < r.ids[i])
                                        return false;
                        }
                        return false;
        }

        bool operator==(const CSIM_Idngram& r) const
        {
                        for (int i=0; i<N; ++i)
                                if (ids[i] != r.ids[i]) return false;
                        return true;
        }
};


template<int N>
class CSIM_IdngramFreq : public CSIM_Idngram<N> {
public:
        CSIM_IdngramFreq()
                : CSIM_Idngram<N>(), freq(0) { }
        CSIM_IdngramFreq& operator= (const CSIM_IdngramFreq& r)
                {
                        for (int i=0; i<N; ++i)
                                this->ids[i] = r.ids[i];
                        freq = r.freq;
                        return *this;
                }
        bool read(FILE *fp, size_t& start_offset, size_t last_offset)
                {
                        if (start_offset + size() <= last_offset) {
                                fseek(fp, start_offset, SEEK_SET);
                                if (fread(this->ids, sizeof(TSIMWordId), N, fp) == N && fread(&freq, sizeof(unsigned int), 1, fp)  == 1) {
                                        start_offset += size();
                                        return true;
                                }
                        }
                        return false;
                }

protected:
        size_t size() { return N*sizeof(TSIMWordId) + sizeof(unsigned int); }

public:
        unsigned int freq;
};

#endif

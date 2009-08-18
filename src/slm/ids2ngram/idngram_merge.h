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

#ifndef _SIM_IDNGRAM_MERGE_H
#define _SIM_IDNGRAM_MERGE_H

#include <stdio.h>
#include <map>
#include <vector>
#include <algorithm>

#include "../sim_fmerge.h"
#include "idngram.h"

template<int N>
void DoIdngramMerge(FILE*out, CMultiWayFileMerger<CSIM_IdngramFreq<N> > &merger)
{
	merger.start();
	CSIM_IdngramFreq<N> prevItem;
	while (true) {
		file_para<CSIM_IdngramFreq<N> >	* ppara = merger.getBest();
		TUnitAndParaInfo<CSIM_IdngramFreq<N> > & upi = *(*ppara);
		if (upi.runOut) {
			if (prevItem.freq != 0) {
				fwrite(prevItem.ids, sizeof(TSIMWordId), N, out);
				fwrite(&(prevItem.freq), sizeof(unsigned int), 1, out);
			}
			break;
		}
		CSIM_IdngramFreq<N>& ng = upi.unit;
		if (!(prevItem == ng)) {
			if (prevItem.freq != 0) {
				fwrite(prevItem.ids, sizeof(TSIMWordId), N, out);
				fwrite(&(prevItem.freq), sizeof(unsigned int), 1, out);
			}
			prevItem = ng;
		} else {
			prevItem.freq += ng.freq;
		}
		merger.next();
	}
}

template<int N>
void ProcessingIdngramMerge(FILE *swap, FILE* out, std::vector<long>& para_offsets)
{
	CMultiWayFileMerger<CSIM_IdngramFreq<N> > merger;
	long s = 0;
	for (int i=0; i < para_offsets.size(); ++i) {
		merger.addPara(swap, s, para_offsets[i]);
		s = para_offsets[i];
	}
	DoIdngramMerge<N>(out, merger);
}

template<int N>
void ProcessingIdngramMerge(FILE* out, std::vector<FILE* >& file_list)
{
	CMultiWayFileMerger<CSIM_IdngramFreq<N> > merger;
	for (int i=0; i < file_list.size(); ++i) {
		fseek(file_list[i], 0, SEEK_END);
		merger.addPara(file_list[i], 0, ftell(file_list[i]));
	}
	DoIdngramMerge<N>(out, merger);
}

#endif


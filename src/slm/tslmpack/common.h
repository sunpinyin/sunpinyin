/*
 * Copyright (c) 2009 Kov Chai <tchaikov@gmail.com>
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
#ifndef _SLM_PACK_COMMON_H
#define _SLM_PACK_COMMON_H

#include <vector>
#include <map>
#include <string>
#include <cmath>
#include <cassert>

#include "../slm.h"

typedef std::vector<CThreadSlm::TNode> TNodeLevel;
typedef std::vector<CThreadSlm::TLeaf> TLeafLevel;
typedef std::vector<CThreadSlm::TNode*> TNodeLevels;
typedef std::map<float, float> EffRealMap;  // map from efficient values to the real ones
typedef std::map<float, int> FreqMap;       // how often the efficient value appears
typedef std::vector<float> CompressedTable; // array of real values, the index is stored in RealIndexMap
typedef std::map<float, int> RealIndexMap;  // map real values to their indices
typedef std::map<std::string, unsigned int> TLexicon; // map word to wid

#define EffectivePr(a)  (float((usingLogPr)?((a)/log(2.0)):(-log2((a)))))
#define OriginalPr(b)   (float((usingLogPr)?((b)*log(2.0)):(exp2(-(b)))))
#define EffectiveBow(a) (float((usingLogPr)?(exp(-(a))):((a))))
#define OriginalBow(b)  (float((usingLogPr)?(-log((b))):((b))))

#endif//_SLM_PACK_COMMON_H

/*
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS HEADER.
 * 
 * Copyright (c) 2008 Sun Microsystems, Inc. All Rights Reserved.
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
    #include <config.h>
#endif

#import "imi_imkitwin.h"
#import "SunPinyinInputController.h"

#ifdef WORDS_BIGENDIAN
#define UTF32Encoding NSUTF32BigEndianStringEncoding
#else
#define UTF32Encoding NSUTF32LittleEndianStringEncoding
#endif

CIMKitWindowHandler::CIMKitWindowHandler (id ic) :_ic(ic) 
{
}

CIMKitWindowHandler::~CIMKitWindowHandler()
{
}

void CIMKitWindowHandler::commit(const TWCHAR* wstr)
{
    size_t len = WCSLEN(wstr) * sizeof(TWCHAR);
    NSString *string = [[NSString alloc] initWithBytes:wstr length:len encoding:UTF32Encoding];
    [_ic commitString:string];
    [string release];
}

void CIMKitWindowHandler::updatePreedit(const IPreeditString* ppd)
{
    const TWCHAR *wstr = ppd->string();
    size_t len = (ppd->size()) * sizeof(TWCHAR);
    NSString *string = [[NSString alloc] initWithBytes:wstr length:len encoding:UTF32Encoding];
    [_ic setCaret:ppd->caret() andCandiStart:ppd->candi_start()];
    [_ic showPreeditString:string];
    [string release];
}

void CIMKitWindowHandler::updateCandidates(const ICandidateList* pcl)
{
    NSMutableArray* candidates = [NSMutableArray array];

    for (int i=0; i<pcl->size(); ++i) {
        const TWCHAR *wstr = pcl->candiString(i);
        size_t len = pcl->candiSize(i) * sizeof(TWCHAR);
        NSString *string = [[[NSString alloc] initWithBytes:wstr length:len encoding:UTF32Encoding] autorelease];
        [candidates addObject:string];
    }

    [_ic showCandidates:candidates];
}

void CIMKitWindowHandler::updateStatus(int key, int value)
{
    [_ic updateStatus:key withValue:value];
}


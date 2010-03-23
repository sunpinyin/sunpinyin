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

#import <Cocoa/Cocoa.h>
#import <Growl/Growl.h>

#import "CandidateWindow.h"
#import "imi_data.h"
#import "imi_options.h"
#import "ic_history.h"
#import "imi_session_wrapper.h"

typedef ECommitPolicies CommitPolicies;

typedef enum {
    SWITCH_BY_NONE      = 0,
    SWITCH_BY_CAPS      = 1,
    SWITCH_BY_SHIFT     = 2,
} SwitchingPolicies;

// if MAC_OS_X_VERSION_10_6 > MAC_OS_X_VERSION_MAX_ALLOWED, 
// @interface SunPinyinApplicationDelegate : NSObject <NSWindowDelegate, GrowlApplicationBridgeDelegate>
// Note: the SunPinyinApplicationDelegate is instantiated automatically as an outlet of NSApp's instance
@interface SunPinyinApplicationDelegate : NSObject <GrowlApplicationBridgeDelegate>
{
    IBOutlet NSMenu*            _menu;
    IBOutlet CandidateWindow*   _candiWin;
    IBOutlet NSPanel*           _prefPanel;
    IBOutlet NSTextField*       _ftTxtField;

    bool                        _inputChinesePuncts;
    bool                        _inputFullSymbols;
    SwitchingPolicies           _switchingPolicy;
    CommitPolicies              _commitPolicy;
    bool                        _usingUSKbLayout;
    CIMIData*                   _data;
    CBigramHistory*             _history;
}

-(NSMenu*)menu;
-(CandidateWindow*)candiWin;

-(IBAction)showPrefPanel:(id)sender;
-(IBAction)showFontPanel:(id)sender;

-(IBAction)toggleChinesePuncts:(id)sender;
-(bool)inputChinesePuncts;
-(IBAction)toggleFullSymbols:(id)sender;
-(bool)inputFullSymbols;

-(SwitchingPolicies)switchingPolicy;
-(CommitPolicies)commitPolicy;
-(bool)usingUSKbLayout;

-(NSDictionary *)registrationDictionaryForGrowl;
-(void)messageNotify:(NSString*)msg;
@end

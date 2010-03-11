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
#import <InputMethodKit/InputMethodKit.h>

// Each input method needs a unique connection name.
// Note that periods and spaces are not allowed in the connection name.
const NSString *kConnectionName = @"SunPinyin_1_Connection";

int main(int argc, char *argv[])
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    IMKServer *server;

    // find the bundle identifier and then initialize the input method server
    server = [[IMKServer alloc] initWithName: (NSString *) kConnectionName
                            bundleIdentifier: [[NSBundle mainBundle] bundleIdentifier]];

    // load the bundle explicitly because in this case the input method is a
    // background only application
    [NSBundle loadNibNamed: @"MainMenu" owner: [NSApplication sharedApplication]];
    [[NSColorPanel sharedColorPanel] setHidesOnDeactivate: NO];
    [[NSFontPanel sharedFontPanel] setHidesOnDeactivate: NO];

    // finally run everything
    [[NSApplication sharedApplication] run];

    [server release];    
    [pool release];
    return 0;
}


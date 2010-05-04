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

#import "SunPinyinInputController.h"
#import "SunPinyinApplicationDelegate.h"
#import "macos_keycode.h"
#import "imi_session_wrapper.h"

// forward declaration of 'Private' category
@interface SunPinyinController(Private) 
-(void)createSession;
-(void)destroySession;
@end 

// implementation of the public interface
@implementation SunPinyinController

/*
Implement one of the three ways to receive input from the client. 
Here are the three approaches:
                 
  1. Support keybinding.
    In this approach the system takes each keydown and trys to map the keydown 
    to an action method that the input method has implemented.  If an action 
    is found the system calls didCommandBySelector:client:.  If no action 
    method is found inputText:client: is called.  An input method choosing 
    this approach should implement

    -(BOOL)inputText:(NSString*)string client:(id)sender;
    -(BOOL)didCommandBySelector:(SEL)aSelector client:(id)sender;
                        
  2. Receive all key events without the keybinding, but do "unpack" the relevant text data.
    Key events are broken down into the Unicodes, the key code that generated 
    them, and modifier flags.  This data is then sent to the input method's 
    inputText:key:modifiers:client: method.  For this approach implement:

     -(BOOL)inputText:(NSString*)string key:(NSInteger)keyCode modifiers:(NSUInteger)flags client:(id)sender;
                        
  3. Receive events directly from the Text Services Manager as NSEvent objects.
    For this approach implement:

    -(BOOL)handleEvent:(NSEvent*)event client:(id)sender;
*/

/*!
    @method     
    @abstract   Receive incoming event
    @discussion This method receives key events from the client application. 
*/

-(BOOL)handleEvent:(NSEvent*)event client:(id)sender
{
    // Return YES to indicate the the key input was received and dealt with.  
    // Key processing will not continue in that case.  In other words the 
    // system will not deliver a key down event to the application.
    // Returning NO means the original key down will be passed on to the client.
    if (!_session->isValid()) return NO;

    _currentClient = sender;
    bool handled = NO;
    NSUInteger modifiers = [event modifierFlags];
    SwitchingPolicies switchPolicy = [[NSApp delegate] switchingPolicy];
    CommitPolicies commitPolicy = [[NSApp delegate] commitPolicy];

    if (SWITCH_BY_CAPS == switchPolicy) {
        _englishMode = (modifiers & NSAlphaShiftKeyMask);
        _session->switchInputMode(_englishMode, commitPolicy);
    }

    switch ([event type]) {
        case NSFlagsChanged:
            // FIXME: a dirty workaround for chrome sending duplicated NSFlagsChanged event
            if (_lastEventTypes[1] == NSFlagsChanged && _lastModifiers[1] == modifiers)
                return YES;

            if (SWITCH_BY_SHIFT == switchPolicy && modifiers == 0 && 
                _lastEventTypes[1] == NSFlagsChanged && _lastModifiers[1] == NSShiftKeyMask &&
                !(_lastModifiers[0] & NSShiftKeyMask))
            {
                _englishMode = !_englishMode;
                _session->switchInputMode(_englishMode, commitPolicy);

                if (_englishMode)
                    [[NSApp delegate] messageNotify:NSLocalizedString(@"Switched to English mode", nil)];
            }
            break;
        case NSKeyDown:
            NSInteger keyCode = [event keyCode];
            NSString* string = [event characters];
            unsigned char keyChar = [string UTF8String][0];

            if (_englishMode) {
                if (SWITCH_BY_CAPS == switchPolicy && isprint(keyChar)) {
                    string = (modifiers & NSShiftKeyMask)? string: [string lowercaseString];
                    [self commitString:string];
                    handled = YES;
                }
                break;
            }
            
            // translate osx keyevents to ime keyevents
            CKeyEvent key_event = osx_keyevent_to_ime_keyevent (keyCode, keyChar, modifiers);
            handled = _session->onKeyEvent (key_event);
            break;
        defaults:
            break;
    }

    _lastModifiers [0] = _lastModifiers[1];
    _lastEventTypes[0] = _lastEventTypes[1];
    _lastModifiers [1] = modifiers;
    _lastEventTypes[1] = [event type];
    return handled;
}

-(NSUInteger)recognizedEvents:(id)sender
{
    return NSKeyDownMask | NSFlagsChangedMask;
}

-(void)activateServer:(id)sender
{
    if ([[NSApp delegate] usingUSKbLayout])
        [sender overrideKeyboardWithKeyboardNamed:@"com.apple.keylayout.US"];
}

-(id)initWithServer:(IMKServer*)server delegate:(id)delegate client:(id)inputClient
{
    if (self = [super initWithServer:server delegate:delegate client:inputClient])
        [self createSession];

    return self;
}

-(void)deactivateServer:(id)sender
{
    [[[NSApp delegate] candiWin] hideCandidates];

    NSString *string = [_preeditString stringByReplacingOccurrencesOfString:@" " withString:@""];
    if (string && [string length])
        [self commitString:string];
    _session->clear();
}

/*!
    @method     
    @abstract   Called when a user action was taken that ends an input session.
                Typically triggered by the user selecting a new input method 
                or keyboard layout.
    @discussion When this method is called your controller should send the 
                current input buffer to the client via a call to 
                insertText:replacementRange:.  Additionally, this is the time 
                to clean up if that is necessary.
*/

-(void)commitComposition:(id)sender 
{
    // FIXME: chrome's address bar issues this callback when showing suggestions. 
    if ([[sender bundleIdentifier] isEqualToString:@"com.google.Chrome"])
        return;

    NSString *string = [_preeditString stringByReplacingOccurrencesOfString:@" " withString:@""];
    if (string && [string length])
        [self commitString:string];
    _session->clear();
}

-(NSMenu*)menu
{
    return [[NSApp delegate] menu];
}

-(void)showPrefPanel:(id)sender
{
    [[NSApp delegate] showPrefPanel:sender];
}

-(void)toggleChinesePuncts:(id)sender
{
    [[NSApp delegate] toggleChinesePuncts:sender];
    _session->setStatusAttrValue(CIMIWinHandler::STATUS_ID_FULLPUNC,
                                 [[NSApp delegate] inputChinesePuncts]);      
}

-(void)toggleFullSymbols:(id)sender
{
    [[NSApp delegate] toggleFullSymbols:sender];
    _session->setStatusAttrValue(CIMIWinHandler::STATUS_ID_FULLSYMBOL,
                                 [[NSApp delegate] inputFullSymbols]);    
}

-(void)dealloc 
{
    [self destroySession];
    [super dealloc];
}

-(void)commitString:(NSString*)string
{
    // fixed that IME does not work with M$ powerpoint 2008
    _caret = [string length];
    [self showPreeditString:[string retain]];

    [_currentClient insertText:string 
                    replacementRange:NSMakeRange(NSNotFound, NSNotFound)];

    [_preeditString release];
    _preeditString = nil;

    [[[NSApp delegate] candiWin] hideCandidates];
}

// firefox would call 'commitComposition:' when preedit is emptied
-(void)showPreeditString:(NSString*)string
{
    // cache the preedit string
    [_preeditString release];
    _preeditString = [string retain];

    NSDictionary*       attrs;
    NSAttributedString* attrString;

    attrs = [self markForStyle:kTSMHiliteSelectedRawText atRange:NSMakeRange(0, [string length])];
    attrString = [[NSAttributedString alloc] initWithString:string attributes:attrs];

    // Range (0, 0) will clear the marked text
    [_currentClient setMarkedText:attrString
                    selectionRange:NSMakeRange(_caret, 0) 
                    replacementRange:NSMakeRange(NSNotFound, NSNotFound)];

    [attrString release];
}

-(void)setCaret:(int)caret andCandiStart:(int)start
{
    _caret = caret;
    _candiStart = start;
}

-(void)showCandidates:(NSArray*)candidates
{
    NSRect cursorRect;
    int curIdx = _candiStart;
    [_currentClient attributesForCharacterIndex:curIdx lineHeightRectangle:&cursorRect];
    [[[NSApp delegate] candiWin] showCandidates:candidates around:cursorRect];
}

-(void)updateStatus:(int)key withValue:(int)value
{
    switch (key) {
    case CIMIWinHandler::STATUS_ID_FULLPUNC:
        if (value != [[NSApp delegate] inputChinesePuncts])
            [self toggleChinesePuncts:nil];
        break;
    case CIMIWinHandler::STATUS_ID_FULLSYMBOL:
        if (value != [[NSApp delegate] inputFullSymbols])
            [self toggleFullSymbols:nil];
        break;
    default:
        break;
    }
}

@end // SunPinyinController 


// implementation of private interface
@implementation SunPinyinController(Private)

-(void)createSession
{
    _session = new CSunpinyinSessionWrapper (self);
}

-(void)destroySession
{
    delete _session;
    _session = nil;
}
@end // SunPinyinController(Private)

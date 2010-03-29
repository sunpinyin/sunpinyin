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

#import "Carbon/Carbon.h"
#import "CandidateWindow.h"

static void tuningFrameForScreen (NSRect *, NSSize, NSRect);

@interface CandidateView : NSView {
    NSAttributedString  *_string;
    NSColor             *_bgColor;
    float                _radius;
}

-(void)setAttributedString:(NSAttributedString *)str;
-(void)setBgColor:(NSColor*)color;

@end

@implementation CandidateView

-(void)setAttributedString:(NSAttributedString *)str
{
    [str retain];
    [_string release];
    _string = str;
    [self setNeedsDisplay:YES];
}

-(NSColor*)bgColor
{
    return _bgColor;
}
    
-(void)setBgColor:(NSColor*)color
{
    [color retain];
    [_bgColor release];
    _bgColor = color;
}

-(void)setRadius:(float)radius
{
    _radius = radius;
}

- (void)drawRect:(NSRect)rect 
{
    if (!_string)
        return;

    [[NSColor clearColor] set];
    NSRectFill([self bounds]);

    NSBezierPath *path = [NSBezierPath bezierPathWithRoundedRect:rect xRadius:_radius yRadius:_radius];
    [_bgColor set];
    [path fill];

    NSPoint stringOrigin;
    NSSize stringSize = [_string size];
    stringOrigin.x = rect.origin.x + (rect.size.width  - stringSize.width)/2;
    stringOrigin.y = rect.origin.y + (rect.size.height - stringSize.height + 1)/2;

    [_string drawAtPoint:stringOrigin];
}

- (void) dealloc 
{
    [_string release];
    [_bgColor release];
    [super dealloc];
}

@end

@implementation CandidateWindow

-(id)init
{
    _font = [[NSFont fontWithName:@"Hei" size:16] retain];
    _fgColor = [[NSColor whiteColor] retain];
    _hlColor = [[NSColor blueColor] retain];

    _attr = [[NSMutableDictionary alloc] init];
    [_attr setObject:_fgColor forKey:NSForegroundColorAttributeName];
    [_attr setObject:_font forKey:NSFontAttributeName];

    _window = [[NSWindow alloc] initWithContentRect:NSMakeRect(0,0,0,0)
                                styleMask:NSBorderlessWindowMask
                                backing:NSBackingStoreBuffered
                                defer:NO];

    [_window setAlphaValue:1.0];
    [_window setLevel:NSScreenSaverWindowLevel+1];
    [_window setHasShadow:YES];    
    [_window setOpaque:NO];

    _view = [[CandidateView alloc] initWithFrame:[[_window contentView] frame]];
    [_window setContentView:_view];
    
    return self;
}

-(NSFont *)font
{
    return _font;
}

-(void)setFont:(NSFont*)font
{
    [font retain];
    [_font release];
    _font = font;
    [_attr setObject:_font forKey:NSFontAttributeName];
}

-(NSColor*)bgColor
{
    return [(CandidateView*)_view bgColor];
}

-(void)setBgColor:(NSColor*)color
{
    [(CandidateView*)_view setBgColor:color];
}     
     
-(NSColor *)fgColor
{
    return _fgColor;
}

-(void)setFgColor:(NSColor*)color
{
    [color retain];
    [_fgColor release];
    _fgColor = color;
}

-(NSColor *)hlColor
{
    return _hlColor;
}

-(void)setHlColor:(NSColor*)color
{
    [color retain];
    [_hlColor release];
    _hlColor = color;
}

-(void)setRadius:(float)radius
{
    [(CandidateView*)_view setRadius:radius];
}

-(void)showCandidates:(NSArray *)candiArray around:(NSRect)cursorRect
{
    if ([candiArray count] == 0) {
        [self hideCandidates];
        return;
    }

    [_attr setObject:_fgColor forKey:NSForegroundColorAttributeName];
    [_attr setObject:_font forKey:NSFontAttributeName];

    int i;
    NSMutableAttributedString *string = [[NSMutableAttributedString alloc] init];
    for (i=0; i<[candiArray count]; i++) {
        NSString *str = [NSString stringWithFormat:@"%d.%@ ", i+1, [candiArray objectAtIndex:i]];
        NSAttributedString *astr = [[[NSAttributedString alloc] initWithString:str attributes:_attr] autorelease];
        [string appendAttributedString:astr];
        
        if (i==0)
            [string addAttribute:NSForegroundColorAttributeName 
                    value:_hlColor
                    range:NSMakeRange(0, [str length])];
    }
    
    NSRect winRect = [_window frame];
    NSSize strSize = [string size];

    tuningFrameForScreen (&winRect, strSize, cursorRect);
    
    [(CandidateView*)_view setAttributedString:string];
    [string release];

    [_window setFrame:winRect display:YES animate:NO];
    [_window orderFront:nil];
}

-(void)hideCandidates
{
    [_window orderOut:self];
}

- (void) dealloc 
{
    [_font release];
    [_fgColor release];
    [_hlColor release];
    [_attr release];
    [_view release];
    [_window release];
    [super dealloc];
}

@end

static void tuningFrameForScreen (NSRect *winRect, NSSize strSize, NSRect cursorRect)
{
    /* caculate the initial window's size */
    winRect->size.height = strSize.height+5;
    winRect->size.width = strSize.width+5;
    winRect->origin.x = cursorRect.origin.x + 2;
    winRect->origin.y = cursorRect.origin.y - winRect->size.height -2;

    /* find a proper screen by input cursor */
    NSRect screenRect = [[NSScreen mainScreen] frame];
    NSArray *screens =[NSScreen screens];
    int i, num_of_scrs = [screens count];

    /* no choices, just use mainScreen */
    if (num_of_scrs == 1)
        goto FOUND_SCREEN;

    for (i=0; i<num_of_scrs; i++) {
        NSScreen *screen = [screens objectAtIndex:i];
        NSRect rect = [screen frame];
        if (NSPointInRect (cursorRect.origin, rect)) {
            screenRect = rect;
            goto FOUND_SCREEN;
        }
    }

    /* find a proper screen by mouse position */
    HIPoint p;
    HIGetMousePosition (kHICoordSpaceScreenPixel, NULL, &p);
    NSPoint mousePosition;
    mousePosition.x = p.x;
    mousePosition.y = screenRect.size.height - p.y;

    for (i=0; i<num_of_scrs; i++) {
        NSScreen *screen = [screens objectAtIndex:i];
        NSRect rect = [screen frame];
        if (NSPointInRect (mousePosition, rect)) {
            screenRect = rect;
            goto FOUND_SCREEN;
        }
    }

FOUND_SCREEN:;
    CGFloat min_x = NSMinX (screenRect), max_x = NSMaxX (screenRect);
    CGFloat min_y = NSMinY (screenRect);

    if (winRect->origin.x > max_x - winRect->size.width)
        winRect->origin.x = max_x - winRect->size.width;

    if (winRect->origin.x < min_x)
        winRect->origin.x = min_x;

    if (winRect->origin.y < min_y)
        winRect->origin.y = cursorRect.origin.y > min_y? 
                            cursorRect.origin.y + cursorRect.size.height + 2:
                            min_y;
}

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

#include <sys/stat.h>
#import "imi_view.h"
#import "SunPinyinApplicationDelegate.h"

static NSString* get_history_path ();
static bool save_history (CICHistory*);
static bool load_history (CICHistory*);
static bool load_preferences (CSunpinyinOptions*);

@implementation SunPinyinApplicationDelegate

//this method is added so that our controllers can access the shared NSMenu.
-(NSMenu*)menu
{
    return _menu;
}

-(CandidateWindow*)candiWin
{
    return _candiWin;
}

//this method is to return a CIMIData instance which loads the language model and lexicon file
-(CIMIData*)sysData
{
    if (_data == NULL) {
        _data = new CIMIData();
        const char * res_path = [[[NSBundle mainBundle] resourcePath] UTF8String];
        char slm_path[512], pydict_path[512];
        snprintf (slm_path, sizeof(slm_path), "%s/%s", res_path, "lm_sc.t3g");
        snprintf (pydict_path, sizeof(pydict_path), "%s/%s", res_path, "pydict_sc.bin");
        if (_data->loadResource(slm_path, pydict_path))
            return _data;

        delete _data;
        _data = NULL;
    }

    return _data;
}

-(CBigramHistory*)history
{
    if (_history == nil) {
        _history = new CBigramHistory();
        load_history(_history);
    }
    return _history;
}

-(void)saveHistory
{
    save_history(_history);
}

-(CSunpinyinOptions*)preferences
{    
    if (_pref == nil) {
        _pref = new CSunpinyinOptions();
        load_preferences (_pref);
    }

    return _pref;
}

-(void)preferencesChanged:(NSNotification *)notification
{
    if (_pref)
        load_preferences (_pref);

    NSUserDefaults* pref = [NSUserDefaults standardUserDefaults];
    
    //setting full/half puncts and symbols
    _inputChinesePuncts = [pref boolForKey:@"inputChinesePuncts"];
    _inputFullSymbols   = [pref boolForKey:@"inputFullSymbols"];
    _switchingPolicy    = (SwitchingPolicies) [pref integerForKey:@"switchingPolicy"];
    _usingUSKbLayout    = [pref boolForKey:@"usingUSKbLayout"];
        
    //setting background color
    NSData *data = [pref dataForKey:@"bgColor"];
    NSColor *color = data? (NSColor*) [NSUnarchiver unarchiveObjectWithData:data]:
                           [NSColor orangeColor];

    float alpha = [pref floatForKey:@"alpha"]/100.0;
    NSColor *bgColor = [color colorWithAlphaComponent:alpha];
    [_ftTxtField setBackgroundColor:color];
    [_candiWin setBgColor:bgColor];

    data = [pref dataForKey:@"fgColor"];
    color = data? (NSColor*) [NSUnarchiver unarchiveObjectWithData:data]:
                  [NSColor whiteColor];
    [_ftTxtField setTextColor:color];
    [_candiWin setFgColor:color];

    data = [pref dataForKey:@"hlColor"];
    color = data? (NSColor*) [NSUnarchiver unarchiveObjectWithData:data]:
                  [NSColor blueColor];
    [_candiWin setHlColor:color];

    //setting font
    NSString *ftname = [pref stringForKey:@"fontName"];
    float ftsize = [pref floatForKey:@"fontSize"];
    NSFont *font = [NSFont fontWithName:ftname size:ftsize];
    NSString* text = [NSString stringWithFormat:@"%@ %.0f",ftname,ftsize];
    [_ftTxtField setFont:font];
    [_ftTxtField setStringValue:text];
    [_candiWin setFont:font];
}

//add an awakeFromNib item so that we can set the action method.  Note that 
//any menuItems without an action will be disabled when displayed in the Text 
//Input Menud.
-(void)awakeFromNib
{
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center addObserver:self
            selector:@selector(preferencesChanged:)
            name:NSUserDefaultsDidChangeNotification
            object:nil];
            
    [GrowlApplicationBridge setGrowlDelegate: self];
}

//though we specified the showPrefPanel: in SunPinyinApplicationDelegate as the
//action receiver, the IMKInputController will actually receive the event. 
-(IBAction)showPrefPanel:(id)sender
{
    [_prefPanel makeKeyAndOrderFront:sender];
}

-(IBAction)showFontPanel:(id)sender
{
    NSFont *oldFont = [_candiWin font];
    NSFontPanel* ftDlg = [NSFontPanel sharedFontPanel];
    [ftDlg setDelegate:self];
    [ftDlg setPanelFont:oldFont isMultiple:NO];
    [ftDlg makeKeyAndOrderFront:sender];
}

-(void) changeFont:(id)sender
{
    NSFont *oldFont =[_candiWin font];
    NSFont *newFont = [sender convertFont:oldFont];
    
    NSUserDefaults* pref = [NSUserDefaults standardUserDefaults];
    NSString* ftname = [newFont familyName];
    float ftsize = [newFont pointSize];
    [pref setObject:ftname forKey:@"fontName"];
    [pref setFloat:ftsize forKey:@"fontSize"];
}

-(void) changeAttributes:(id)sender
{
    NSColor *fgColor = [_candiWin fgColor];
    NSDictionary *oldAttributes = [NSDictionary dictionaryWithObject:fgColor forKey:@"NSColor"];
    NSDictionary *newAttributes = [sender convertAttributes: oldAttributes];

    fgColor = [newAttributes objectForKey:@"NSColor"];
    [_ftTxtField setTextColor:fgColor];

    NSUserDefaults* pref = [NSUserDefaults standardUserDefaults];
    NSData *data=[NSArchiver archivedDataWithRootObject:fgColor];
    [pref setObject:data forKey:@"fgColor"];
}

-(IBAction)toggleChinesePuncts:(id)sender
{
    NSMenuItem *item = [_menu itemWithTag:0];
    _inputChinesePuncts = ![item state];
    [[NSUserDefaults standardUserDefaults] setBool:_inputChinesePuncts 
                                           forKey:@"inputChinesePuncts"];
}

-(bool)inputChinesePuncts
{
    return _inputChinesePuncts;
}

-(IBAction)toggleFullSymbols:(id)sender
{
    NSMenuItem *item = [_menu itemWithTag:1];
    _inputFullSymbols = ![item state];
    [[NSUserDefaults standardUserDefaults] setBool:_inputFullSymbols 
                                           forKey:@"inputFullSymbols"];
}

-(bool)inputFullSymbols
{
    return _inputFullSymbols;
}

-(SwitchingPolicies)switchingPolicy
{
    return _switchingPolicy;
}

-(bool)usingUSKbLayout
{
    return _usingUSKbLayout;
}

-(void)dealloc 
{
    delete _data;
    delete _history;
    delete _pref;
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [super dealloc];
}

-(NSDictionary *)registrationDictionaryForGrowl
{
    NSArray *notifications;
    notifications = [NSArray arrayWithObject: @"SunPinyin"];

    NSDictionary *dict;
    dict = [NSDictionary dictionaryWithObjectsAndKeys:
                         notifications, GROWL_NOTIFICATIONS_ALL,
                         notifications, GROWL_NOTIFICATIONS_DEFAULT, nil];
           
    return (dict);
}

-(void)messageNotify:(NSString*)msg
{
    [GrowlApplicationBridge notifyWithTitle: @"SunPinyin"
                            description: msg
                            notificationName: @"SunPinyin"
                            iconData: [NSData dataWithData:[[NSImage imageNamed:@"SunPinyin"] TIFFRepresentation]]
                            priority: 0
                            isSticky: NO
                            clickContext: nil];
}

@end

//this method is to return the path to history cache file.
static NSString* get_history_path ()
{
    NSString* path = [NSHomeDirectory() stringByAppendingPathComponent:@"Library/SunPinyin"];
    NSFileManager* fm = [NSFileManager defaultManager];
    if(![fm fileExistsAtPath:path])
        [fm createDirectoryAtPath:path attributes:nil];

    return [path stringByAppendingPathComponent:@"history"];
}

static bool save_history (CICHistory* history)
{
    bool suc = NO;
    size_t sz = 0;
    void* buf = NULL;
    NSString* path = get_history_path ();

    if (history->bufferize(&buf, &sz) && buf) {
        FILE* fp = fopen ([path UTF8String], "w+b");
        if (fp) {
            suc = (fwrite(buf, 1, sz, fp) == sz);
            fclose(fp);
        }   
        free(buf);
    }

    return suc;
}

static bool load_history (CICHistory* history)
{
    bool suc = NO;

    NSString* path = get_history_path ();
    FILE* fp = fopen([path UTF8String], "rb");

    if (fp) {
        struct stat info;
        fstat(fileno(fp), &info);
        void* buf = malloc(info.st_size);
        if (buf) {
            fread(buf, info.st_size, 1, fp);
            suc = history->loadFromBuffer(buf, info.st_size);
            free(buf);
        }   
        fclose(fp);
    }

    return suc;
}

static bool load_preferences (CSunpinyinOptions* opts)
{
    NSUserDefaults* pref = [NSUserDefaults standardUserDefaults];
    opts->m_ViewType = [pref integerForKey: @"inputStyle"]?
                       CIMIViewFactory::SVT_CLASSIC:
                       CIMIViewFactory::SVT_MODERN;
    opts->m_MinusAsPageUp = [pref boolForKey:@"pagingByMinusAndEqual"];
    opts->m_BracketAsPageUp = [pref boolForKey:@"pagingByBrackets"];
    opts->m_CommaAsPageUp = [pref boolForKey:@"pagingByCommaAndDot"];
    opts->m_CandiWindowSize = [pref integerForKey:@"candiNumbers"];
    opts->m_GBK = [[pref stringForKey:@"charset"] isEqualToString:@"GBK"];

    return YES;
}


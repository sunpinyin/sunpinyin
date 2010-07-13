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
#import "imi_option_keys.h"
#import "imi_session_wrapper.h"
#import "SunPinyinApplicationDelegate.h"
#import <Sparkle/Sparkle.h>

using namespace std;

void updateFactorySettings(NSUserDefaults* pref);
void updateQuanpinSettings(NSUserDefaults* pref);
void updateShuangpinSettings(NSUserDefaults* pref);
void updateSimplifiedChineseSettings(NSUserDefaults* pref);
void updateKeyProfileSettings(NSUserDefaults* pref);

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

-(void)updateUISettings
{
    NSUserDefaults* pref = [NSUserDefaults standardUserDefaults];
    
    //setting background color
    NSData *data = [pref dataForKey:@"bgColor"];
    NSColor *color = data? (NSColor*) [NSUnarchiver unarchiveObjectWithData:data]:
    [NSColor orangeColor];
    
    float alpha = [pref floatForKey:@"alpha"]/100.0;
    NSColor *bgColor = [color colorWithAlphaComponent:alpha];
    [_ftTxtField setBackgroundColor:color];
    [_candiWin setBgColor:bgColor];
    
    float radius = [pref floatForKey:@"radius"];
    [_candiWin setRadius:radius];

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

-(void)updateIMESettings
{
    NSUserDefaults* pref = [NSUserDefaults standardUserDefaults];
    
    //setting full/half puncts and symbols
    _inputChinesePuncts = [pref boolForKey:@"inputChinesePuncts"];
    _inputFullSymbols   = [pref boolForKey:@"inputFullSymbols"];
    _switchingPolicy    = (SwitchingPolicies) [pref integerForKey:@"switchingPolicy"];
    _commitPolicy       = (CommitPolicies) [pref integerForKey:@"commitPolicy"];
    _usingUSKbLayout    = [pref boolForKey:@"usingUSKbLayout"];
    
    updateFactorySettings(pref);
    updateQuanpinSettings(pref);
    updateShuangpinSettings(pref);
    updateSimplifiedChineseSettings(pref);
    updateKeyProfileSettings(pref);
}

-(void)loadPreferences
{
    [self updateUISettings];
    [self updateIMESettings];
}

//add an awakeFromNib item so that we can set the action method.  Note that 
//any menuItems without an action will be disabled when displayed in the Text 
//Input Menud.
-(void)awakeFromNib
{
    [self loadPreferences];
    
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    [center addObserver:self
            selector:@selector(preferencesChanged:)
            name:NSUserDefaultsDidChangeNotification
            object:nil];

    [GrowlApplicationBridge setGrowlDelegate: self];
    
    string res_path = [[[NSBundle mainBundle] resourcePath] UTF8String];
    AOptionEventBus::instance().publishEvent(COptionEvent(SYSTEM_DATA_DIR, res_path));

    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
    if ([paths count]) 
    {
        string user_data_dir = [[paths objectAtIndex: 0] UTF8String];
        user_data_dir.append ("/SunPinyin");
        AOptionEventBus::instance().publishEvent(COptionEvent(USER_DATA_DIR, user_data_dir));
    }
}

-(void)preferencesChanged:(NSNotification *)notification
{
    if ([[notification name] compare: @"NSUserDefaultsDidChangeNotification"])
        return;
    
    [self loadPreferences];
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
    NSString* ftname = [newFont fontName];
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

-(CommitPolicies)commitPolicy
{
    return _commitPolicy;
}

-(bool)usingUSKbLayout
{
    return _usingUSKbLayout;
}

-(void)dealloc 
{
    delete _data;
    delete _history;
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

@end //SunPinyinApplicationDelegate

void updateFactorySettings(NSUserDefaults* pref)
{
    CSunpinyinSessionFactory& factory = CSunpinyinSessionFactory::getFactory();
    COptionEventBus& event_bus = AOptionEventBus::instance();
    
    int pinyin_scheme = [pref integerForKey: @"pinyinMode"];
    factory.setPinyinScheme((CSunpinyinSessionFactory::EPyScheme)pinyin_scheme);
    /* pinyin mode is not a shared configuration item, need to broadcast to session wrapper */
    event_bus.publishEvent(COptionEvent(PINYIN_SCHEME, pinyin_scheme));      
    
    int candi_number = [pref integerForKey:@"candiNumbers"];
    factory.setCandiWindowSize(candi_number);
    /* candiWindowSize is not a shared configuration item */
    event_bus.publishEvent(COptionEvent(CONFIG_GENERAL_PAGE_SIZE, candi_number));
}

void updateQuanpinSettings(NSUserDefaults* pref)
{    
    CQuanpinSchemePolicy& quanpin_policy = AQuanpinSchemePolicy::instance();
    CShuangpinSchemePolicy& shuangpin_policy = AShuangpinSchemePolicy::instance();
    
    bool simpler_initials_enabled = [pref boolForKey:@"Quanpin.Fuzzy.SimplerInitials"];    
    bool quanpin_fuzzy_enabled = [pref boolForKey: @"Quanpin.Fuzzy.Enabled"];
     
    quanpin_policy.setFuzzyForwarding   (quanpin_fuzzy_enabled, simpler_initials_enabled);
    shuangpin_policy.setFuzzyForwarding (quanpin_fuzzy_enabled, simpler_initials_enabled);
    
    quanpin_policy.clearFuzzyPinyinPairs();
    shuangpin_policy.clearFuzzyPinyinPairs();

    if (simpler_initials_enabled)
    {
        string_pairs fuzzy_pairs;
        
        fuzzy_pairs.push_back(make_pair("z",  "zh"));
        fuzzy_pairs.push_back(make_pair("c",  "ch"));
        fuzzy_pairs.push_back(make_pair("s",  "sh"));
        
        quanpin_policy.setFuzzyPinyinPairs   (fuzzy_pairs, false);
        shuangpin_policy.setFuzzyPinyinPairs (fuzzy_pairs, false);        
    }
    
    if (quanpin_fuzzy_enabled)
    {
        string_pairs fuzzy_pairs;
        
        if ([pref boolForKey: @"Quanpin.Fuzzy.ZhiZi"])   fuzzy_pairs.push_back(make_pair("zh",  "z"));
        if ([pref boolForKey: @"Quanpin.Fuzzy.ChiCi"])   fuzzy_pairs.push_back(make_pair("ch",  "c"));
        if ([pref boolForKey: @"Quanpin.Fuzzy.ShiSi"])   fuzzy_pairs.push_back(make_pair("sh",  "s"));
        if ([pref boolForKey: @"Quanpin.Fuzzy.AnAng"])   fuzzy_pairs.push_back(make_pair("an",  "ang"));
        if ([pref boolForKey: @"Quanpin.Fuzzy.OnOng"])   fuzzy_pairs.push_back(make_pair("on",  "ong"));
        if ([pref boolForKey: @"Quanpin.Fuzzy.EnEng"])   fuzzy_pairs.push_back(make_pair("en",  "eng"));
        if ([pref boolForKey: @"Quanpin.Fuzzy.InIng"])   fuzzy_pairs.push_back(make_pair("in",  "ing"));
        if ([pref boolForKey: @"Quanpin.Fuzzy.EngOng"])  fuzzy_pairs.push_back(make_pair("eng", "ong"));
        if ([pref boolForKey: @"Quanpin.Fuzzy.IanIang"]) fuzzy_pairs.push_back(make_pair("ian", "iang"));
        if ([pref boolForKey: @"Quanpin.Fuzzy.UanUang"]) fuzzy_pairs.push_back(make_pair("uan", "uang"));
        if ([pref boolForKey: @"Quanpin.Fuzzy.NeLe"])    fuzzy_pairs.push_back(make_pair("n",   "l"));
        if ([pref boolForKey: @"Quanpin.Fuzzy.FoHe"])    fuzzy_pairs.push_back(make_pair("f",   "h"));
        if ([pref boolForKey: @"Quanpin.Fuzzy.LeRi"])    fuzzy_pairs.push_back(make_pair("l",   "r"));
        if ([pref boolForKey: @"Quanpin.Fuzzy.KeGe"])    fuzzy_pairs.push_back(make_pair("k",   "g"));
        
        quanpin_policy.setFuzzyPinyinPairs   (fuzzy_pairs);
        shuangpin_policy.setFuzzyPinyinPairs (fuzzy_pairs);
    }
        
    bool quanpin_autocorrecting_enabled = [pref boolForKey: @"Quanpin.AutoCorrecting.Enabled"];
    quanpin_policy.setAutoCorrecting (quanpin_autocorrecting_enabled);
    
    string_pairs correcting_pairs;
    
    if ([pref boolForKey: @"Quanpin.AutoCorrecting.GnNg"]) correcting_pairs.push_back(make_pair("gn", "ng"));
    if ([pref boolForKey: @"Quanpin.AutoCorrecting.UenUn"])  correcting_pairs.push_back(make_pair("uen", "un"));
    if ([pref boolForKey: @"Quanpin.AutoCorrecting.ImgIng"]) correcting_pairs.push_back(make_pair("img", "ing"));
    if ([pref boolForKey: @"Quanpin.AutoCorrecting.IouIu"])  correcting_pairs.push_back(make_pair("iou", "iu"));
    if ([pref boolForKey: @"Quanpin.AutoCorrecting.UeiUi"])  correcting_pairs.push_back(make_pair("uei", "ui"));
    
    quanpin_policy.setAutoCorrectionPairs (correcting_pairs);

    bool quanpin_fuzzy_segments_enabled       = [pref boolForKey:@"Quanpin.FuzzySegs.Enabled"];
    bool quanpin_inner_fuzzy_segments_enabled = [pref boolForKey:@"Quanpin.FuzzySegs.InnerFuzzy.Enabled"];
    quanpin_policy.setFuzzySegmentation (quanpin_fuzzy_segments_enabled);
    quanpin_policy.setInnerFuzzySegmentation (quanpin_inner_fuzzy_segments_enabled);
}

void updateShuangpinSettings(NSUserDefaults* pref)
{
    /* Shuangpin type is not a shared configuration item */
    COptionEventBus& event_bus = AOptionEventBus::instance();    
    int  shuangpin_type = [pref integerForKey: @"Shuangpin.Scheme"];
    event_bus.publishEvent(COptionEvent(SHUANGPIN_TYPE, shuangpin_type));    
}

void updateSimplifiedChineseSettings(NSUserDefaults* pref)
{    
    CSimplifiedChinesePolicy& lang_policy = ASimplifiedChinesePolicy::instance();
    
    lang_policy.enableFullSymbol ([pref boolForKey:@"inputFullSymbols"]);
    lang_policy.enableFullPunct ([pref boolForKey:@"inputChinesePuncts"]);
        
    NSString *charset = [pref stringForKey:@"charset"];
    int charset_level = [charset isEqualToString:@"GB18030"]? 2: 
                            [charset isEqualToString:@"GBK"]? 1: 0;
    
    /* charset is not a shared configuration item */
    AOptionEventBus::instance().publishEvent (COptionEvent(CONFIG_GENERAL_CHARSET_LEVEL, charset_level));    
    
    string_pairs punct_pairs;
    NSArray* punct_mappings = [pref arrayForKey: @"punctationMappings"];
    for (int i=0; i<[punct_mappings count]; ++i)
    {
        NSDictionary *tmp = [punct_mappings objectAtIndex:i];
        NSNumber *enabled = [tmp objectForKey:@"enabled"];
        NSString *ascii = [tmp objectForKey:@"ASCII"];
        if ([ascii isEqualToString:@"SPACE"]) ascii = @" ";
        
        NSString *mapped_string = [tmp objectForKey:@"mappedString"];
        
        if ([enabled boolValue] && [mapped_string UTF8String])
            punct_pairs.push_back (make_pair([ascii UTF8String], [mapped_string UTF8String]));
    }
    lang_policy.setPunctMapping(punct_pairs);    
}

void updateKeyProfileSettings(NSUserDefaults* pref)
{
    /* hotkey profile settings are not shared configuration items */
    COptionEventBus& event_bus = AOptionEventBus::instance(); 
    
    bool paging_by_minus_equals = [pref boolForKey:@"pagingByMinusAndEqual"];
    event_bus.publishEvent (COptionEvent(CONFIG_KEYBOARD_PAGE_MINUS, paging_by_minus_equals));
    
    bool paging_by_brackets = [pref boolForKey:@"pagingByBrackets"];
    event_bus.publishEvent (COptionEvent(CONFIG_KEYBOARD_PAGE_BRACKET, paging_by_brackets));
    
    bool paging_by_comma_period = [pref boolForKey:@"pagingByCommaAndDot"];
    event_bus.publishEvent (COptionEvent(CONFIG_KEYBOARD_PAGE_COMMA, paging_by_comma_period));

    bool paging_by_arrows = [pref boolForKey:@"pagingByArrowUpAndDown"];
    event_bus.publishEvent (COptionEvent(CONFIG_KEYBOARD_PAGE_ARROWS, paging_by_arrows));
        
    bool cancel_on_backspace = [pref boolForKey:@"cancelSelectionOnBackspace"];
    event_bus.publishEvent(COptionEvent(CONFIG_KEYBOARD_MISC_CANCELONBSP, cancel_on_backspace));
    
    // store the session specific configurations
    CSessionConfigStore::instance().m_paging_by_minus_equals = paging_by_minus_equals;
    CSessionConfigStore::instance().m_paging_by_brackets     = paging_by_brackets;        
    CSessionConfigStore::instance().m_paging_by_comma_period = paging_by_comma_period;        
    CSessionConfigStore::instance().m_paging_by_arrows       = paging_by_arrows;
    CSessionConfigStore::instance().m_cancel_on_backspace    = cancel_on_backspace;
}



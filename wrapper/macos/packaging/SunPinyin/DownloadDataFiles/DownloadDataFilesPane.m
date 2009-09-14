//
//  DownloadDataFilesPane.m
//  DownloadDataFiles
//
//  Created by Jjgod Jiang on 4/21/09.

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

#import "DownloadDataFilesPane.h"
#import <CommonCrypto/CommonDigest.h>
#import <SecurityFoundation/SFAuthorization.h>

#define JJLocalizedString(key, comment) NSLocalizedStringFromTableInBundle(key, @"Localizable", [NSBundle bundleForClass: [self class]], comment)

#if __BIG_ENDIAN__
#define kDataFileExt    @".be"
#else
#define kDataFileExt    @".le"
#endif

NSDictionary *findInSpec(NSArray *specArray, NSString *file)
{
    NSString *fileNameByArch = [file stringByAppendingString: kDataFileExt];

    for (NSDictionary *dict in specArray)
        if ([[dict objectForKey: @"Name"] isEqual: fileNameByArch])
            return dict;

    return nil;
}

@interface DownloadDataFilesPane (Private)

- (BOOL) doAuthorizedCopyFromPath: (NSString *) src toPath: (NSString *) dest withAuthorization: (SFAuthorization *) authorization;
- (SFAuthorization *) prepareAuthorization;

@end

@implementation DownloadDataFilesPane

- (void) checkFile: (NSString *) file withSpec: (NSArray *) specArray
{
    NSString *targetDirectory = [self objectForKeyInBundle: @"TargetInstallDirectory"];
    NSString *path = [targetDirectory stringByAppendingPathComponent: file];
    NSDictionary *fileDict = findInSpec(specArray, file);
    
    if (! fileDict)
    {
        NSLog(@"Cannot find %@ in %@", file, specArray);
        return;
    }

    if ([[NSFileManager defaultManager] fileExistsAtPath: path])
    {
        NSString *expectedHash = [fileDict objectForKey: @"SHA256"];
        unsigned char hashedChars[CC_SHA256_DIGEST_LENGTH];
        NSData *data = [NSData dataWithContentsOfFile: path];

        CC_SHA256([data bytes], [data length], hashedChars);
    
        if (expectedHash)
        {
            int i;
            NSMutableString *fileHash = [NSMutableString stringWithCapacity: CC_SHA256_DIGEST_LENGTH * 2];

            for (i = 0; i < sizeof(hashedChars); i++)
                [fileHash appendFormat: @"%02x", hashedChars[i]];

            if (! [fileHash isEqualToString: expectedHash])
                NSLog(@"file hash mismatch: %@ (now) vs. %@ (expected)", fileHash, expectedHash);
            else
                // No need to update
                return;
        }
    }

    [filesToDownload addObject: fileDict];
    totalBytesToDownload += [(NSNumber *)[fileDict objectForKey: @"Size"] unsignedIntValue];
}

- (NSString *) title
{
	return [[NSBundle bundleForClass: [self class]] localizedStringForKey: @"PaneTitle"
                                                                    value: nil
                                                                    table: nil];
}

- (id) objectForKeyInBundle: (NSString *) key
{
	return [[[NSBundle bundleForClass: [self class]] infoDictionary] objectForKey: key];
}

- (void) finishDownload
{
    [mainTextField setStringValue: JJLocalizedString(@"All files are successfully downloaded.",
                                                     "All files downloaded.")];
    [auxTextField setHidden: YES];
    [getButton setHidden: YES];
    [progress setHidden: YES];
    
    [self setNextEnabled: YES];
    [self setPreviousEnabled: YES];
    
    isDownloading = NO;
    
    if (filesDownloaded)
    {
        // authorize
        SFAuthorization *authorization = [self prepareAuthorization];
        
        if (authorization)
        {
            for (NSString *file in filesDownloaded)
            {
                NSString *from = [@"/tmp" stringByAppendingPathComponent: file];
                NSString *to = [[self objectForKeyInBundle: @"TargetInstallDirectory"] stringByAppendingPathComponent: file];
                [self doAuthorizedCopyFromPath: from
                                        toPath: to
                             withAuthorization: authorization];
            }

            // deauthorize
            [authorization invalidateCredentials];
        }
    }
}

- (void) downloadNextFile
{
    NSDictionary *dict;
    isDownloading = YES;
    
    // If all files are downloaded
    if (! [filesToDownload count])
    {
        [self finishDownload];
        return;
    } else
        dict = [filesToDownload objectAtIndex: 0];
    
    NSURL *URL = [NSURL URLWithString: [dict objectForKey: @"URL"]];
    
    NSLog(@"Start downloading: %@", URL);

    // create the request
    NSURLRequest *theRequest = [NSURLRequest requestWithURL: URL
                                                cachePolicy: NSURLRequestUseProtocolCachePolicy
                                            timeoutInterval: 30.0];
    // create the connection with the request
    // and start loading the data
    NSURLDownload  *theDownload = [[NSURLDownload alloc] initWithRequest: theRequest
                                                                delegate: self];
    if (theDownload)
    {
        currentDownload = theDownload;
        NSString *destination = [@"/tmp" stringByAppendingPathComponent:
                                    [[dict objectForKey: @"Name"] stringByDeletingPathExtension]];
        NSLog(@"Set destination to %@", destination);
        // set the destination file now
        [theDownload setDestination: destination
                     allowOverwrite: YES];
    } else {
        // inform the user that the download could not be made
    }
}

- (NSString *) _humanReadableSizeFromDouble: (double) value
{
	if (value < 1024)
		return [NSString stringWithFormat:@"%.0lf %@", value,
                JJLocalizedString(@"B", @"the unit for bytes")];
	
	if (value < 1024 * 1024)
		return [NSString stringWithFormat:@"%.0lf %@", value / 1024.0,
                JJLocalizedString(@"KB", @"the unit for kilobytes")];
	
	if (value < 1024 * 1024 * 1024)
		return [NSString stringWithFormat:@"%.1lf %@", value / 1024.0 / 1024.0,
                JJLocalizedString(@"MB", @"the unit for megabytes")];
	
	return [NSString stringWithFormat:@"%.2lf %@", value / 1024.0 / 1024.0 / 1024.0,
            JJLocalizedString(@"GB", @"the unit for gigabytes")];	
}

- (IBAction) stopDownload: (id) sender
{
    [currentDownload cancel];
    [currentDownload release];
    currentDownload = nil;

    [getButton setTitle: JJLocalizedString(@"Start...", "Start downloading")];
    [getButton setAction: @selector(startDownload:)];
    [progress setDoubleValue: 0];
    [progress setHidden: YES];
    
    [self setNextEnabled: YES];
    [self setPreviousEnabled: YES];
    [auxTextField setStringValue: JJLocalizedString(@"Download stopped.", "Download stopped")];
}

- (IBAction) startDownload: (id) sender
{
    [progress setHidden: NO];
    [progress setDoubleValue: 0];
    
    [getButton setTitle: JJLocalizedString(@"Cancel", "Stop downloading")];
    [getButton setAction: @selector(stopDownload:)];
    [auxTextField setStringValue: [NSString stringWithFormat: JJLocalizedString(@"%@ of %@", nil), 
                                   [self _humanReadableSizeFromDouble: 0],
                                   [self _humanReadableSizeFromDouble: [progress maxValue]]]];
    
    [self setNextEnabled: NO];
    [self setPreviousEnabled: NO];

    [self downloadNextFile];
}

- (void) download: (NSURLDownload *) download didFailWithError: (NSError *) error
{
    [filesToDownload removeObjectAtIndex: 0];
    // release the connection
    [download release];

    [self downloadNextFile];
    // inform the user
    NSLog(@"Download failed! Error - %@ %@",
          [error localizedDescription],
          [[error userInfo] objectForKey: NSErrorFailingURLStringKey]);
}

- (void) download: (NSURLDownload *) download didReceiveDataOfLength: (NSUInteger) length
{
    double value = [progress doubleValue] + length;
    if (value > [progress maxValue])
        value = [progress maxValue];

    [progress setDoubleValue: value];
    [auxTextField setStringValue: [NSString stringWithFormat: JJLocalizedString(@"%@ of %@", nil), 
                                   [self _humanReadableSizeFromDouble: value],
                                   [self _humanReadableSizeFromDouble: [progress maxValue]]]];
}

- (void) downloadDidFinish: (NSURLDownload *) download
{
    // release the connection
    [download release];

    NSString *filename = [[[[[download request] URL] path] lastPathComponent] stringByDeletingPathExtension];
    // do something with the data
    NSLog(@"Finish downloading: %@", filename);

    if ([filesToDownload count])
        [filesToDownload removeObjectAtIndex: 0];

    [filesDownloaded addObject: filename];
    [self downloadNextFile];
}

- (void) startFetchingPropertyListFromURL: (NSURL *) URL
{
    [mainTextField setStringValue: JJLocalizedString(@"Checking updates for data files...", "Checking property list on network")];

    NSURLRequest *request = [NSURLRequest requestWithURL: URL];
    NSURLConnection *connection = [[NSURLConnection alloc] initWithRequest: request
                                                                  delegate: self];
    if (connection)
    {
        receivedData = [[NSMutableData data] retain];
    } else
    {
        // Failed to download plist
        NSLog(@"Failed to open %@", [URL path]);
        [self gotoNextPane];
    }
}

- (void) connection: (NSURLConnection *) connection didReceiveResponse: (NSURLResponse *) response
{
    [receivedData setLength: 0];
}

- (void) connection: (NSURLConnection *) connection didReceiveData: (NSData *) data
{
    [receivedData appendData: data];
}

- (void) connection: (NSURLConnection *) connection didFailWithError: (NSError *) error
{
    // release the connection, and the data object
    [connection release];
    // receivedData is declared as a method instance elsewhere
    [receivedData release];
    
    // inform the user
    NSLog(@"Connection failed! Error - %@ %@",
          [error localizedDescription],
          [[error userInfo] objectForKey: NSErrorFailingURLStringKey]);

    [self gotoNextPane];
}

- (void) setSummaryString: (NSString *) summary detailString: (NSString *) detail
{
    NSDictionary *summaryAttributes, *detailAttributes;
    summaryAttributes = [NSDictionary dictionaryWithObject: [NSFont fontWithName: @"Lucida Grande" size: 13.0]
                                                    forKey: NSFontAttributeName];
    detailAttributes = [NSDictionary dictionaryWithObject: [NSFont fontWithName: @"Monaco" size: 11.0]
                                                   forKey: NSFontAttributeName];

    NSMutableAttributedString *stringToSet = [[NSMutableAttributedString alloc] initWithString: summary
                                                                                    attributes: summaryAttributes];
    NSAttributedString *detailString = [[NSAttributedString alloc] initWithString: detail
                                                                       attributes: detailAttributes];
    [stringToSet appendAttributedString: [[[NSAttributedString alloc] initWithString: @"\n\n"] autorelease]];
    [stringToSet appendAttributedString: detailString];
    [detailString release];
    [mainTextField setAttributedStringValue: stringToSet];
    [stringToSet release];
}

- (void) connectionDidFinishLoading: (NSURLConnection *) connection
{
    [connection release];

    NSString *errorDesc = nil;
    NSArray *specArray = (NSArray *)[NSPropertyListSerialization propertyListFromData: receivedData
                                                                     mutabilityOption: NSPropertyListImmutable
                                                                               format: NULL
                                                                     errorDescription: &errorDesc];
    [receivedData release];

    if (! specArray)
    {
        NSLog(@"Failed to read property list: %@", errorDesc);
        
        NSString *errorTitle = [NSString stringWithFormat: JJLocalizedString(@"Failed to fetch a valid data file list from %@:", nil),
                                [self objectForKeyInBundle: @"FilesSpecPlistURL"]];
        
        [self setSummaryString: errorTitle
                  detailString: errorDesc];

        [errorDesc release];
        return;
    }

    NSArray *files = [NSArray arrayWithObjects: @"lm_sc.t3g", @"pydict_sc.bin", nil];

    if (! filesToDownload)
        filesToDownload = [[NSMutableArray alloc] initWithCapacity: 2];
    
    if (! filesDownloaded)
        filesDownloaded = [[NSMutableArray alloc] initWithCapacity: 2];

    totalBytesToDownload = 0;
    isDownloading = NO;

    for (NSString *file in files)
        [self checkFile: file withSpec: specArray];
    
    if ([filesToDownload count])
    {
        NSString *text = [NSString stringWithFormat: JJLocalizedString(@"The following files need to be downloaded and save to\n%@:",
                                                                       "Need To Download"),
                          [self objectForKeyInBundle: @"TargetInstallDirectory"]];
        NSMutableString *urls = [NSMutableString stringWithCapacity: 50];
        for (NSDictionary *dict in filesToDownload)
            [urls appendFormat: @"%@\n", [dict objectForKey: @"URL"]];

        [self setSummaryString: text
                  detailString: urls];

        [auxTextField setStringValue: JJLocalizedString(@"Click the Start button to download them automatically.",
                                                        "Click Download Button")];
        [getButton setTitle: JJLocalizedString(@"Start...", "Start downloading")];
        [getButton setHidden: NO];
        [progress setStyle: NSProgressIndicatorBarStyle];
        [progress setMaxValue: totalBytesToDownload];
        [progress setIndeterminate: NO];
    }
    else
        [mainTextField setStringValue: JJLocalizedString(@"You already have the latest data files, just Continue.", "Bypass")];
}

/* pane's entry point: code called when user enters this pane */
- (void) didEnterPane: (InstallerSectionDirection) dir
{
    NSURL *plistURL = [NSURL URLWithString: [self objectForKeyInBundle: @"FilesSpecPlistURL"]];
    [self startFetchingPropertyListFromURL: plistURL];
}

/* called when user clicks "Continue" -- return value indicates if application should exit pane */
- (BOOL) shouldExitPane: (InstallerSectionDirection) dir
{
    if (isDownloading)
        return NO;
    
    [filesToDownload release];
    filesToDownload = nil;

	return YES;
}

- (SFAuthorization *) prepareAuthorization
{
    // authorize
    AuthorizationFlags authFlags =  kAuthorizationFlagPreAuthorize |
    kAuthorizationFlagExtendRights |
    kAuthorizationFlagInteractionAllowed;
    AuthorizationItem authItem = { kAuthorizationRightExecute, 0, nil, 0 };
    AuthorizationRights authRights = { 1, &authItem };
    SFAuthorization *authorization = [SFAuthorization authorizationWithFlags: authFlags
                                                                      rights: &authRights
                                                                 environment: kAuthorizationEmptyEnvironment];
    return authorization;
}

// doAuthorizedCopyFromPath does an authorized copy, getting admin rights
//
// NOTE: when running the task with admin privileges, this waits on any child
// process, since AEWP doesn't tell us the child's pid.  This could be fooled
// by any other child process that quits in the window between launch and
// completion of our actual tool.
- (BOOL) doAuthorizedCopyFromPath: (NSString *) src 
                           toPath: (NSString *) dest
                withAuthorization: (SFAuthorization *) authorization
{    
    // execute the copy
    const char taskPath[] = "/usr/bin/ditto";
    const char* arguments[] = {
        "-rsrcFork",  // 0: copy resource forks; --rsrc requires 10.3
        NULL,  // 1: src path
        NULL,  // 2: dest path
        NULL
    };
    arguments[1] = [src fileSystemRepresentation];
    arguments[2] = [dest fileSystemRepresentation];
    
    FILE **kNoPipe = nil;
    OSStatus status = AuthorizationExecuteWithPrivileges([authorization authorizationRef],
                                                         taskPath,
                                                         kAuthorizationFlagDefaults,
                                                         (char *const *)arguments,
                                                         kNoPipe);
    if (status == errAuthorizationSuccess) {
        int wait_status;
        int pid = wait(&wait_status);
        if (pid == -1 || !WIFEXITED(wait_status))   {
            status = -1;
        } else {
            status = WEXITSTATUS(wait_status);
        }
    }

    return (status == 0);
}

@end

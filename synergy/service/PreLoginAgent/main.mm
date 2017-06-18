/*
     File: main.m
 Abstract: Application main.
  Version: 1.1
 
 Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
 Inc. ("Apple") in consideration of your agreement to the following
 terms, and your use, installation, modification or redistribution of
 this Apple software constitutes acceptance of these terms.  If you do
 not agree with these terms, please do not use, install, modify or
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and
 subject to these terms, Apple grants you a personal, non-exclusive
 license, under Apple's copyrights in this original Apple software (the
 "Apple Software"), to use, reproduce, modify and redistribute the Apple
 Software, with or without modifications, in source and/or binary forms;
 provided that if you redistribute the Apple Software in its entirety and
 without modifications, you must retain this notice and the following
 text and disclaimers in all such redistributions of the Apple Software.
 Neither the name, trademarks, service marks or logos of Apple Inc. may
 be used to endorse or promote products derived from the Apple Software
 without specific prior written permission from Apple.  Except as
 expressly stated in this notice, no other rights or licenses, express or
 implied, are granted by Apple herein, including but not limited to any
 patent rights that may be infringed by your derivative works or by other
 works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
 MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
 THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
 FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
 OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
 OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
 AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
 STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 
 Copyright (C) 2014 Apple Inc. All Rights Reserved.
 */

#import <Cocoa/Cocoa.h>
#import "LogManager.h"
#include <boost/process.hpp>
#include <fstream>

std::string const kSharedConfigPath ("/Users/Shared/Synergy");
std::string const kAppPath ("/Applications/Synergy.app");
auto const kAppClientExecPath = kAppPath + "/Contents/MacOS/synergyc";

static void WaitForWindowServerSession(void)
    // This routine waits for the window server to register its per-session 
    // services in our session.  This code was necessary in various pre-release 
    // versions of Mac OS X 10.5, but it is not necessary on the final version. 
    // However, I've left it in, and the option to enable it, to give me the 
    // flexibility to test this edge case.
{
    [[LogManager sharedManager] logWithFormat:@"Waiting for window server session begin"];
    
    do {
        NSDictionary *  sessionDict;

        sessionDict = CFBridgingRelease( CGSessionCopyCurrentDictionary() );
        if (sessionDict != nil) {
            break;
        }
        [[LogManager sharedManager] logWithFormat:@"No window server session, wait"];
        sleep(1);
    } while (YES);

    [[LogManager sharedManager] logWithFormat:@"Waiting for window server session end"];
}

static void InstallHandleSIGTERMFromRunLoop(void)
    // This routine installs a SIGTERM handler that's called on the main thread, allowing 
    // it to then call into Cocoa to quit the app.
{
    static dispatch_once_t   sOnceToken;
    static dispatch_source_t sSignalSource;

    dispatch_once(&sOnceToken, ^{
        signal(SIGTERM, SIG_IGN);
    
        sSignalSource = dispatch_source_create(DISPATCH_SOURCE_TYPE_SIGNAL, SIGTERM, 0, dispatch_get_main_queue());
        assert(sSignalSource != NULL);
        
        dispatch_source_set_event_handler(sSignalSource, ^{
            assert([NSThread isMainThread]);
            
            [[LogManager sharedManager] logWithFormat:@"Got SIGTERM"];

            [[NSApplication sharedApplication] terminate:nil];
        });
        
        dispatch_resume(sSignalSource);
    });
}

static std::ofstream&
log() {
    static std::ofstream log_ (kSharedConfigPath + "/pla.log", std::ios::out | std::ios::app);
    return log_;
}

int
main (int argc, char *argv[])
{
    int             retVal;
    NSTimeInterval  delay;
    
    [[LogManager sharedManager] logWithFormat:@"Start"];
    
    // Register the default defaults, so to speak.
    
    [[NSUserDefaults standardUserDefaults] registerDefaults:@{
        @"DelayStartup":               @0.0, 
        @"WaitForWindowServerSession": @NO,
        @"ForceOrderFront":            @YES,
        @"CleanExit":                  @YES
    }];
    
    // Handle various options startup options.
    
    if ([[NSUserDefaults standardUserDefaults] boolForKey:@"WaitForWindowServerSession"]) {
        WaitForWindowServerSession();
    } else {
        [[LogManager sharedManager] logWithFormat:@"Not waiting for CGSessionCopyCurrentDictionary"];
    }
    
    delay = [[NSUserDefaults standardUserDefaults] doubleForKey:@"DelayStartup"];
    if (delay > 0.0) {
        [[LogManager sharedManager] logWithFormat:@"Delaying %.1f", delay];
        [NSThread sleepForTimeInterval:delay];
    } else {
        [[LogManager sharedManager] logWithFormat:@"Not delaying"];
    }
    
    if ([[NSUserDefaults standardUserDefaults] boolForKey:@"CleanExit"]) {
        [[LogManager sharedManager] logWithFormat:@"Installing SIGTERM handler"];
        InstallHandleSIGTERMFromRunLoop();
    } else {
        [[LogManager sharedManager] logWithFormat:@"Not installing SIGTERM handler"];
    }
    
    // Go go gadget Cocoa!
    
    [[LogManager sharedManager] logWithFormat:@"Starting Cocoa application"];

    [[LogManager sharedManager] logWithFormat:@"Starting synergyc"];

    boost::process::ipstream synergyc_out;
    boost::process::child synergyc (kAppClientExecPath, "-f", "--debug", "DEBUG2",
                                    "192.168.1.71",
                                   boost::process::std_out > synergyc_out,
                                   boost::process::std_err > synergyc_out);
    std::string line;
    while (synergyc_out && std::getline(synergyc_out, line)) {
        // This should block and never spin
        if (!line.empty()) {
            log() << line << std::endl;
            line.clear();
        }
    }

    synergyc.wait();
    log().close();

    retVal = NSApplicationMain(argc, (const char **) argv);

    [[LogManager sharedManager] logWithFormat:@"Cocoa application returned!?"];
    
    return 0;
}

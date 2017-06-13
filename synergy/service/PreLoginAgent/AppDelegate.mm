/*
     File: AppDelegate.m
 Abstract: Main app controller..
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

#import "AppDelegate.h"

#import "LogManager.h"

@interface AppDelegate () <NSApplicationDelegate>

@property (nonatomic, assign, readwrite) IBOutlet NSPanel *     panel;

@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)note
{
    #pragma unused(note)
    
    [[LogManager sharedManager] logWithFormat:@"Did finish launching begin"];

    assert(self.panel != nil);

    // We have to call -[NSWindow setCanBecomeVisibleWithoutLogin:] to let the 
    // system know that we're not accidentally trying to display a window 
    // pre-login.
    
    [self.panel setCanBecomeVisibleWithoutLogin:YES];
    
    // Our application is a UI element which never activates, so we want our 
    // panel to show regardless.
    
    [self.panel setHidesOnDeactivate:NO];

    // Due to a problem with the relationship between the UI frameworks and the 
    // window server <rdar://problem/5136400>, -[NSWindow orderFront:] is not 
    // sufficient to show the window.  We have to use -[NSWindow orderFrontRegardless].

    if ([[NSUserDefaults standardUserDefaults] boolForKey:@"ForceOrderFront"]) {
        [[LogManager sharedManager] logWithFormat:@"Showing window with extreme prejudice"];
        [self.panel orderFrontRegardless];
    } else {
        [[LogManager sharedManager] logWithFormat:@"Showing window normally"];
        [self.panel orderFront:self];
    }

    [[LogManager sharedManager] logWithFormat:@"Did finish launching end"];
}

- (void)applicationWillTerminate:(NSNotification *)note
{
    #pragma unused(note)
    [[LogManager sharedManager] logWithFormat:@"Will terminate"];
}

@end

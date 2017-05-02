#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#import <ApplicationServices/ApplicationServices.h>

extern "C++" {

bool
processHasAccessibility() {
    NSDictionary* options = @{static_cast<id> (kAXTrustedCheckOptionPrompt): @NO};
    return AXIsProcessTrustedWithOptions(static_cast<CFDictionaryRef> (options));
}

void
openAccessibilityDialog() {
    NSString* urlString = @"x-apple.systempreferences:com.apple.preference.security?Privacy_Accessibility";
    [[NSWorkspace sharedWorkspace] openURL: [NSURL URLWithString:urlString]];
}

} // extern "C++"

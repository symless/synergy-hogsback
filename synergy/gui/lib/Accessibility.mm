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
    NSDictionary* options = @{static_cast<id> (kAXTrustedCheckOptionPrompt): @YES};
    return AXIsProcessTrustedWithOptions(static_cast<CFDictionaryRef> (options));
}

} // extern "C++"

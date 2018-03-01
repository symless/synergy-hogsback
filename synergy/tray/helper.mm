#import <AppKit/AppKit.h>

extern "C++" {
void hideDockIcon(){
    [NSApp setActivationPolicy: NSApplicationActivationPolicyAccessory];
}
} // extern "C++"

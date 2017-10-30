#import <DiskArbitration/DiskArbitration.h>
#import <Foundation/NSURL.h>
#import <Foundation/NSDictionary.h>
#include <iostream>

extern "C++"
void unmountDMG (char const*);

static BOOL
unmountDMGAtURL (NSURL* url) {
    if (!url.isFileURL) {
        return NO;
    }

    BOOL isDMG = NO;

    DASessionRef session = DASessionCreate(kCFAllocatorDefault);
    if (session != nil) {
        DADiskRef disk = DADiskCreateFromVolumePath (kCFAllocatorDefault, session,
                                                     (__bridge CFURLRef) url);
        if (disk != nil) {
            NSDictionary* desc = CFBridgingRelease (DADiskCopyDescription (disk));
            NSString* model = desc[(NSString*) kDADiskDescriptionDeviceModelKey];
            isDMG = ([model isEqualToString:@"Disk Image"]);
            if (isDMG) {
                DADiskUnmount (disk, kDADiskUnmountOptionDefault, nil, nil);
            }
            CFRelease (disk);
        }

        CFRelease (session);
    }

    return isDMG;
}

extern "C++"
void unmountDMG (char const* path) {
    NSURL* url = [[NSURL alloc] initFileURLWithPath: [NSString stringWithUTF8String: path]];
    if (unmountDMGAtURL (url)) {
        std::clog << "Unmounting " << path << "... \n";
    }
}

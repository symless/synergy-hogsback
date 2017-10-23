#import <Cocoa/Cocoa.h>
#import <ServiceManagement/ServiceManagement.h>
#import <Security/Authorization.h>

static BOOL
blessHelperWithLabel (NSString* label, AuthorizationRef& authRef, NSError** errorPtr)
{
    BOOL result = NO;
    NSError * error = nil;

    AuthorizationItem authItem		= { kSMRightBlessPrivilegedHelper, 0, NULL, 0 };
    AuthorizationRights authRights	= { 1, &authItem };
    AuthorizationFlags flags		=	kAuthorizationFlagDefaults				|
                                        kAuthorizationFlagInteractionAllowed	|
                                        kAuthorizationFlagPreAuthorize			|
                                        kAuthorizationFlagExtendRights;

    /* Obtain the right to install our privileged helper tool (kSMRightBlessPrivilegedHelper). */
    OSStatus status = AuthorizationCopyRights(authRef, &authRights,
                                              kAuthorizationEmptyEnvironment, flags, NULL);
    if (status != errAuthorizationSuccess) {
        error = [NSError errorWithDomain:NSOSStatusErrorDomain code:status userInfo:nil];
    } else {
        /* This does all the work of verifying the helper tool against the application
         * and vice-versa. Once verification has passed, the embedded launchd.plist
         * is extracted and placed in /Library/LaunchDaemons and then loaded. The
         * executable is placed in /Library/PrivilegedHelperTools.
         */
        CFErrorRef cfError;
        result = (BOOL) SMJobBless (kSMDomainSystemLaunchd, (CFStringRef) label, authRef, &cfError);
        if (!result) {
            error = CFBridgingRelease(cfError);
        }
    }
    if (!result && (errorPtr != NULL)) {
        assert (error != nil);
        *errorPtr = error;
    }

    return result;
}

static BOOL
blessServiceHelper (AuthorizationRef& authRef, NSError** errorPtr) {
    return blessHelperWithLabel (@"com.symless.synergy.v2.ServiceHelper", authRef, errorPtr);
}

extern "C++" {

bool
installServiceHelper() {
    AuthorizationRef authRef = NULL;
    NSError* error = nil;

    OSStatus status = AuthorizationCreate (NULL, kAuthorizationEmptyEnvironment, kAuthorizationFlagDefaults, &authRef);
    if (status != errAuthorizationSuccess) {
        /* AuthorizationCreate should never fail. */
        assert (NO);
    }

    if (!blessServiceHelper (authRef, &error)) {
        NSLog (@"Failed to bless helper %@ / %d", [error domain], (int) [error code]);
    } else {
        NSLog (@"Helper installed successfully");
        return true;
    }

    return false;
}

}

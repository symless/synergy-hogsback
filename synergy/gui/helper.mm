#import <Cocoa/Cocoa.h>
#import <ServiceManagement/ServiceManagement.h>
#import <Security/Authorization.h>

BOOL
blessHelperWithLabel (NSString* label, NSError** errorPtr, AuthorizationRef& authRef)
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
    OSStatus status = AuthorizationCopyRights(authRef, &authRights, kAuthorizationEmptyEnvironment, flags, NULL);
    if (status != errAuthorizationSuccess) {
        error = [NSError errorWithDomain:NSOSStatusErrorDomain code:status userInfo:nil];
    } else {
        CFErrorRef cfError;
        /* This does all the work of verifying the helper tool against the application
         * and vice-versa. Once verification has passed, the embedded launchd.plist
         * is extracted and placed in /Library/LaunchDaemons and then loaded. The
         * executable is placed in /Library/PrivilegedHelperTools.
         */
        result = (BOOL) SMJobBless(kSMDomainSystemLaunchd, (CFStringRef) label, authRef, &cfError);
        if (!result) {
            error = CFBridgingRelease(cfError);
        }
    }
    if (!result && (errorPtr != NULL)) {
        assert(error != nil);
        *errorPtr = error;
    }

    return result;
}

void
runHelper() {
    AuthorizationRef authRef;
    NSError *error = nil;

    OSStatus status = AuthorizationCreate(NULL, kAuthorizationEmptyEnvironment, kAuthorizationFlagDefaults, &authRef);
    if (status != errAuthorizationSuccess) {
        /* AuthorizationCreate really shouldn't fail. */
        assert(NO);
        authRef = NULL;
    }

    if (!blessHelperWithLabel (@"com.symless.synergy.v2.ServiceHelper", &error, authRef)) {
        NSLog (@"Something went wrong! %@ / %d", [error domain], (int) [error code]);
    } else {
        /* At this point, the job is available. However, this is a very
         * simple sample, and there is no IPC infrastructure set up to
         * make it launch-on-demand. You would normally achieve this by
         * using XPC (via a MachServices dictionary in your launchd.plist).
         */
        NSLog (@"Job is available!");
    }
}

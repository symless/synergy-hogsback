#import <Cocoa/Cocoa.h>
#import <ServiceManagement/ServiceManagement.h>
#import <Security/Authorization.h>

static BOOL
blessHelperWithLabel (NSString* label, AuthorizationRef& authRef, NSError** errorPtr)
{
    BOOL result = NO;
    NSError* error = nil;

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

    if (!result && (errorPtr != NULL)) {
        assert (error != nil);
        *errorPtr = error;
    }

    return result;
}

static AuthorizationRef g_helperAuthRef = NULL;

static BOOL
blessServiceHelper (NSError** errorPtr) {
    return blessHelperWithLabel (@"com.symless.synergy.v2.ServiceHelper", g_helperAuthRef, errorPtr);
}

extern "C++" {

bool
authorizeServiceHelper() {
    if (g_helperAuthRef != NULL) {
        return true;
    }

    AuthorizationRef authRef = NULL;

    OSStatus status = AuthorizationCreate (NULL, kAuthorizationEmptyEnvironment, kAuthorizationFlagDefaults, &authRef);
    if (status != errAuthorizationSuccess) {
        /* AuthorizationCreate should never fail. */
        assert (NO);
    }

    /* Obtain the right to install our privileged helper tool (kSMRightBlessPrivilegedHelper). */
    AuthorizationItem authItem		= { kSMRightBlessPrivilegedHelper, 0, NULL, 0 };
    AuthorizationRights authRights	= { 1, &authItem };
    AuthorizationFlags flags		=	kAuthorizationFlagDefaults				|
                                        kAuthorizationFlagInteractionAllowed	|
                                        kAuthorizationFlagPreAuthorize			|
                                        kAuthorizationFlagExtendRights;
    status = AuthorizationCopyRights (authRef, &authRights,
                                      kAuthorizationEmptyEnvironment, flags, NULL);
    if (status == errAuthorizationSuccess) {
        g_helperAuthRef = authRef;
        return true;
    }

    return false;
}

bool
installServiceHelper() {
    if (!authorizeServiceHelper()) {
        return false;
    }

    NSError* error = nil;
    if (blessServiceHelper (&error)) {
        NSLog (@"Failed to bless service helper %@ / %d", [error domain], (int) [error code]);
        return false;
    } else {
        NSLog (@"Service helper installed successfully");
        return true;
    }
}

} // extern "C++"

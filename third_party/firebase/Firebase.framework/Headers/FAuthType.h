//
//  FAuthType.h
//  Firebase
//
//  Created by Katherine Fang on 7/30/14.
//
// All public-facing auth enums.
//

#ifndef Firebase_FAuthenticationTypes_h
#define Firebase_FAuthenticationTypes_h

typedef NS_ENUM(NSInteger, FAuthenticationError) {
    // Developer / Config Errors
    FAuthenticationErrorProviderDisabled = -1,
    FAuthenticationErrorInvalidConfiguration = -2,
    FAuthenticationErrorInvalidOrigin = -3,
    FAuthenticationErrorInvalidProvider = -4,

    // User Errors (Email / Password)
    FAuthenticationErrorInvalidEmail = -5,
    FAuthenticationErrorInvalidPassword = -6,
    FAuthenticationErrorInvalidToken = -7,
    FAuthenticationErrorUserDoesNotExist = -8,
    FAuthenticationErrorEmailTaken = -9,

    // User Errors (Facebook / Twitter / Github / Google)
    FAuthenticationErrorDeniedByUser = -10,
    FAuthenticationErrorInvalidCredentials = -11,
    FAuthenticationErrorInvalidArguments = -12,
    FAuthenticationErrorProviderError = -13,
    FAuthenticationErrorLimitsExceeded = -14,

    // Client side errors
    FAuthenticationErrorNetworkError = -15,
    FAuthenticationErrorPreempted = -16,

    FAuthenticationErrorUnknown = -9999
};

#endif
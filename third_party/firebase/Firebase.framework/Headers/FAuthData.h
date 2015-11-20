//
//  FAuthData.h
//  Firebase
//
//  Created by Katherine Fang on 7/30/14.
//

#import "FAuthType.h"


/**
* The FAuthData class is a wrapper around the user metadata returned from the Firebase auth server.
* It includes the provider authenticated against, a uid (with the possible exception of authenticating against a custom
* backend), and a token used to authenticate with Firebase.
*
* It may include other metadata about the user, depending on the provider used to do the authentication.
*/
@interface FAuthData : NSObject

/**
* @return Raw authentication token payload returned by the server
*/
@property (nonatomic, strong, readonly) NSDictionary *auth;

/**
 * @return Authentication token expiration timestamp (seconds since epoch) returned by the server
 */
@property (nonatomic, strong, readonly) NSNumber *expires;

/**
* @return A uid for this user. It is unique across all auth providers.
*/
@property (nonatomic, strong, readonly) NSString *uid;


/**
* @return The provider that authenticated this user
*/
@property (nonatomic, readonly) NSString *provider;


/**
* @return The token that was used to authenticate this user with Firebase
*/
@property (nonatomic, strong, readonly) NSString *token;


/**
* @return Provider data keyed by provider. Includes cached data from third-party providers
*/
@property (nonatomic, strong, readonly) NSDictionary *providerData;


@end

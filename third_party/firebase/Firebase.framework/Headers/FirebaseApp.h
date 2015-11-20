#import <Foundation/Foundation.h>

/**
 * All Firebase references to the same database share a connection, persistent cache, etc. FirebaseApp
 * represents this shared state and can be accessed from any reference via its `app` property.
 * It has methods for managing your Firebase connection, etc.
 *
 * There is a one-to-one relationship between a FirebaseApp instance and a connection to Firebase.
 */
@interface FirebaseApp : NSObject

/**
 * The Firebase client automatically queues writes and sends them to the server at the earliest opportunity,
 * depending on network connectivity.  In some cases (e.g. offline usage) there may be a large number of writes
 * waiting to be sent. Calling this method will purge all outstanding writes so they are abandoned.
 *
 * All writes will be purged, including transactions and {@link Firebase#onDisconnect} writes.  The writes will
 * be rolled back locally, perhaps triggering events for affected event listeners, and the client will not
 * (re-)send them to the Firebase backend.
 */
- (void)purgeOutstandingWrites;


/**
 * Shuts down our connection to the Firebase backend until goOnline is called.
 */
- (void)goOffline;

/**
 * Resumes our connection to the Firebase backend after a previous goOffline call.
 */
- (void)goOnline;

@end

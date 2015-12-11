/*
 * Firebase iOS Client Library
 *
 * Copyright © 2013 Firebase - All Rights Reserved
 * https://www.firebase.com
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binaryform must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY FIREBASE AS IS AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL FIREBASE BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#import <Foundation/Foundation.h>
#import "FEventType.h"
#import "FDataSnapshot.h"

typedef NSUInteger FirebaseHandle;

/**
 * An FQuery instance represents a query over the data at a particular location.
 * 
 * You create one by calling one of the query methods (queryStartingAtPriority:, queryEndingAtPriority:, etc.) 
 * on a Firebase reference. The query methods can be chained to further specify the data you are interested in 
 * observing
 */
@interface FQuery : NSObject


/** @name Attaching observers to read data */

/**
 * observeEventType:withBlock: is used to listen for data changes at a particular location.
 * This is the primary way to read data from Firebase. Your block will be triggered
 * for the initial data and again whenever the data changes.
 *
 * Use removeObserverWithHandle: to stop receiving updates.
 * @param eventType The type of event to listen for.
 * @param block The block that should be called with initial data and updates.  It is passed the data as an FDataSnapshot.
 * @return A handle used to unregister this block later using removeObserverWithHandle:
 */
- (FirebaseHandle) observeEventType:(FEventType)eventType withBlock:(void (^)(FDataSnapshot* snapshot))block;


/**
 * observeEventType:andPreviousSiblingKeyWithBlock: is used to listen for data changes at a particular location.
 * This is the primary way to read data from Firebase. Your block will be triggered
 * for the initial data and again whenever the data changes. In addition, for FEventTypeChildAdded, FEventTypeChildMoved, and
 * FEventTypeChildChanged events, your block will be passed the key of the previous node by priority order.
 *
 * Use removeObserverWithHandle: to stop receiving updates.
 *
 * @param eventType The type of event to listen for.
 * @param block The block that should be called with initial data and updates.  It is passed the data as an FDataSnapshot
 * and the previous child's key.
 * @return A handle used to unregister this block later using removeObserverWithHandle:
 */
- (FirebaseHandle) observeEventType:(FEventType)eventType andPreviousSiblingKeyWithBlock:(void (^)(FDataSnapshot* snapshot, NSString* prevKey))block;


/**
 * observeEventType:withBlock: is used to listen for data changes at a particular location.
 * This is the primary way to read data from Firebase. Your block will be triggered
 * for the initial data and again whenever the data changes.
 *
 * The cancelBlock will be called if you will no longer receive new events due to no longer having permission.
 *
 * Use removeObserverWithHandle: to stop receiving updates.
 *
 * @param eventType The type of event to listen for.
 * @param block The block that should be called with initial data and updates.  It is passed the data as an FDataSnapshot.
 * @param cancelBlock The block that should be called if this client no longer has permission to receive these events
 * @return A handle used to unregister this block later using removeObserverWithHandle:
 */
- (FirebaseHandle) observeEventType:(FEventType)eventType withBlock:(void (^)(FDataSnapshot* snapshot))block withCancelBlock:(void (^)(NSError* error))cancelBlock;


/**
 * observeEventType:andPreviousSiblingKeyWithBlock: is used to listen for data changes at a particular location.
 * This is the primary way to read data from Firebase. Your block will be triggered
 * for the initial data and again whenever the data changes. In addition, for FEventTypeChildAdded, FEventTypeChildMoved, and
 * FEventTypeChildChanged events, your block will be passed the key of the previous node by priority order.
 *
 * The cancelBlock will be called if you will no longer receive new events due to no longer having permission.
 *
 * Use removeObserverWithHandle: to stop receiving updates.
 *
 * @param eventType The type of event to listen for.
 * @param block The block that should be called with initial data and updates.  It is passed the data as an FDataSnapshot
 * and the previous child's key.
 * @param cancelBlock The block that should be called if this client no longer has permission to receive these events
 * @return A handle used to unregister this block later using removeObserverWithHandle:
 */
- (FirebaseHandle) observeEventType:(FEventType)eventType andPreviousSiblingKeyWithBlock:(void (^)(FDataSnapshot* snapshot, NSString* prevKey))block withCancelBlock:(void (^)(NSError* error))cancelBlock;


/**
 * This is equivalent to observeEventType:withBlock:, except the block is immediately canceled after the initial data is returned.
 *
 * @param eventType The type of event to listen for.
 * @param block The block that should be called.  It is passed the data as an FDataSnapshot.
 */
- (void) observeSingleEventOfType:(FEventType)eventType withBlock:(void (^)(FDataSnapshot* snapshot))block;


/**
 * This is equivalent to observeEventType:withBlock:, except the block is immediately canceled after the initial data is returned. In addition, for FEventTypeChildAdded, FEventTypeChildMoved, and
 * FEventTypeChildChanged events, your block will be passed the key of the previous node by priority order.
 *
 * @param eventType The type of event to listen for.
 * @param block The block that should be called.  It is passed the data as an FDataSnapshot and the previous child's key.
 */
- (void) observeSingleEventOfType:(FEventType)eventType andPreviousSiblingKeyWithBlock:(void (^)(FDataSnapshot* snapshot, NSString* prevKey))block;


/**
 * This is equivalent to observeEventType:withBlock:, except the block is immediately canceled after the initial data is returned.
 *
 * The cancelBlock will be called if you do not have permission to read data at this location.
 *
 * @param eventType The type of event to listen for.
 * @param block The block that should be called.  It is passed the data as an FDataSnapshot.
 * @param cancelBlock The block that will be called if you don't have permission to access this data
 */
- (void) observeSingleEventOfType:(FEventType)eventType withBlock:(void (^)(FDataSnapshot* snapshot))block withCancelBlock:(void (^)(NSError* error))cancelBlock;


/**
 * This is equivalent to observeEventType:withBlock:, except the block is immediately canceled after the initial data is returned. In addition, for FEventTypeChildAdded, FEventTypeChildMoved, and
 * FEventTypeChildChanged events, your block will be passed the key of the previous node by priority order.
 *
 * The cancelBlock will be called if you do not have permission to read data at this location.
 *
 * @param eventType The type of event to listen for.
 * @param block The block that should be called.  It is passed the data as an FDataSnapshot and the previous child's key.
 * @param cancelBlock The block that will be called if you don't have permission to access this data
 */
- (void) observeSingleEventOfType:(FEventType)eventType andPreviousSiblingKeyWithBlock:(void (^)(FDataSnapshot* snapshot, NSString* prevKey))block withCancelBlock:(void (^)(NSError* error))cancelBlock;

/** @name Detaching observers */

/**
 * Detach a block previously attached with observeEventType:withBlock:.
 *
 * @param handle The handle returned by the call to observeEventType:withBlock: which we are trying to remove.
 */
- (void) removeObserverWithHandle:(FirebaseHandle)handle;


/**
 * Detach all blocks previously attached to this Firebase location with observeEventType:withBlock:
 */
- (void) removeAllObservers;

/**
 * By calling `keepSynced:YES` on a location, the data for that location will automatically be downloaded and
 * kept in sync, even when no listeners are attached for that location. Additionally, while a location is kept
 * synced, it will not be evicted from the persistent disk cache.
 *
 * @param keepSynced Pass YES to keep this location synchronized, pass NO to stop synchronization.
*/
 - (void) keepSynced:(BOOL)keepSynced;


/** @name Querying and limiting */


/**
* This method is deprecated in favor of using queryStartingAtValue:. This can be used with queryOrderedByPriority
* to query by priority.
*
 * queryStartingAtPriority: is used to generate a reference to a limited view of the data at this location.
 * The FQuery instance returned by queryStartingAtPriority: will respond to events at nodes with a priority
 * greater than or equal to startPriority
 *
 * @param startPriority The lower bound, inclusive, for the priority of data visible to the returned FQuery
 * @return An FQuery instance, limited to data with priority greater than or equal to startPriority
 */
- (FQuery *) queryStartingAtPriority:(id)startPriority __attribute__((deprecated("Use [[FQuery queryOrderedByPriority] queryStartingAtValue:] instead")));


/**
* This method is deprecated in favor of using queryStartingAtValue:childKey:. This can be used with queryOrderedByPriority
* to query by priority.
*
 * queryStartingAtPriority:andChildName: is used to generate a reference to a limited view of the data at this location.
 * The FQuery instance returned by queryStartingAtPriority:andChildName will respond to events at nodes with a priority
 * greater than startPriority, or equal to startPriority and with a name greater than or equal to childName
 *
 * @param startPriority The lower bound, inclusive, for the priority of data visible to the returned FQuery
 * @param childName The lower bound, inclusive, for the name of nodes with priority equal to startPriority
 * @return An FQuery instance, limited to data with priority greater than or equal to startPriority
 */
- (FQuery *) queryStartingAtPriority:(id)startPriority andChildName:(NSString *)childName __attribute__((deprecated("Use [[FQuery queryOrderedByPriority] queryStartingAtValue:childKey:] instead")));

/**
* This method is deprecated in favor of using queryEndingAtValue:. This can be used with queryOrderedByPriority
* to query by priority.
*
 * queryEndingAtPriority: is used to generate a reference to a limited view of the data at this location.
 * The FQuery instance returned by queryEndingAtPriority: will respond to events at nodes with a priority
 * less than or equal to startPriority and with a name greater than or equal to childName
 *
 * @param endPriority The upper bound, inclusive, for the priority of data visible to the returned FQuery
 * @return An FQuery instance, limited to data with priority less than or equal to endPriority
 */
- (FQuery *) queryEndingAtPriority:(id)endPriority __attribute__((deprecated("Use [[FQuery queryOrderedByPriority] queryEndingAtValue:] instead")));


/**
* This method is deprecated in favor of using queryEndingAtValue:childKey:. This can be used with queryOrderedByPriority
* to query by priority.
*
 * queryEndingAtPriority:andChildName: is used to generate a reference to a limited view of the data at this location.
 * The FQuery instance returned by queryEndingAtPriority:andChildName will respond to events at nodes with a priority
 * less than endPriority, or equal to endPriority and with a name less than or equal to childName
 *
 * @param endPriority The upper bound, inclusive, for the priority of data visible to the returned FQuery
 * @param childName The upper bound, inclusive, for the name of nodes with priority equal to endPriority
 * @return An FQuery instance, limited to data with priority less than endPriority or equal to endPriority and with a name less than or equal to childName
 */
- (FQuery *) queryEndingAtPriority:(id)endPriority andChildName:(NSString *)childName __attribute__((deprecated("Use [[FQuery queryOrderedByPriority] queryEndingAtValue:childKey:] instead")));


/**
* This method is deprecated in favor of using queryEqualToValue:. This can be used with queryOrderedByPriority
* to query by priority.
*
* queryEqualToPriority: is used to generate a reference to a limited view of the data at this location.
* The FQuery instance returned by queryEqualToPriority: will respond to events at nodes with a priority equal to
* supplied argument.
*
* @param priority The priority that the data returned by this FQuery will have
* @return An Fquery instance, limited to data with the supplied priority.
*/
- (FQuery *) queryEqualToPriority:(id)priority __attribute__((deprecated("Use [[FQuery queryOrderedByPriority] queryEqualToValue:] instead")));


/**
* This method is deprecated in favor of using queryEqualAtValue:childKey:. This can be used with queryOrderedByPriority
* to query by priority.
*
* queryEqualToPriority:andChildName: is used to generate a reference to a limited view of the data at this location.
* The FQuery instance returned by queryEqualToPriority:andChildName will respond to events at nodes with a priority
* equal to the supplied argument with a name equal to childName. There will be at most one node that matches because
* child names are unique.
*
* @param priority The priority that the data returned by this FQuery will have
* @param childName The name of nodes with the right priority
* @return An FQuery instance, limited to data with the supplied priority and the name.
*/
- (FQuery *) queryEqualToPriority:(id)priority andChildName:(NSString *)childName __attribute__((deprecated("Use [[FQuery queryOrderedByPriority] queryEqualToValue:childKey:] instead")));

/**
 * This method is deprecated in favor of using queryLimitedToFirst:limit or queryLimitedToLast:limit instead.
 *
 * queryLimitedToNumberOfChildren: is used to generate a reference to a limited view of the data at this location.
 * The FQuery instance returned by queryLimitedToNumberOfChildren: will respond to events from at most limit child nodes.
 *
 * @param limit The upper bound, inclusive, for the number of child nodes to receive events for
 * @return An FQuery instance, limited to at most limit child nodes.
 */
- (FQuery *) queryLimitedToNumberOfChildren:(NSUInteger)limit __attribute__((deprecated("Use [FQuery queryLimitedToFirst:limit] or [FQuery queryLimitedToLast:limit] instead")));


/**
* queryLimitedToFirst: is used to generate a reference to a limited view of the data at this location.
* The FQuery instance returned by queryLimitedToFirst: will respond to at most the first limit child nodes.
*
* @param limit The upper bound, inclusive, for the number of child nodes to receive events for
* @return An FQuery instance, limited to at most limit child nodes.
*/
- (FQuery *) queryLimitedToFirst:(NSUInteger)limit;


/**
* queryLimitedToLast: is used to generate a reference to a limited view of the data at this location.
* The FQuery instance returned by queryLimitedToLast: will respond to at most the last limit child nodes.
*
* @param limit The upper bound, inclusive, for the number of child nodes to receive events for
* @return An FQuery instance, limited to at most limit child nodes.
*/
- (FQuery *) queryLimitedToLast:(NSUInteger)limit;

/**
* queryOrderBy: is used to generate a reference to a view of the data that's been sorted by the values of
* a particular child key. This method is intended to be used in combination with queryStartingAtValue:,
* queryEndingAtValue:, or queryEqualToValue:.
*
 * @param key The child key to use in ordering data visible to the returned FQuery
 * @return An FQuery instance, ordered by the values of the specified child key.
*/
- (FQuery *) queryOrderedByChild:(NSString *)key;

/**
 * queryOrderedByKey: is used to generate a reference to a view of the data that's been sorted by child key.
 * This method is intended to be used in combination with queryStartingAtValue:, queryEndingAtValue:,
 * or queryEqualToValue:.
 *
 * @return An FQuery instance, ordered by child keys.
 */
- (FQuery *) queryOrderedByKey;

/**
 * queryOrderedByValue: is used to generate a reference to a view of the data that's been sorted by child value.
 * This method is intended to be used in combination with queryStartingAtValue:, queryEndingAtValue:,
 * or queryEqualToValue:.
 *
 * @return An FQuery instance, ordered by child value.
 */
- (FQuery *) queryOrderedByValue;

/**
 * queryOrderedByPriority: is used to generate a reference to a view of the data that's been sorted by child
 * priority. This method is intended to be used in combination with queryStartingAtValue:, queryEndingAtValue:,
 * or queryEqualToValue:.
 *
 * @return An FQuery instance, ordered by child priorities.
 */
- (FQuery *) queryOrderedByPriority;

/**
 * queryStartingAtValue: is used to generate a reference to a limited view of the data at this location.
 * The FQuery instance returned by queryStartingAtValue: will respond to events at nodes with a value
 * greater than or equal to startValue.
 *
 * @param startValue The lower bound, inclusive, for the value of data visible to the returned FQuery
 * @return An FQuery instance, limited to data with value greater than or equal to startValue
 */
- (FQuery *) queryStartingAtValue:(id)startValue;

/**
 * queryStartingAtValue:childKey: is used to generate a reference to a limited view of the data at this location.
 * The FQuery instance returned by queryStartingAtValue:childKey will respond to events at nodes with a value
 * greater than startValue, or equal to startValue and with a key greater than or equal to childKey.
 *
 * @param startValue The lower bound, inclusive, for the value of data visible to the returned FQuery
 * @param childKey The lower bound, inclusive, for the key of nodes with value equal to startValue
 * @return An FQuery instance, limited to data with value greater than or equal to startValue
 */
- (FQuery *) queryStartingAtValue:(id)startValue childKey:(NSString *)childKey;

/**
 * queryEndingAtValue: is used to generate a reference to a limited view of the data at this location.
 * The FQuery instance returned by queryEndingAtValue: will respond to events at nodes with a value
 * less than or equal to endValue.
 *
 * @param endValue The upper bound, inclusive, for the value of data visible to the returned FQuery
 * @return An FQuery instance, limited to data with value less than or equal to endValue
 */
- (FQuery *) queryEndingAtValue:(id)endValue;

/**
 * queryEndingAtValue:childKey: is used to generate a reference to a limited view of the data at this location.
 * The FQuery instance returned by queryEndingAtValue:childKey will respond to events at nodes with a value
 * less than endValue, or equal to endValue and with a key less than or equal to childKey.
 *
 * @param endValue The upper bound, inclusive, for the value of data visible to the returned FQuery
 * @param childKey The upper bound, inclusive, for the key of nodes with value equal to endValue
 * @return An FQuery instance, limited to data with value less than or equal to endValue
 */
- (FQuery *) queryEndingAtValue:(id)endValue childKey:(NSString *)childKey;

/**
 * queryEqualToValue: is used to generate a reference to a limited view of the data at this location.
 * The FQuery instance returned by queryEqualToValue: will respond to events at nodes with a value equal
 * to the supplied argument.
 *
 * @param value The value that the data returned by this FQuery will have
 * @return An Fquery instance, limited to data with the supplied value.
 */
- (FQuery *) queryEqualToValue:(id)value;

/**
 * queryEqualToValue:childKey: is used to generate a reference to a limited view of the data at this location.
 * The FQuery instance returned by queryEqualToValue:childKey will respond to events at nodes with a value
 * equal to the supplied argument with a name equal to childKey. There will be at most one node that matches because
 * child keys are unique.
 *
 * @param value The value that the data returned by this FQuery will have
 * @param childKey The name of nodes with the right value
 * @return An FQuery instance, limited to data with the supplied value and the key.
 */
- (FQuery *) queryEqualToValue:(id)value childKey:(NSString *)childKey;


/** @name Properties */


/**
* Get a Firebase reference for the location of this query.
*
* @return A Firebase instance for the location of this query.
*/
@property (nonatomic, readonly, strong) Firebase* ref;

@end

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

@class Firebase;

/**
 * An FDataSnapshot contains data from a Firebase location. Any time you read 
 * Firebase data, you receive the data as an FDataSnapshot.
 *
 * FDataSnapshots are passed to the blocks you attach with observeEventType:withBlock: or observeSingleEvent:withBlock:.
 * They are efficiently-generated immutable copies of the data at a Firebase location.
 * They can't be modified and will never change. To modify data at a location,
 * use a Firebase reference (e.g. with setValue:).
 */
@interface FDataSnapshot : NSObject


/** @name Navigating and inspecting a snapshot */

/**
 * Get an FDataSnapshot for the location at the specified relative path.
 * The relative path can either be a simple child key (e.g. 'fred') 
 * or a deeper slash-separated path (e.g. 'fred/name/first'). If the child
 * location has no data, an empty FDataSnapshot is returned.
 *
 * @param childPathString A relative path to the location of child data.
 * @return The FDataSnapshot for the child location.
 */
- (FDataSnapshot *) childSnapshotForPath:(NSString *)childPathString;


/**
 * Return YES if the specified child exists.
 *
 * @param childPathString A relative path to the location of a potential child.
 * @return YES if data exists at the specified childPathString, else false.
 */
- (BOOL) hasChild:(NSString *)childPathString;


/**
 * Return YES if the DataSnapshot has any children.
 * 
 * @return YES if this snapshot has any children, else NO.
 */
- (BOOL) hasChildren;


/**
 * Return YES if the DataSnapshot contains a non-null value.
 *
 * @return YES if this snapshot contains a non-null value, otherwise NO.
 */
- (BOOL) exists;


/** @name Data export */

/**
 * Returns the raw value at this location, coupled with any metadata, such as priority.
 *
 * Priorities, where they exist, are accessible under the ".priority" key in instances of NSDictionary. 
 * For leaf locations with priorities, the value will be under the ".value" key.
 */
- (id) valueInExportFormat;


/** @name Properties */

/**
 * Returns the contents of this data snapshot as native types.
 *
 * Data types returned:
 * * NSDictionary
 * * NSArray
 * * NSNumber (also includes booleans)
 * * NSString
 *
 * @return The data as a native object.
 */
@property (strong, readonly, nonatomic) id value;


/**
 * Get the number of children for this DataSnapshot.
 *
 * @return An integer indicating the number of children.
 */
@property (readonly, nonatomic) NSUInteger childrenCount;


/**
 * Get a Firebase reference for the location that this data came from
 *
 * @return A Firebase instance for the location of this data
 */
@property (nonatomic, readonly, strong) Firebase* ref;


/**
 * The key of the location that generated this FDataSnapshot.
 *
 * @return An NSString containing the key for the location of this FDataSnapshot.
 */
@property (strong, readonly, nonatomic) NSString* key;


/**
 * An iterator for snapshots of the child nodes in this snapshot.
 * You can use the native for..in syntax:
 *
 * for (FDataSnapshot* child in snapshot.children) {
 *     ...
 * }
 *
 * @return An NSEnumerator of the children
 */
@property (strong, readonly, nonatomic) NSEnumerator* children;

/**
 * The priority of the data in this FDataSnapshot.
 *
 * @return The priority as a string, or nil if no priority was set.
 */
@property (strong, readonly, nonatomic) id priority;

@end

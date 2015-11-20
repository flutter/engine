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

/**
 * An FMutableData instance is populated with data from a Firebase location. 
 * When you are using runTransactionBlock:, you will be given an instance containing the current
 * data at that location. Your block will be responsible for updating that instance to the data
 * you wish to save at that location, and then returning using [FTransactionResult successWithValue:].
 *
 * To modify the data, set its value property to any of the native types support by Firebase:
 * * NSNumber (includes BOOL)
 * * NSDictionary
 * * NSArray
 * * NSString
 * * nil / NSNull to remove the data
 *
 * Note that changes made to a child FMutableData instance will be visible to the parent.
 */
@interface FMutableData : NSObject


/** @name Inspecting and navigating the data */


/**
 * Returns boolean indicating whether this mutable data has children.
 *
 * @return YES if this data contains child nodes.
 */
- (BOOL) hasChildren;


/**
 * Indicates whether this mutable data has a child at the given path.
 *
 * @param path A path string, consisting either of a single segment, like 'child', or multiple segments, 'a/deeper/child'
 * @return YES if this data contains a child at the specified relative path
 */
- (BOOL) hasChildAtPath:(NSString *)path;


/**
 * Used to obtain an FMutableData instance that encapsulates the data at the given relative path.
 * Note that changes made to the child will be visible to the parent.
 *
 * @param path A path string, consisting either of a single segment, like 'child', or multiple segments, 'a/deeper/child'
 * @return An FMutableData instance containing the data at the given path
 */
- (FMutableData *) childDataByAppendingPath:(NSString *)path;


/** @name Properties */


/**
 * This method is deprecated.
 *
 * @return An FMutableData instance containing the data at the parent location, or nil if this is the top-most location
 */
@property (strong, readonly, nonatomic) FMutableData* parent __attribute__((deprecated("Deprecated. Do not use.")));;


/**
 * To modify the data contained by this instance of FMutableData, set this to any of the native types support by Firebase:
 *
 * * NSNumber (includes BOOL)
 * * NSDictionary
 * * NSArray
 * * NSString
 * * nil / NSNull to remove the data
 *
 * Note that setting the value will override the priority at this location.
 *
 * @return The current data at this location as a native object
 */
@property (strong, nonatomic) id value;


/**
 * Set this property to update the priority of the data at this location. Can be set to the following types:
 *
 * * NSNumber
 * * NSString
 * * nil / NSNull to remove the priority
 *
 * @return The priority of the data at this location
 */
@property (strong, nonatomic) id priority;


/**
 * @return The number of child nodes at this location
 */
@property (readonly, nonatomic) NSUInteger childrenCount;


/**
 * Used to iterate over the children at this location. You can use the native for .. in syntax:
 *
 * for (FMutableData* child in data.children) {
 *     ...
 * }
 *
 * Note that this enumerator operates on an immutable copy of the child list. So, you can modify the instance
 * during iteration, but the new additions will not be visible until you get a new enumerator.
 */
@property (readonly, nonatomic, strong) NSEnumerator* children;


/**
 * @return The key name of this node, or nil if it is the top-most location
 */
@property (readonly, nonatomic, strong) NSString* key;


@end

// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Cocoa/Cocoa.h>

/**
 * FlutterBackingStoreData holds data to be stored in the
 * BackingStore's user_data.
 */
@interface FlutterBackingStoreData : NSObject

- (nullable instancetype)initWithIsRootView:(bool)isRootView;

- (bool)isRootView;

@end

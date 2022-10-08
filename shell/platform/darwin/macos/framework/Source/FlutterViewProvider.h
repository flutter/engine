// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterView.h"

@class FlutterEngine;

/**
 * @brief A facade over FlutterEngine that allows FlutterEngine's children
 *        components to query FlutterView. FlutterViewProvider only holds a
 *        weak reference to FlutterEngine.
 */
@interface FlutterViewProvider : NSObject

/**
 * Create a FlutterViewProvider with the underlying engine.
 */
- (nonnull instancetype)initWithEngine:(nonnull __weak FlutterEngine*)engine;

/**
 * Get the FlutterView with the given view ID.
 *
 * Return nil if the ID is not available.
 */
- (nullable FlutterView*)getView:(uint64_t)id;

@end

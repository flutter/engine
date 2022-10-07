// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterView.h"

@class FlutterEngine;

@interface FlutterViewProvider : NSObject

/**
 *
 */
- (nonnull instancetype)initWithEngine:(nonnull FlutterEngine*)engine;

/**
 *
 */
- (nullable FlutterView*)getView:(nonnull NSNumber*)id;

@end

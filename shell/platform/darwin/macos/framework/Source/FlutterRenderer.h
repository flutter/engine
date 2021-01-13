// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Cocoa/Cocoa.h>

#import "flutter/shell/platform/darwin/macos/framework/Headers/FlutterEngine.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterView.h"
#import "flutter/shell/platform/embedder/embedder.h"

@protocol FlutterRenderer <FlutterTextureRegistry>

/**
 * Intializes the renderer with the given FlutterEngine.
 */
- (nullable instancetype)initWithFlutterEngine:(nonnull FlutterEngine*)flutterEngine;

/**
 * Sets the FlutterView to render to.
 */
- (void)setFlutterView:(nullable FlutterView*)view;

/**
 * Creates a FlutterRendererConfig that renders using Metal.
 */
- (FlutterRendererConfig)createRendererConfig;

@end

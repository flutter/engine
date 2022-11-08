// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Cocoa/Cocoa.h>

#import "flutter/shell/platform/darwin/macos/framework/Headers/FlutterEngine.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterTextureRegistrar.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterView.h"
#import "flutter/shell/platform/embedder/embedder.h"

/**
 * Rendering backend agnostic FlutterRendererConfig provider to be used by the embedder API.
 */
@protocol FlutterRenderer <FlutterTextureRegistry, FlutterTextureRegistrarDelegate>

/**
 * Creates a FlutterRendererConfig that renders using the appropriate backend.
 */
- (FlutterRendererConfig)createRendererConfig;

/**
 * Called by the engine when the given view's buffers should be swapped.
 */
- (BOOL)present:(nonnull FlutterView*)view;

/**
 * Tells the renderer that there is no Flutter content available for this frame.
 */
- (void)presentWithoutContent:(nonnull FlutterView*)view;

@end

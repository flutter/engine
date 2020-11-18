// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>

#import "flutter/shell/platform/darwin/macos/framework/Headers/FlutterEngine.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterView.h"
#import "flutter/shell/platform/embedder/embedder.h"

NS_ASSUME_NONNULL_BEGIN

/**
 * Provides the renderer config needed to initialize the embedder engine and also handles external
 * texture management. This is initialized during FlutterEngine creation and then attached to the
 * FlutterView once the FlutterViewController is initializer.
 */
@interface FlutterMetalRenderer : NSObject <FlutterTextureRegistry>

@property(nonatomic, readonly) id<MTLDevice> mtlDevice;

@property(nonatomic, readonly) id<MTLCommandQueue> mtlCommandQueue;

/**
 * Intializes the renderer with the given FlutterEngine.
 */
- (instancetype)initWithFlutterEngine:(FLUTTER_API_SYMBOL(FlutterEngine))engine;

/**
 * Attaches to the FlutterView and sets up the renderers main command queue.
 */
- (void)attachToFlutterView:(FlutterView*)view;

/**
 * Creates a FlutterRendererConfig that renders using Metal.
 */
- (FlutterRendererConfig)createRendererConfig;

- (void)populateTextureForSize:(CGSize)size to:(FlutterMetalTexture*)output;

- (bool)present:(int64_t)textureId;

@end

NS_ASSUME_NONNULL_END

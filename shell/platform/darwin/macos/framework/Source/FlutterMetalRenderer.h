// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>

#import "flutter/shell/platform/darwin/macos/framework/Headers/FlutterEngine.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterView.h"
#import "flutter/shell/platform/embedder/embedder.h"

/**
 * Provides the renderer config needed to initialize the embedder engine. This is initialized during
 * FlutterEngine creation and then attached to the FlutterView once the FlutterViewController is
 * initialized.
 */
@interface FlutterMetalRenderer : NSObject <FlutterTextureRegistry>

/**
 * Interface to the system GPU. Used to issue all the rendering commands.
 */
@property(nonatomic, readonly, nonnull) id<MTLDevice> device;

/**
 * Used to get the command buffers for the MTLDevice to render to.
 */
@property(nonatomic, readonly, nonnull) id<MTLCommandQueue> commandQueue;

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

/**
 * Creates a Metal texture for the given size.
 */
- (FlutterMetalTexture)createTextureForSize:(CGSize)size;

/**
 * Presents the texture specefied by the texture id.
 */
- (BOOL)present:(int64_t)textureId;

@end

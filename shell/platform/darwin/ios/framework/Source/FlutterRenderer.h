// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <UIKit/UIKit.h>

#import "flutter/shell/platform/darwin/common/framework/Headers/FlutterTexture.h"
#import "flutter/shell/platform/darwin/ios/framework/Headers/FlutterEngine.h"
#import "flutter/shell/platform/darwin/ios/framework/Source/FlutterView.h"
#import "flutter/shell/platform/embedder/embedder.h"

NS_ASSUME_NONNULL_BEGIN

/**
 * A set of texture registration methods that works with Embedder API.
 *
 * Because the texture registration methods in the Embedder API returns bool.
 * This set of protocol methods work better with Embedder API than <FlutterTextureRegistry>.
 */
@protocol FlutterRendererTextureRegistryDelegate

- (BOOL)registerTextureWithID:(int64_t)textureId;
- (BOOL)markTextureFrameAvailable:(int64_t)textureID;
- (BOOL)unregisterTextureWithID:(int64_t)textureID;

@end

/**
 * Rendering backend agnostic FlutterRendererConfig provider to be used by the embedder API.
 *
 * Only supports single UIView.
 */
@interface FlutterRenderer : NSObject <FlutterTextureRegistry>

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
- (nullable instancetype)initWithTextureDelegate:
    (NSObject<FlutterRendererTextureRegistryDelegate>*)textureDelegate;

/**
 * Creates a FlutterRendererConfig that renders using the appropriate backend.
 */
- (FlutterRendererConfig)createRendererConfig;

/**
 * Called by the engine when the given view's buffers should be swapped.
 */
- (BOOL)present:(uint64_t)viewId texture:(nonnull const FlutterMetalTexture*)texture;

/**
 * Creates a Metal texture for the given view with the given size.
 */
- (FlutterMetalTexture)createTextureForView:(uint64_t)viewId size:(CGSize)size;

/**
 * Populates the texture registry with the provided metalTexture.
 */
- (BOOL)populateTextureWithIdentifier:(int64_t)textureID
                         metalTexture:(nonnull FlutterMetalExternalTexture*)metalTexture;

/**
 * Sets the layer to be rendered on.
 */
- (void)setLayer:(CALayer*)layer;

@end

NS_ASSUME_NONNULL_END
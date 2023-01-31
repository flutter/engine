// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

@class FlutterEmbedderAPIBridge;

#include <Metal/Metal.h>

#import "flutter/shell/platform/darwin/embedder/FlutterTextureRegistrar.h"
#import "flutter/shell/platform/embedder/embedder.h"

NS_ASSUME_NONNULL_BEGIN

@protocol FlutterPresenter

- (BOOL)present:(uint64_t)viewId texture:(nonnull const FlutterMetalTexture*)texture;

- (FlutterMetalTexture)createTextureForView:(uint64_t)viewId size:(CGSize)size;

@end

/**
 * Rendering backend agnostic FlutterRendererConfig provider to be used by the embedder API.
 */
@interface FlutterRenderer : FlutterTextureRegistrar <FlutterTextureRegistrarDelegate>

/**
 * Interface to the system GPU. Used to issue all the rendering commands.
 */
@property(nonatomic, readonly, nonnull) id<MTLDevice> device;

/**
 * Used to get the command buffers for the MTLDevice to render to.
 */
@property(nonatomic, readonly, nonnull) id<MTLCommandQueue> commandQueue;

@property(strong, nonatomic, nonnull, readonly) NSObject<FlutterPresenter>* presenter;

/**
 * Intializes the renderer with the given FlutterEmbedderAPIBridge.
 */
- (nullable instancetype)initWithEmbedderAPIBridge:(FlutterEmbedderAPIBridge*)embedderAPIBridge
                                         presenter:(NSObject<FlutterPresenter>*)presenter;

/**
 * Creates a FlutterRendererConfig that renders using the appropriate backend.
 */
- (FlutterRendererConfig)createRendererConfig;

/**
 * Populates the texture registry with the provided metalTexture.
 */
- (BOOL)populateTextureWithIdentifier:(int64_t)textureID
                         metalTexture:(nonnull FlutterMetalExternalTexture*)metalTexture;

@end

NS_ASSUME_NONNULL_END
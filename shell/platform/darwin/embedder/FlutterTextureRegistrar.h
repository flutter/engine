// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

@class FlutterEmbedderAPIBridge;
#import "flutter/shell/platform/darwin/embedder/FlutterExternalTexture.h"

NS_ASSUME_NONNULL_BEGIN

// #import <Cocoa/Cocoa.h>

/*
 * Delegate methods for FlutterTextureRegistrar.
 */
@protocol FlutterTextureRegistrarDelegate

/*
 * Called by the FlutterTextureRegistrar when a texture is registered.
 */
- (nonnull FlutterExternalTexture*)onRegisterTexture:(nonnull id<FlutterTexture>)texture;

@end

/*
 * Holds the external textures and implements the FlutterTextureRegistry.
 */
@interface FlutterTextureRegistrar : NSObject <FlutterTextureRegistry>

/*
 * Use `initWithDelegate:embedderAPIBridge:` instead.
 */
- (nullable instancetype)init NS_UNAVAILABLE;

/*
 * Use `initWithDelegate:embedderAPIBridge:` instead.
 */
+ (nullable instancetype)new NS_UNAVAILABLE;

/*
 * Initialzes the texture registrar.
 */
- (nullable instancetype)initWithDelegate:(nonnull id<FlutterTextureRegistrarDelegate>)delegate
                        embedderAPIBridge:(nonnull FlutterEmbedderAPIBridge*)bridge
    NS_DESIGNATED_INITIALIZER;

/*
 * Returns the registered texture with the provided `textureID`.
 */
- (nullable FlutterExternalTexture*)getTextureWithID:(int64_t)textureID;

@end

NS_ASSUME_NONNULL_END
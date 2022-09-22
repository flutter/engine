// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/common/framework/Headers/FlutterTexture.h"
#import "flutter/shell/platform/darwin/graphics/FlutterExternalTexture.h"

/*
 * Delegate methods for FlutterTextureRegistrar.
 */
@protocol FlutterTextureRegistrarDelegate

/*
 * Called by the FlutterTextureRegistrar when a texture is registered.
 */
- (id<FlutterExternalTexture>)onRegisterTexture:(id<FlutterTexture>)texture;

@end

/*
 * Delegate methods for FlutterTextureRegistrar for EmbedderAPI.
 */
@protocol FlutterTextureRegistrarEmbedderAPIDelegate

/**
 * Registers an external texture with the given id. Returns YES on success.
 */
- (BOOL)registerTextureWithID:(int64_t)textureId;

/**
 * Marks texture with the given id as available. Returns YES on success.
 */
- (BOOL)markTextureFrameAvailable:(int64_t)textureID;

/**
 * Unregisters an external texture with the given id. Returns YES on success.
 */
- (BOOL)unregisterTextureWithID:(int64_t)textureID;

@end

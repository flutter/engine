
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <CoreVideo/CoreVideo.h>
#import <Foundation/Foundation.h>

/**
 * Implement a texture object for the FLE plugin side.
 */
@protocol FLETexture <NSObject>

/**
 * When the texture on the platform side is ready,
 * the flutter engine will copy the texture buffer
 * using copyPixelBuffer.
 */
- (nullable CVPixelBufferRef)copyPixelBuffer:(size_t)width height:(size_t)height;

@end

/**
 * The protocol for an object managing registration for texture.
 */
@protocol FLETextureRegistrar <NSObject>

/**
 * Register a |texture| object and return a textureID.
 */
- (int64_t)registerTexture:(nonnull id<FLETexture>)texture;

/**
 * Mark a texture buffer is ready.
 */
- (void)textureFrameAvailable:(int64_t)textureID;

/**
 * Unregister an existing Texture object.
 */
- (void)unregisterTexture:(int64_t)textureID;

@end

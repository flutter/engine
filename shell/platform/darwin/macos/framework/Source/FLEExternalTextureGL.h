// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Foundation/Foundation.h>

#import "flutter/shell/platform/darwin/macos/framework/Headers/FLETexture.h"
#import "flutter/shell/platform/embedder/embedder.h"

/**
 * Used to bridge external FLETexture objects,
 * so the implementer only needs to return the CVPixelBufferRef object,
 * which will make the interface consistent with the FlutterTexture.
 */
@interface FLEExternalTextureGL : NSObject

/**
 * Initializes a texture adapter with |fleTexture|.
 */
- (nonnull instancetype)initWithFLETexture:(nonnull id<FLETexture>)fleTexture;

/**
 * Accept texture rendering notifications from the flutter engine.
 */
- (BOOL)populateTextureWidth:(size_t)width
                      height:(size_t)height
                     texture:(nonnull FlutterOpenGLTexture*)texture;

/**
 * The texture id.
 */
- (int64_t)textureId;

@end

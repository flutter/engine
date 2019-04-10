// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Foundation/Foundation.h>

#import "flutter/shell/platform/darwin/macos/framework/Headers/FLETexture.h"
#import "flutter/shell/platform/embedder/embedder.h"

/**
 * Used to bridge FLETexture object and handle the texture copy request.
 */
@interface FLEExternalTextureGL : NSObject

/**
 * Initializes a texture adapter with |texture| return a FLEExternalTextureGL.
 */
- (nonnull instancetype)initWithFLETexture:(nonnull id<FLETexture>)texture;

/**
 * Accept texture buffer copy request from the flutter engine.
 */
- (BOOL)populateTextureWithWidth:(size_t)width
                          height:(size_t)height
                         texture:(nonnull FlutterOpenGLTexture*)texture;

/**
 * The texture ID.
 */
- (int64_t)textureID;

@end

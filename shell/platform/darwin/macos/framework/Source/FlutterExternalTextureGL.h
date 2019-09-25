// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Foundation/Foundation.h>

#import "flutter/shell/platform/darwin/common/framework/Headers/FlutterTexture.h"
#import "flutter/shell/platform/embedder/embedder.h"

/**
 * Used to bridge FlutterTexture object and handle the texture copy request the
 * flutter engine.
 */
@interface FlutterExternalTextureGL : NSObject

/**
 * Initializes a texture adapter with |texture| return a instance.
 */
- (nonnull instancetype)initWithFlutterTexture:(nonnull id<FlutterTexture>)texture;

/**
 * Accepts texture buffer copy request from the flutter engine.
 * When the user side marks the textureId as available, the flutter engine will
 * callback to this method and ask for populate the |openGLTexture| object,
 * such as the texture type and the format of the pixel buffer and the texture object.
 */
- (BOOL)populateTextureWithWidth:(size_t)width
                          height:(size_t)height
                   openGLTexture:(nonnull FlutterOpenGLTexture*)openGLTexture;

/**
 * Returns the ID for the FlutterTexture instance.
 */
- (int64_t)textureID;

@end

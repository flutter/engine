// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/embedder/FlutterTextureRegistrarDelegate.h"
#import "flutter/shell/platform/darwin/graphics/FlutterExternalTexture.h"

NS_ASSUME_NONNULL_BEGIN

/*
 * Holds the external textures and implements the FlutterTextureRegistry.
 */
@interface FlutterTextureRegistrar : NSObject <FlutterTextureRegistry>

/*
 * Use `initWithDelegate:` instead.
 */
+ (nullable instancetype)new NS_UNAVAILABLE;

/*
 * Initialzes the texture registrar.
 */
- (nullable instancetype)initWithDelegate:(id<FlutterTextureRegistrarDelegate>)delegate;

- (void)setEmbedderAPIDelegate:(id<FlutterTextureRegistrarEmbedderAPIDelegate>)embedderAPIDelegate;
/*
 * Returns the registered texture with the provided `textureID`.
 */
- (nullable id<FlutterExternalTexture>)getTextureWithID:(int64_t)textureID;

@end

NS_ASSUME_NONNULL_END

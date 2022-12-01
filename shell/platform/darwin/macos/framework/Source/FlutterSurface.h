// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Cocoa/Cocoa.h>

#import "flutter/shell/platform/embedder/embedder.h"

/**
 * Opaque surface type.
 * Can be represented as FlutterMetalTexture to cross the embedder API boundary.
 */
@interface FlutterSurface : NSObject

- (FlutterMetalTexture)asFlutterMetalTexture;

@end

/**
 * Internal FlutterSurface interface used by FlutterSurfaceManager.
 * Wraps an IOSurface framebuffer and metadata related to the surface.
 */
@interface FlutterSurface (Private)

- (instancetype)initWithSize:(CGSize)size device:(id<MTLDevice>)device;

@property(readonly, nonatomic) IOSurfaceRef ioSurface;
@property(readonly, nonatomic) CGSize size;
@property(readonly, nonatomic) int64_t textureId;

@end

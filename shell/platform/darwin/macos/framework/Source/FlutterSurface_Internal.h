// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterSurface.h"

/**
 * Internal FlutterSurface interface used by FlutterSurfaceManager.
 */
@interface FlutterSurface ()

- (instancetype)initWithSize:(CGSize)size device:(id<MTLDevice>)device;

@property(readonly, nonatomic) IOSurfaceRef ioSurface;
@property(readonly, nonatomic) CGSize size;
@property(readonly, nonatomic) int64_t textureId;

@end

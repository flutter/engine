// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Cocoa/Cocoa.h>
#import <IOSurface/IOSurface.h>

@interface FlutterMetalTextureProvider : NSObject

- (nullable instancetype)initWithMTLDevice:(nonnull const id<MTLDevice>)device;

/**
 * Creates an MTLTexture and returns it, transferring ownership to the caller.
 */
- (nullable id<MTLTexture>)createTextureWithSize:(CGSize)size iosurface:(nonnull IOSurfaceRef) ioSurface;

@end

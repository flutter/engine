// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterSurfaceManager.h"

#import <QuartzCore/QuartzCore.h>

/**
 * Responsible for caching back buffers to prevent unnecessary IOSurface allocations.
 */
@interface FlutterBackBufferCache : NSObject

/**
 * Removes surface with given size from cache (if available) and returns it.
 */
- (nullable FlutterSurface*)removeSurfaceForSize:(CGSize)size;

/**
 * Removes all cached surface replacing them with new ones.
 */
- (void)replaceWith:(nonnull NSArray<FlutterSurface*>*)surfaces;

/**
 * Returns number of surfaces currently in cache. Used for tests.
 */
- (NSUInteger)count;

@end

/**
 * Interface to internal properties used for testing.
 */
@interface FlutterSurfaceManager ()

@property(readonly, nonatomic, nonnull) FlutterBackBufferCache* backBufferCache;
@property(readonly, nonatomic, nonnull) NSArray<FlutterSurface*>* borrowedSurfaces;
@property(readonly, nonatomic, nonnull) NSArray<FlutterSurface*>* frontSurfaces;
@property(readonly, nonatomic, nonnull) NSArray<CALayer*>* layers;

@end

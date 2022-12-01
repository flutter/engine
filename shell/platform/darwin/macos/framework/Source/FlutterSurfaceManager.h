// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Cocoa/Cocoa.h>
#import <QuartzCore/QuartzCore.h>

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterSurface.h"

/**
 * Surface with additional properties needed for presenting.
 */
@interface FlutterSurfacePresentInfo : NSObject

@property(readwrite, strong, nonatomic, nonnull) FlutterSurface* surface;
@property(readwrite, nonatomic) CGPoint offset;
@property(readwrite, nonatomic) size_t zIndex;

@end

@protocol FlutterSurfaceManagerDelegate <NSObject>

/*
 * Schedules the block on platform thread and waits until the block is executed.
 * Provided `frameSize` is used to unblock platform thread if it waits for
 * certain frame size during resizing.
 */
- (void)onPresent:(CGSize)frameSize withBlock:(nonnull dispatch_block_t)block;

@end

/**
 * Owned by `FlutterView`.
 * Responsible for providing surfaces for Flutter to render into and subsequent
 * presentation of these surfaces. Manages core animation sublayers.
 */
@interface FlutterSurfaceManager : NSObject

/**
 * Initializes and returns a surface manager that renders to a child layer (referred to as the
 * content layer) of the containing layer.
 */
- (nullable instancetype)initWithDevice:(nonnull id<MTLDevice>)device
                           commandQueue:(nonnull id<MTLCommandQueue>)commandQueue
                                  layer:(nonnull CALayer*)containingLayer
                               delegate:(nonnull id<FlutterSurfaceManagerDelegate>)delegate;

/**
 * Returns back buffer surface for given size that Flutter can render content to.
 * Will return cached surface if one is available, or create new one otherwise.
 *
 * Must be called on raster thread.
 */
- (nonnull FlutterSurface*)surfaceForSize:(CGSize)size;

/**
 * Looks up surface for given metal texture. Can only be called for surfaces
 * obtained through `surfaceForSize:` until `present:` is called.
 */
- (nullable FlutterSurface*)lookupSurface:(nonnull const FlutterMetalTexture*)texture;

/**
 * Sets the provided surfaces as contents of FlutterView. Will create, update and
 * remove sublayers as needed.
 *
 * Must be called on raster thread. This will schedule a commit on platform
 * thread and block raster thread until the commit is done.
 * `notify` block will be invoked on platform thread and can be used to perform
 * additional work, such as mutating platform views. It is guaranteed be called in
 * same CATransaction.
 */
- (void)present:(nonnull NSArray<FlutterSurfacePresentInfo*>*)surfaces
         notify:(nullable dispatch_block_t)notify;

@end

/**
 * Cache of back buffers to prevent unnecessary IOSurface allocations.
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
@interface FlutterSurfaceManager (Private)

@property(readonly, nonatomic, nonnull) FlutterBackBufferCache* backBufferCache;
@property(readonly, nonatomic, nonnull) NSArray<FlutterSurface*>* borrowedSurfaces;
@property(readonly, nonatomic, nonnull) NSArray<FlutterSurface*>* frontSurfaces;
@property(readonly, nonatomic, nonnull) NSArray<CALayer*>* layers;

@end

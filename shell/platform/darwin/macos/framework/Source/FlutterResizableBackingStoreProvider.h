// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterBackingStoreDescriptor.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterResizeSynchronizer.h"

/**
 * Represents a buffer that can be resized.
 */
@protocol FlutterResizableBackingStoreProvider <FlutterResizeSynchronizerDelegate>

/**
 * Notify of the required backing store size updates. Called during window resize.
 */
- (void)updateBackingStoreIfNecessaryForSize:(CGSize)size;

/**
 * Returns the FlutterBackingStoreDescriptor corresponding to the latest size.
 */
- (nonnull FlutterBackingStoreDescriptor*)backingStoreDescriptor;

@end

/**
 * OpenGL backed FlutterResizableBackingStoreProvider. Backing store in this context implies a frame
 * buffer.
 */
@interface FlutterOpenGLResizableBackingStoreProvider
    : NSObject <FlutterResizableBackingStoreProvider>

/**
 * Creates a resizable backing store provider for the given CALayer.
 */
- (nullable instancetype)initWithMainContext:(nonnull NSOpenGLContext*)mainContext
                                     caLayer:(nonnull CALayer*)layer;

@end

/**
 * Metal backed FlutterResizableBackingStoreProvider. Backing store in this context implies a
 * MTLTexture.
 */
@interface FlutterMetalResizableBackingStoreProvider
    : NSObject <FlutterResizableBackingStoreProvider>

/**
 * Creates a resizable backing store provider for the given CAMetalLayer.
 */
- (nullable instancetype)initWithDevice:(nonnull id<MTLDevice>)mtlDevice
                           commandQueue:(nonnull id<MTLCommandQueue>)mtlCommandQueue
                             metalLayer:(nonnull CAMetalLayer*)layer;

@end

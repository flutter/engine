// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Cocoa/Cocoa.h>
#include <stdint.h>

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterResizableBackingStoreProvider.h"

/**
 * The view ID for APIs that don't support multi-view.
 *
 * Some single-view APIs will eventually be replaced by their multi-view
 * variant. During the deprecation period, the single-view APIs will coexist with
 * and work with the multi-view APIs as if the other views don't exist.  For
 * backward compatibility, single-view APIs will always operate the view with
 * this ID. Also, the first view assigned to the engine will also have this ID.
 */
constexpr uint64_t kFlutterDefaultViewId = 0;

/**
 * Listener for view resizing.
 */
@protocol FlutterViewReshapeListener <NSObject>
/**
 * Called when the view's backing store changes size.
 */
- (void)viewDidReshape:(nonnull NSView*)view;
@end

/**
 * View capable of acting as a rendering target and input source for the Flutter
 * engine.
 */
@interface FlutterView : NSView

/**
 * Initialize a FlutterView that will be rendered to using Metal rendering apis.
 */
- (nullable instancetype)initWithMTLDevice:(nonnull id<MTLDevice>)device
                              commandQueue:(nonnull id<MTLCommandQueue>)commandQueue
                           reshapeListener:(nonnull id<FlutterViewReshapeListener>)reshapeListener
    NS_DESIGNATED_INITIALIZER;

- (nullable instancetype)initWithFrame:(NSRect)frameRect
                           pixelFormat:(nullable NSOpenGLPixelFormat*)format NS_UNAVAILABLE;
- (nonnull instancetype)initWithFrame:(NSRect)frameRect NS_UNAVAILABLE;
- (nullable instancetype)initWithCoder:(nonnull NSCoder*)coder NS_UNAVAILABLE;
- (nonnull instancetype)init NS_UNAVAILABLE;

/**
 * Flushes the graphics context and flips the surfaces. Expected to be called on raster thread.
 */
- (void)present;

/**
 * Called when there is no Flutter content available to render. This must be passed to resize
 * synchronizer.
 */
- (void)presentWithoutContent;

/**
 * Ensures that a backing store with requested size exists and returns the descriptor. Expected to
 * be called on raster thread.
 */
- (nonnull FlutterRenderBackingStore*)backingStoreForSize:(CGSize)size;

/**
 * Must be called when shutting down. Unblocks raster thread and prevents any further
 * synchronization.
 */
- (void)shutdown;

/**
 * By default, the `FlutterSurfaceManager` creates two layers to manage Flutter
 * content, the content layer and containing layer. To set the native background
 * color, onto which the Flutter content is drawn, call this method with the
 * NSColor which you would like to override the default, black background color
 * with.
 */
- (void)setBackgroundColor:(nonnull NSColor*)color;

@end

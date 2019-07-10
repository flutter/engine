// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Headers/FLEEngine.h"

#import <Cocoa/Cocoa.h>

#import "flutter/shell/platform/embedder/embedder.h"

@interface FLEEngine ()

/**
 * True if the engine is currently running.
 */
@property(nonatomic, readonly) BOOL running;

/**
 * The resource context used by the engine for texture uploads. FlutterViews associated with this
 * engine should be created to share with this context.
 */
@property(nonatomic, readonly, nullable) NSOpenGLContext* resourceContext;

/**
 * Informs the engine that the display region's size has changed.
 *
 * @param size The size of the display, in pixels.
 * @param pixelRatio The number of pixels per screen coordinate.
 */
- (void)updateWindowMetricsWithSize:(CGSize)size pixelRatio:(double)pixelRatio;

/**
 * Dispatches the given pointer event data to engine.
 */
- (void)sendPointerEvent:(const FlutterPointerEvent&)event;

@end

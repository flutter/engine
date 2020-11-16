// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Headers/FlutterEngine.h"

#import <Cocoa/Cocoa.h>

#include "flutter/shell/platform/embedder/embedder.h"

@interface FlutterEngine ()

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
 * Function pointers for interacting with the embedder.h API.
 */
@property(nonatomic) FlutterEngineProcTable& embedderAPI;

/**
 * Informs the engine that the associated view controller's view size has changed.
 */
- (void)updateWindowMetrics;

/**
 * Dispatches the given pointer event data to engine.
 */
- (void)sendPointerEvent:(const FlutterPointerEvent&)event;

// Accessibility API.

/**
 * To enable of disable the semantics in the Flutter framework. The Flutter framework starts sending semantics update through the embedder as soon
 * as it is enabled. 
 */
- (void)updateSemanticsEnabled:(BOOL)enabled;

/**
 * To enable of disable the semantics in the Flutter framework. The Flutter framework starts sending semantics update through the embedder as soon
 * as it is enabled. 
 */
- (void)dispatchSemanticsAction:(uint16_t)target
                         action:(FlutterSemanticsAction)action
                           data:(const uint8_t*)data
                       dataSize:(size_t)dataSize;

@end

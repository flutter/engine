// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Headers/FlutterEngine.h"

#import <Cocoa/Cocoa.h>

#include <memory>

#import "flutter/shell/platform/darwin/macos/framework/Source/AccessibilityBridgeMac.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterCompositor.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterPlatformViewController.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterRenderer.h"

@interface FlutterEngine ()

/**
 * True if the engine is currently running.
 */
@property(nonatomic, readonly) BOOL running;

/**
 * Provides the renderer config needed to initialize the engine and also handles external
 * texture management.
 */
@property(nonatomic, readonly, nullable) FlutterRenderer* renderer;

/**
 * Function pointers for interacting with the embedder.h API.
 */
@property(nonatomic) FlutterEngineProcTable& embedderAPI;

/**
 * True if the semantics is enabled. The Flutter framework starts sending
 * semantics update through the embedder as soon as it is set to YES.
 */
@property(nonatomic) BOOL semanticsEnabled;

/**
 * The executable name for the current process.
 */
@property(nonatomic, readonly, nonnull) NSString* executableName;

/**
 * This just returns the NSPasteboard so that it can be mocked in the tests.
 */
@property(nonatomic, readonly, nonnull) NSPasteboard* pasteboard;

/**
 * Attach a view controller to the engine and associate it with a newly
 * generated ID.
 *
 * The engine holds a weak reference to each attached view controller.
 *
 * The first added view controller (either with this method or the
 * viewController property) will always have ID kFlutterDefaultViewId.
 *
 * If the given view controller is already attached to an engine, this call
 * throws an assertion.
 *
 * TODO(dkwingsmt): Move this method to the public API once stable. This method
 * should be a public method but its behavior is "broken" before we implement
 * the proper embedder API. See its implementation for detail.
 */
- (void)addViewController:(nonnull FlutterViewController*)viewController;

/**
 * Dissociate the given view controller from this engine.
 *
 * If the view controller is not associated with this engine, this call throws an
 * assertion.
 *
 * TODO(dkwingsmt): Move this method to the public API once stable. This method
 * should be a public method but its behavior is "broken" before we implement
 * the proper embedder API. See its implementation for detail.
 */
- (void)removeViewController:(nonnull FlutterViewController*)viewController;

/**
 * Informs the engine that the specified view controller's view size has changed.
 */
- (void)updateWindowMetricsForViewController:(nonnull FlutterViewController*)viewController;

/**
 * Dispatches the given pointer event data to engine.
 */
- (void)sendPointerEvent:(const FlutterPointerEvent&)event;

/**
 * Dispatches the given pointer event data to engine.
 */
- (void)sendKeyEvent:(const FlutterKeyEvent&)event
            callback:(nullable FlutterKeyEventCallback)callback
            userData:(nullable void*)userData;

/**
 * Registers an external texture with the given id. Returns YES on success.
 */
- (BOOL)registerTextureWithID:(int64_t)textureId;

/**
 * Marks texture with the given id as available. Returns YES on success.
 */
- (BOOL)markTextureFrameAvailable:(int64_t)textureID;

/**
 * Unregisters an external texture with the given id. Returns YES on success.
 */
- (BOOL)unregisterTextureWithID:(int64_t)textureID;

- (nonnull FlutterPlatformViewController*)platformViewController;

// Accessibility API.

/**
 * Dispatches semantics action back to the framework. The semantics must be enabled by calling
 * the updateSemanticsEnabled before dispatching semantics actions.
 */
- (void)dispatchSemanticsAction:(FlutterSemanticsAction)action
                       toTarget:(uint16_t)target
                       withData:(fml::MallocMapping)data;

@end

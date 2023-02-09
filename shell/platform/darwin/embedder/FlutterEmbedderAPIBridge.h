// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#import "flutter/shell/platform/darwin/common/framework/Headers/FlutterBaseDartProject.h"
#import "flutter/shell/platform/darwin/common/framework/Headers/FlutterBinaryMessenger.h"
#import "flutter/shell/platform/darwin/common/framework/Headers/FlutterTexture.h"
#import "flutter/shell/platform/darwin/embedder/FlutterRenderer.h"

#include "flutter/shell/platform/embedder/embedder.h"

NS_ASSUME_NONNULL_BEGIN

@protocol FlutterEmbedderAPIBridgeDelegate

// FlutterCompositors
- (bool)compositorCreatingBackingStore:(const FlutterBackingStoreConfig*)config
                       backingStoreOut:(FlutterBackingStore*)backingStoreOut;
- (bool)compositorPresent:(uint64_t)view_id
                   layers:(const FlutterLayer* _Nullable* _Nullable)layers
              layersCount:(size_t)layersCount;
- (void)updateSemantics:(const FlutterSemanticsUpdate*)update;
- (void)engineCallbackOnPreEngineRestart;

@end

/**
 * The view ID for APIs that don't support multi-view.
 *
 * Some single-view APIs will eventually be replaced by their multi-view
 * variant. During the deprecation period, the single-view APIs will coexist with
 * and work with the multi-view APIs as if the other views don't exist.  For
 * backward compatibility, single-view APIs will always operate the view with
 * this ID. Also, the first view assigned to the engine will also have this ID.
 */
extern const uint64_t kFlutterDefaultViewId;

// TODO(cyanglaz embedder api).
@interface FlutterEmbedderAPIBridge : NSObject <FlutterTextureRegistry, FlutterBinaryMessenger>

/**
 * Provides the renderer config needed to initialize the engine and also handles external
 * texture management.
 */
@property(nonatomic, readonly, nullable) FlutterRenderer* renderer;

@property(nonatomic, readonly, nullable) FlutterBaseDartProject* project;

@property(nonatomic, weak) NSObject<FlutterEmbedderAPIBridgeDelegate>* delegate;

// (TODO)cyanglaz: embedder api, temporary, remove after `engine` is removed from `FlutterEngine`.
@property(nonatomic, readonly) FLUTTER_API_SYMBOL(FlutterEngine) engine;

/**
//  * Provides the renderer config needed to initialize the engine and also handles external
//  * texture management.
//  */
// @property(nonatomic, readonly, nullable) FlutterTextureRegistrar<FlutterRenderer>* renderer;

/**
 * Function pointers for interacting with the embedder.h API.
 */
@property(nonatomic) FlutterEngineProcTable& embedderAPI;

/*
 * Use `initWithRenderer:` instead.
 */
- (nullable instancetype)init NS_UNAVAILABLE;

/*
 * Use `initWithRenderer:` instead.
 */
+ (nullable instancetype)new NS_UNAVAILABLE;

- (instancetype)initWithProject:(nullable FlutterBaseDartProject*)project
                      presenter:(NSObject<FlutterPresenter>*)presenter;

// /**
//  * The default init method.
//  */
// - (instancetype)initWithRenderer:(FlutterTextureRegistrar<FlutterRenderer>*)flutterRenderer;

- (FlutterEngineResult)initializeEmbedderAPIAndRun:(NSString*)entrypoint;

/**
 * Shut down the embedderAPIBridge. The embedderAPIBridge instance must always be shut down
 * before it may be collected. Not shutting down the embedderAPIBridge instance before releasing it
 will
 * result in the leak of that embedderAPIBridge instance.
 */
- (void)shutDownBridge;

- (BOOL)running;

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

// Embedder API callbacks
- (void)engineCallbackOnPlatformMessage:(const FlutterPlatformMessage*)message;

- (void)updateSemantics:(const FlutterSemanticsUpdate*)update;

- (void)engineCallbackOnPreEngineRestart;

- (FlutterCompositor*)createFlutterCompositor;

- (void)sendPointerEvent:(const FlutterPointerEvent&)event;

- (void)updateSemanticsEnabled:(BOOL)enable;

- (void)dispatchSemanticsAction:(FlutterSemanticsAction)action
                       toTarget:(uint16_t)target
                       withData:(const uint8_t*)data
                           size:(size_t)size;

- (void)sendUserLocales;

// MacOS embedder API callbacks
- (void)notifyDisplayUpdate:(FlutterEngineDisplayId)mainDisplayID
                refreshRate:(double)refreshRate
                 updateType:(FlutterEngineDisplaysUpdateType)updateType;

- (void)SendWindowMetricsEventWithWidth:(size_t)width
                                 height:(size_t)height
                             pixelRatio:(double)pixelRatio
                                   left:(size_t)left
                                    top:(size_t)top;

- (void)sendKeyEvent:(const FlutterKeyEvent&)event
            callback:(nullable FlutterKeyEventCallback)callback
            userData:(nullable void*)userData;
@end

NS_ASSUME_NONNULL_END

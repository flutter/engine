// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#import "flutter/shell/platform/darwin/embedder/FlutterRenderer.h"
#import "flutter/shell/platform/darwin/embedder/FlutterTextureRegistrar.h"

NS_ASSUME_NONNULL_BEGIN
// TODO(cyanglaz embedder api).
// Right now, FlutterEmbedderEngine requires a <Renderer>, at the same time, the <Renderer>
// implementer (e.g. FlutterMetalRenderer) is a subclass of |FlutterTextureRegistrar|, which
// needs <FlutterTextureRegistrarEmbedderAPIDelegate> (currently the FlutterEmbedderEngine)
// This made the initialization of FlutterEmbedderEngine and FlutterRenderer depends on each other.
// The current workaround is introducing an extra step [setEmbedderAPIDelegate] after initializing
// both objects. We should refactor this so the extra step is not required. Possibly separate
// <Renderer> from |FlutterTextureRegistrar|.
@interface FlutterEmbedderEngine : NSObject <FlutterTextureRegistrarEmbedderAPIDelegate>

/**
 * Provides the renderer config needed to initialize the engine and also handles external
 * texture management.
 */
@property(nonatomic, readonly, nullable) FlutterTextureRegistrar<FlutterRenderer>* renderer;

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

/**
 * The default init method.
 */
- (instancetype)initWithRenderer:(FlutterTextureRegistrar<FlutterRenderer>*)flutterRenderer;

- (BOOL)prepareEmbedderAPI:(NSString*)entrypoint;

/**
 * Shuts the Flutter engine if it is running. The FlutterEngine instance must always be shutdown
 * before it may be collected. Not shutting down the FlutterEngine instance before releasing it will
 * result in the leak of that engine instance.
 */
- (void)shutDownEngine;

@end

NS_ASSUME_NONNULL_END

// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// TODO(cyanglaz embedder api), move this to darwin; upport both macos and ios.
#import "flutter/shell/platform/darwin/common/framework/Headers/FlutterTexture.h"
#import "flutter/shell/platform/darwin/embedder/FlutterPresenter.h"
#import "flutter/shell/platform/darwin/embedder/FlutterTextureRegistrarDelegate.h"
#import "flutter/shell/platform/embedder/embedder.h"

NS_ASSUME_NONNULL_BEGIN

/**
 * Rendering backend agnostic FlutterRendererConfig provider to be used by the embedder API.
 */
@protocol FlutterRenderer <FlutterTextureRegistry, FlutterTextureRegistrarDelegate>

/**
 * Sets the FlutterView to render to.
 */
- (void)setFlutterView:(nullable NSObject<FlutterPresenter>*)view;

/**
 * Creates a FlutterRendererConfig that renders using the appropriate backend.
 */
- (FlutterRendererConfig)createRendererConfig;

/**
 * Called by the engine when the context's buffers should be swapped.
 */
- (BOOL)present;

/**
 * Tells the renderer that there is no Flutter content available for this frame.
 */
- (void)presentWithoutContent;

@end

NS_ASSUME_NONNULL_END

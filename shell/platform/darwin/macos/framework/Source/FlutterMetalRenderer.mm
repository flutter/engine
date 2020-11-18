// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterMetalRenderer.h"
#include "flutter/shell/platform/embedder/embedder.h"

@implementation FlutterMetalRenderer {
  // The embedding-API-level engine object.
  FLUTTER_API_SYMBOL(FlutterEngine) _engine;
}

- (instancetype)initWithFlutterEngine:(FLUTTER_API_SYMBOL(FlutterEngine))engine {
  self = [super init];
  if (self) {
    _engine = engine;
  }
  return self;
}

/**
 * Attaches to the FlutterView and sets up the renderers main command queue.
 */
- (void)attachToFlutterView:(FlutterView*)view {
}

/**
 * Creates a FlutterRendererConfig that renders using Metal.
 */
- (FlutterRendererConfig)createRendererConfig {
  FlutterRendererConfig config;
  return config;
}

#pragma mark - FlutterTextureRegistrar methods.

- (int64_t)registerTexture:(id<FlutterTexture>)texture {
  NSAssert(NO, @"External textures aren't supported when using Metal on macOS.");
  return 0;
}

- (void)textureFrameAvailable:(int64_t)textureID {
  NSAssert(NO, @"External textures aren't supported when using Metal on macOS.");
}

- (void)unregisterTexture:(int64_t)textureID {
  NSAssert(NO, @"External textures aren't supported when using Metal on macOS.");
}

@end

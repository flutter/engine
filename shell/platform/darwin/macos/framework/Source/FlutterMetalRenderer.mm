// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterMetalRenderer.h"
#include "flutter/shell/platform/darwin/macos/framework/Source/FlutterView.h"

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterEngine_Internal.h"
#include "flutter/shell/platform/embedder/embedder.h"

#pragma mark - Static methods for openGL callbacks that require the engine.

static void OnGetTexture(FlutterEngine* engine,
                         const FlutterFrameInfo* frameInfo,
                         FlutterMetalTexture* textureOut) {
  CGSize size = CGSizeMake(frameInfo->size.width, frameInfo->size.height);
  [engine.metalRenderer populateTextureForSize:size to:textureOut];
  return;
}

static bool OnPresent(FlutterEngine* engine, intptr_t metalTextureId) {
  return [engine.metalRenderer present:metalTextureId];
}

#pragma mark - FlutterMetalRenderer implementation

@implementation FlutterMetalRenderer {
  // The embedding-API-level engine object.
  FLUTTER_API_SYMBOL(FlutterEngine) _engine;

  FlutterView* _flutterView;
}

- (instancetype)initWithFlutterEngine:(FLUTTER_API_SYMBOL(FlutterEngine))engine {
  self = [super init];
  if (self) {
    _engine = engine;

    _mtlDevice = MTLCreateSystemDefaultDevice();
    if (!_mtlDevice) {
      NSLog(@"Could not acquire Metal device.");
      return nil;
    }

    _mtlCommandQueue = [_mtlDevice newCommandQueue];
    if (!_mtlCommandQueue) {
      NSLog(@"Could not create Metal command queue.");
      return nil;
    }
  }
  return self;
}

- (void)attachToFlutterView:(FlutterView*)view {
  _flutterView = view;
}

/**
 * Creates a FlutterRendererConfig that renders using Metal.
 */
- (FlutterRendererConfig)createRendererConfig {
  FlutterRendererConfig config = {
      .type = FlutterRendererType::kMetal,
      .metal.struct_size = sizeof(FlutterMetalRendererConfig),
      .metal.device = (__bridge FlutterMetalDevice)_mtlDevice,
      .metal.command_queue = (__bridge FlutterMetalCommandQueue)_mtlCommandQueue,
      .metal.texture_callback = reinterpret_cast<FlutterMetalTextureCallback>(OnGetTexture),
      .metal.present_callback = reinterpret_cast<FlutterMetalPresentCallback>(OnPresent),
  };
  return config;
}

#pragma mark - Embedder callback implementations.

- (void)populateTextureForSize:(CGSize)size to:(FlutterMetalTexture*)output {
  if (!_mtlCommandQueue || !_flutterView) {
    return;
  }
  FlutterBackingStoreDescriptor* backingStore = [_flutterView backingStoreForSize:size];
  id<MTLTexture> mtlTexture = [backingStore getMetalTexture];
  output->struct_size = sizeof(FlutterMetalTexture);
  output->texture = (__bridge void*)mtlTexture;
  output->texture_id = 1;
}

- (bool)present:(int64_t)textureId {
  if (!_mtlCommandQueue || !_flutterView) {
    return false;
  }
  [_flutterView present];
  return true;
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

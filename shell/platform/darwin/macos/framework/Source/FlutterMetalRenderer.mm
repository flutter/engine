
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterMetalRenderer.h"

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterEngine_Internal.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterExternalTextureMetal.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterViewController_Internal.h"
#include "flutter/shell/platform/embedder/embedder.h"

#pragma mark - Static callbacks that require the engine.

static FlutterMetalTexture OnGetNextDrawableForDefaultView(FlutterEngine* engine,
                                                           const FlutterFrameInfo* frameInfo) {
  // TODO(dkwingsmt): This callback only supports single-view, therefore it only
  // operates on the default view. To support multi-view, we need a new callback
  // that also receives a view ID, or pass the ID via FlutterFrameInfo.
  FlutterView* view = engine.viewController.flutterView;
  if (view == nil) {
    FML_LOG(WARNING) << "Can't create drawables on a non-existent view.";
    // FlutterMetalTexture has texture `null`, therefore is discarded.
    return FlutterMetalTexture{};
  }
  CGSize size = CGSizeMake(frameInfo->size.width, frameInfo->size.height);
  FlutterMetalRenderer* metalRenderer = reinterpret_cast<FlutterMetalRenderer*>(engine.renderer);
  return [metalRenderer createTextureForView:view size:size];
}

static bool OnPresentDrawableToDefaultView(FlutterEngine* engine,
                                           const FlutterMetalTexture* texture) {
  // TODO(dkwingsmt): This callback only supports single-view, therefore it only
  // operates on the default view. To support multi-view, we need a new callback
  // that also receives a view ID.
  FlutterView* view = engine.viewController.flutterView;
  if (view == nil) {
    return false;
  }
  return [engine.renderer present:view];
}

static bool OnAcquireExternalTexture(FlutterEngine* engine,
                                     int64_t textureIdentifier,
                                     size_t width,
                                     size_t height,
                                     FlutterMetalExternalTexture* metalTexture) {
  FlutterMetalRenderer* metalRenderer = reinterpret_cast<FlutterMetalRenderer*>(engine.renderer);
  return [metalRenderer populateTextureWithIdentifier:textureIdentifier metalTexture:metalTexture];
}

#pragma mark - FlutterMetalRenderer implementation

@implementation FlutterMetalRenderer {
  FlutterDarwinContextMetalSkia* _darwinMetalContext;
}

- (instancetype)initWithFlutterEngine:(nonnull FlutterEngine*)flutterEngine {
  self = [super initWithDelegate:self engine:flutterEngine];
  if (self) {
    _device = MTLCreateSystemDefaultDevice();
    if (!_device) {
      NSLog(@"Could not acquire Metal device.");
      return nil;
    }

    _commandQueue = [_device newCommandQueue];
    if (!_commandQueue) {
      NSLog(@"Could not create Metal command queue.");
      return nil;
    }

    _darwinMetalContext = [[FlutterDarwinContextMetalSkia alloc] initWithMTLDevice:_device
                                                                      commandQueue:_commandQueue];
  }
  return self;
}

- (FlutterRendererConfig)createRendererConfig {
  FlutterRendererConfig config = {
      .type = FlutterRendererType::kMetal,
      .metal.struct_size = sizeof(FlutterMetalRendererConfig),
      .metal.device = (__bridge FlutterMetalDeviceHandle)_device,
      .metal.present_command_queue = (__bridge FlutterMetalCommandQueueHandle)_commandQueue,
      .metal.get_next_drawable_callback =
          reinterpret_cast<FlutterMetalTextureCallback>(OnGetNextDrawableForDefaultView),
      .metal.present_drawable_callback =
          reinterpret_cast<FlutterMetalPresentCallback>(OnPresentDrawableToDefaultView),
      .metal.external_texture_frame_callback =
          reinterpret_cast<FlutterMetalTextureFrameCallback>(OnAcquireExternalTexture),
  };
  return config;
}

#pragma mark - Embedder callback implementations.

- (FlutterMetalTexture)createTextureForView:(FlutterView*)view size:(CGSize)size {
  FlutterMetalRenderBackingStore* backingStore =
      (FlutterMetalRenderBackingStore*)[view backingStoreForSize:size];
  id<MTLTexture> texture = backingStore.texture;
  FlutterMetalTexture embedderTexture;
  embedderTexture.struct_size = sizeof(FlutterMetalTexture);
  embedderTexture.texture = (__bridge void*)texture;
  embedderTexture.texture_id = reinterpret_cast<int64_t>(texture);
  return embedderTexture;
}

- (BOOL)present:(FlutterView*)view {
  [view present];
  return YES;
}

- (void)presentWithoutContent:(FlutterView*)view {
  [view presentWithoutContent];
}

#pragma mark - FlutterTextureRegistrar methods.

- (BOOL)populateTextureWithIdentifier:(int64_t)textureID
                         metalTexture:(FlutterMetalExternalTexture*)textureOut {
  id<FlutterMacOSExternalTexture> texture = [self getTextureWithID:textureID];
  FlutterExternalTextureMetal* metalTexture =
      reinterpret_cast<FlutterExternalTextureMetal*>(texture);
  return [metalTexture populateTexture:textureOut];
}

- (id<FlutterMacOSExternalTexture>)onRegisterTexture:(id<FlutterTexture>)texture {
  return [[FlutterExternalTextureMetal alloc] initWithFlutterTexture:texture
                                                  darwinMetalContext:_darwinMetalContext];
}

@end

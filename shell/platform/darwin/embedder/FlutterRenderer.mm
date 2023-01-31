// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/embedder/FlutterRenderer.h"

#import "flutter/shell/platform/darwin/embedder/FlutterEmbedderAPIBridge.h"
#import "flutter/shell/platform/darwin/embedder/FlutterExternalTexture.h"
// #import "flutter/shell/platform/darwin/macos/framework/Source/FlutterViewController_Internal.h"
// #import "flutter/shell/platform/darwin/macos/framework/Source/FlutterViewEngineProvider.h"
#include "flutter/shell/platform/embedder/embedder.h"

#pragma mark - Static callbacks that require the engine.

static FlutterMetalTexture OnGetNextDrawableForDefaultView(FlutterRenderer* renderer,
                                                           const FlutterFrameInfo* frameInfo) {
  // TODO(dkwingsmt): This callback only supports single-view, therefore it only
  // operates on the default view. To support multi-view, we need a new callback
  // that also receives a view ID, or pass the ID via FlutterFrameInfo.
  uint64_t viewId = kFlutterDefaultViewId;
  CGSize size = CGSizeMake(frameInfo->size.width, frameInfo->size.height);
  return [renderer.presenter createTextureForView:viewId size:size];
}

static bool OnPresentDrawableOfDefaultView(FlutterRenderer* renderer,
                                           const FlutterMetalTexture* texture) {
  // TODO(dkwingsmt): This callback only supports single-view, therefore it only
  // operates on the default view. To support multi-view, we need a new callback
  // that also receives a view ID.
  uint64_t viewId = kFlutterDefaultViewId;
  return [renderer.presenter present:viewId texture:texture];
}

static bool OnAcquireExternalTexture(FlutterRenderer* renderer,
                                     int64_t textureIdentifier,
                                     size_t width,
                                     size_t height,
                                     FlutterMetalExternalTexture* metalTexture) {
  return [renderer populateTextureWithIdentifier:textureIdentifier metalTexture:metalTexture];
}

#pragma mark - FlutterRenderer implementation

@implementation FlutterRenderer {
  FlutterDarwinContextMetalSkia* _darwinMetalContext;
}

- (instancetype)initWithEmbedderAPIBridge:(nonnull FlutterEmbedderAPIBridge*)bridge
                                presenter:(NSObject<FlutterPresenter>*)presenter {
  self = [super initWithDelegate:self embedderAPIBridge:bridge];
  if (self) {
    _presenter = presenter;
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
          reinterpret_cast<FlutterMetalPresentCallback>(OnPresentDrawableOfDefaultView),
      .metal.external_texture_frame_callback =
          reinterpret_cast<FlutterMetalTextureFrameCallback>(OnAcquireExternalTexture),
  };
  return config;
}

#pragma mark - FlutterTextureRegistrar methods.

- (BOOL)populateTextureWithIdentifier:(int64_t)textureID
                         metalTexture:(FlutterMetalExternalTexture*)textureOut {
  FlutterExternalTexture* texture = [self getTextureWithID:textureID];
  return [texture populateTexture:textureOut];
}

- (FlutterExternalTexture*)onRegisterTexture:(id<FlutterTexture>)texture {
  return [[FlutterExternalTexture alloc] initWithFlutterTexture:texture
                                             darwinMetalContext:_darwinMetalContext];
}

@end

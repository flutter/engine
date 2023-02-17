// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/graphics/FlutterExternalTexture.h"

#import "flutter/shell/platform/darwin/ios/framework/Source/FlutterEngine_Internal.h"
#import "flutter/shell/platform/darwin/ios/framework/Source/FlutterRenderer.h"
#import "flutter/shell/platform/darwin/ios/framework/Source/FlutterViewController_Internal.h"
#include "flutter/shell/platform/embedder/embedder.h"

#pragma mark - Static callbacks that require the engine.

const uint64_t kFlutterDefaultViewId = 0;

static void ReleaseSurface(void* surface) {
  if (surface != nullptr) {
    CFBridgingRelease(surface);
  }
}

static FlutterMetalTexture OnGetNextDrawableForDefaultView(FlutterEngine* engine,
                                                           const FlutterFrameInfo* frameInfo) {
  uint64_t viewId = kFlutterDefaultViewId;
  CGSize size = CGSizeMake(frameInfo->size.width, frameInfo->size.height);
  return [engine.renderer createTextureForView:viewId size:size];
}

static bool OnPresentDrawableOfDefaultView(FlutterEngine* engine,
                                           const FlutterMetalTexture* texture) {
  // TODO(dkwingsmt): This callback only supports single-view, therefore it only
  // operates on the default view. To support multi-view, we need a new callback
  // that also receives a view ID.
  uint64_t viewId = kFlutterDefaultViewId;
  return [engine.renderer present:viewId texture:texture];
}

static bool OnAcquireExternalTexture(FlutterEngine* engine,
                                     int64_t textureIdentifier,
                                     size_t width,
                                     size_t height,
                                     FlutterMetalExternalTexture* metalTexture) {
  return [engine.renderer populateTextureWithIdentifier:textureIdentifier
                                           metalTexture:metalTexture];
}

#pragma mark - FlutterRenderer implementation

@implementation FlutterRenderer {
  CALayer* _layer;

  // TODO(cyanglaz): embedder api, below context only supports skia. we need to also support
  // impeller.
  FlutterDarwinContextMetalSkia* _darwinMetalContextSkia;

  // We hold a reference for current drawable when one is created and
  // release immediately after committed.
  id<CAMetalDrawable> _currentDrawable;

  NSMutableDictionary<NSNumber*, FlutterExternalTexture*>* _textures;

  FlutterEngine* _flutterEngine;
  NSObject<FlutterRendererTextureRegistryDelegate>* _textureDelegate;
}

- (instancetype)initWithTextureDelegate:
    (NSObject<FlutterRendererTextureRegistryDelegate>*)textureDelegate {
  self = [super init];
  if (self) {
    _textureDelegate = textureDelegate;
    _device = [MTLCreateSystemDefaultDevice() retain];
    _textures = [[[NSMutableDictionary alloc] init] retain];
    if (!_device) {
      NSLog(@"Could not acquire Metal device.");
      return nil;
    }

    _commandQueue = [_device newCommandQueue];
    if (!_commandQueue) {
      NSLog(@"Could not create Metal command queue.");
      return nil;
    }

    _darwinMetalContextSkia =
        [[[FlutterDarwinContextMetalSkia alloc] initWithMTLDevice:_device
                                                     commandQueue:_commandQueue] retain];
  }
  return self;
}

- (void)dealloc {
  [_darwinMetalContextSkia release];
  [_device release];
  [_textures release];
  [super dealloc];
}

- (void)setLayer:(CALayer*)layer {
  _layer = layer;
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

#pragma mark - Embedder callback implementations.

// Called on raster thread
- (CAMetalLayer*)getMetalLayerSkia:(CALayer*)layer size:(CGSize)size {
  FML_DCHECK([_layer isKindOfClass:[CAMetalLayer class]]);
  CAMetalLayer* metalLayer = (CAMetalLayer*)_layer;
  metalLayer.device = _device;

  metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
  // Flutter needs to read from the color attachment in cases where there are effects such as
  // backdrop filters.
  metalLayer.framebufferOnly = NO;

  if (!CGSizeEqualToSize(size, metalLayer.drawableSize)) {
    metalLayer.drawableSize = size;
  }

  // When there are platform views in the scene, the drawable needs to be presented in the same
  // transaction as the one created for platform views. When the drawable are being presented from
  // the raster thread, there is no such transaction.
  metalLayer.presentsWithTransaction = [[NSThread currentThread] isMainThread];
  return metalLayer;
}

// Called on raster thread
- (FlutterMetalTexture)createTextureForView:(uint64_t)viewId size:(CGSize)size {
  // (TODO)cyanglaz: embedder api, use view prodider to support multiple views (platform view
  // overlay).
  if (!_layer) {
    // FlutterMetalTexture has texture `null`, therefore is discarded.
    return FlutterMetalTexture{};
  }
  CAMetalLayer* metalLayer = [self getMetalLayerSkia:_layer size:size];

  FlutterMetalTexture res;
  FML_DCHECK(!_currentDrawable);
  _currentDrawable = [[metalLayer nextDrawable] retain];
  id<MTLTexture> texture = _currentDrawable.texture;
  memset(&res, 0, sizeof(FlutterMetalTexture));
  res.struct_size = sizeof(FlutterMetalTexture);
  res.texture = (__bridge void*)texture;
  res.texture_id = reinterpret_cast<int64_t>(texture);
  res.user_data = (void*)CFBridgingRetain(self);
  res.destruction_callback = ReleaseSurface;
  return res;
}

// Called on raster thread
- (BOOL)present:(uint64_t)viewId texture:(const FlutterMetalTexture*)texture {
  if (!_layer) {
    return NO;
  }

  id<MTLCommandBuffer> commandBuffer = [_commandQueue commandBuffer];
  [commandBuffer commit];
  [commandBuffer waitUntilScheduled];
  [_currentDrawable present];
  [_currentDrawable release];
  _currentDrawable = nil;
  return YES;
}

#pragma mark - Texture methods for Embedder API

- (BOOL)populateTextureWithIdentifier:(int64_t)textureID
                         metalTexture:(FlutterMetalExternalTexture*)textureOut {
  FlutterExternalTexture* texture = [self getTextureWithID:textureID];
  return [texture populateTexture:textureOut];
}

#pragma mark - FlutterTextureRegistry.

- (int64_t)registerTexture:(id<FlutterTexture>)texture {
  FlutterExternalTexture* externalTexture =
      [[[FlutterExternalTexture alloc] initWithFlutterTexture:texture
                                           darwinMetalContext:_darwinMetalContextSkia] autorelease];
  int64_t textureID = [externalTexture textureID];
  BOOL success = [_textureDelegate registerTextureWithID:textureID];
  if (success) {
    _textures[@(textureID)] = externalTexture;
    return textureID;
  } else {
    NSLog(@"Unable to register the texture with id: %lld.", textureID);
    return 0;
  }
}

- (void)textureFrameAvailable:(int64_t)textureID {
  BOOL success = [_textureDelegate markTextureFrameAvailable:textureID];
  if (!success) {
    NSLog(@"Unable to mark texture with id %lld as available.", textureID);
  }
}

- (void)unregisterTexture:(int64_t)textureID {
  bool success = [_textureDelegate unregisterTextureWithID:textureID];
  if (success) {
    [_textures removeObjectForKey:@(textureID)];
  } else {
    NSLog(@"Unable to unregister texture with id: %lld.", textureID);
  }
}

- (FlutterExternalTexture*)getTextureWithID:(int64_t)textureID {
  return _textures[@(textureID)];
}

@end

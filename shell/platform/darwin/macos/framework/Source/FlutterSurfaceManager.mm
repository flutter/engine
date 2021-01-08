// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterSurfaceManager.h"

#import <Metal/Metal.h>
#import <OpenGL/gl.h>

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterFrameBufferProvider.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterIOSurfaceHolder.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/MacOSGLContextSwitch.h"

enum {
  kFlutterSurfaceManagerFrontBuffer = 0,
  kFlutterSurfaceManagerBackBuffer = 1,
  kFlutterSurfaceManagerBufferCount,
};

@implementation FlutterGLSurfaceManager {
  CGSize _surfaceSize;
  CALayer* _containingLayer;  // provided (parent layer)
  CALayer* _contentLayer;

  NSOpenGLContext* _openGLContext;

  FlutterIOSurfaceHolder* _ioSurfaces[kFlutterSurfaceManagerBufferCount];
  FlutterFrameBufferProvider* _frameBuffers[kFlutterSurfaceManagerBufferCount];
}

- (instancetype)initWithLayer:(CALayer*)containingLayer
                openGLContext:(NSOpenGLContext*)openGLContext {
  if (self = [super init]) {
    _containingLayer = containingLayer;
    _openGLContext = openGLContext;

    // Layer for content. This is separate from provided layer, because it needs to be flipped
    // vertically if we render to OpenGL texture
    _contentLayer = [[CALayer alloc] init];
    [_containingLayer addSublayer:_contentLayer];

    _frameBuffers[0] = [[FlutterFrameBufferProvider alloc] initWithOpenGLContext:_openGLContext];
    _frameBuffers[1] = [[FlutterFrameBufferProvider alloc] initWithOpenGLContext:_openGLContext];

    _ioSurfaces[0] = [[FlutterIOSurfaceHolder alloc] init];
    _ioSurfaces[1] = [[FlutterIOSurfaceHolder alloc] init];
  }
  return self;
}

- (void)ensureSurfaceSize:(CGSize)size {
  if (CGSizeEqualToSize(size, _surfaceSize)) {
    return;
  }
  _surfaceSize = size;

  MacOSGLContextSwitch context_switch(_openGLContext);

  for (int i = 0; i < kFlutterSurfaceManagerBufferCount; ++i) {
    [_ioSurfaces[i] recreateIOSurfaceWithSize:size];

    GLuint fbo = [_frameBuffers[i] glFrameBufferId];
    GLuint texture = [_frameBuffers[i] glTextureId];
    [_ioSurfaces[i] bindSurfaceToTexture:texture fbo:fbo size:size];
  }
}

- (void)swapBuffers {
  _contentLayer.frame = _containingLayer.bounds;

  // The surface is an OpenGL texture, which means it has origin in bottom left corner
  // and needs to be flipped vertically
  _contentLayer.transform = CATransform3DMakeScale(1, -1, 1);
  IOSurfaceRef contentIOSurface = [_ioSurfaces[kFlutterSurfaceManagerBackBuffer] ioSurface];
  [_contentLayer setContents:(__bridge id)contentIOSurface];

  std::swap(_ioSurfaces[kFlutterSurfaceManagerBackBuffer],
            _ioSurfaces[kFlutterSurfaceManagerFrontBuffer]);
  std::swap(_frameBuffers[kFlutterSurfaceManagerBackBuffer],
            _frameBuffers[kFlutterSurfaceManagerFrontBuffer]);
}

- (FlutterBackingStoreDescriptor*)renderBufferDescriptor {
  uint32_t fboId = [_frameBuffers[kFlutterSurfaceManagerBackBuffer] glFrameBufferId];
  return [[FlutterBackingStoreDescriptor alloc] initOpenGLDescriptorWithFBOId:fboId];
}

@end

@implementation FlutterMetalSurfaceManager {
  CGSize _surfaceSize;
  id<MTLDevice> _mtlDevice;
  id<MTLCommandQueue> _mtlCommandQueue;
  CAMetalLayer* _containingLayer;  // provided (parent layer)
  CALayer* _contentLayer;

  FlutterIOSurfaceHolder* _ioSurfaces[kFlutterSurfaceManagerBufferCount];
  id<MTLTexture> _textures[kFlutterSurfaceManagerBufferCount];
}

- (nullable instancetype)initWithDevice:(nonnull id<MTLDevice>)device
                           commandQueue:(nonnull id<MTLCommandQueue>)commandQueue
                             metalLayer:(nonnull CAMetalLayer*)layer {
  self = [super init];
  if (self) {
    _mtlDevice = device;
    _mtlCommandQueue = commandQueue;
    _containingLayer = layer;

    // Layer for content. This is separate from provided layer, because it needs to be flipped
    // vertically if we render to Metal texture
    _contentLayer = [[CALayer alloc] init];
    [_containingLayer addSublayer:_contentLayer];

    _ioSurfaces[0] = [[FlutterIOSurfaceHolder alloc] init];
    _ioSurfaces[1] = [[FlutterIOSurfaceHolder alloc] init];
  }
  return self;
}

- (void)ensureSurfaceSize:(CGSize)size {
  if (CGSizeEqualToSize(size, _surfaceSize)) {
    return;
  }
  _surfaceSize = size;

  for (size_t i = 0; i < kFlutterSurfaceManagerBufferCount; i++) {
    [_ioSurfaces[i] recreateIOSurfaceWithSize:size];
    MTLTextureDescriptor* textureDescriptor =
        [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatBGRA8Unorm
                                                           width:size.width
                                                          height:size.height
                                                       mipmapped:NO];
    textureDescriptor.usage =
        MTLTextureUsageShaderRead | MTLTextureUsageRenderTarget | MTLTextureUsageShaderWrite;
    // plane = 0 for BGRA.
    _textures[i] = [_mtlDevice newTextureWithDescriptor:textureDescriptor
                                              iosurface:[_ioSurfaces[i] ioSurface]
                                                  plane:0];
  }
}

- (void)swapBuffers {
  _contentLayer.frame = _containingLayer.bounds;

  IOSurfaceRef backBuffer = [_ioSurfaces[kFlutterSurfaceManagerBackBuffer] ioSurface];
  [_contentLayer setContents:(__bridge id)backBuffer];

  std::swap(_ioSurfaces[kFlutterSurfaceManagerBackBuffer],
            _ioSurfaces[kFlutterSurfaceManagerFrontBuffer]);
  std::swap(_textures[kFlutterSurfaceManagerBackBuffer],
            _textures[kFlutterSurfaceManagerFrontBuffer]);
}

- (FlutterBackingStoreDescriptor*)renderBufferDescriptor {
  id<MTLTexture> texture = _textures[kFlutterSurfaceManagerBackBuffer];
  return [[FlutterBackingStoreDescriptor alloc] initMetalDescriptorWithTexture:texture];
}

@end

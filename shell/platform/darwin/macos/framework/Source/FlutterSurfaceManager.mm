// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterSurfaceManager.h"

#include <cstddef>
#include <OpenGL/gl.h>
#include <vector>

#import "flutter/shell/platform/darwin/macos/framework/Source/MacOSGLContextSwitch.h"

enum {
  kFlutterSurfaceManagerFrontBuffer = 0,
  kFlutterSurfaceManagerBackBuffer = 1,
  kFlutterSurfaceManagerBufferCount,
};

@interface IOSurfaceHolder : NSObject

- (instancetype)init:(size_t)numSurfaces;

- (void)recreateIOSurfacesWithSize:(CGSize)size;

- (IOSurfaceRef)get:(size_t)index;

- (void)swapSurfacesAtIndex:(size_t)i andIndex:(size_t)j;

@end

@implementation IOSurfaceHolder {
  size_t _numSurfaces;
  std::vector<IOSurfaceRef> _ioSurfaces;
}

- (instancetype)init:(size_t)numSurfaces {
  self = [super init];
  if (self) {
    _numSurfaces = numSurfaces;
    _ioSurfaces.resize(numSurfaces);
  }
  return self;
}

- (void)recreateIOSurfacesWithSize:(CGSize)size {
  for (IOSurfaceRef& ioSurface : _ioSurfaces) {
    if (ioSurface) {
      CFRelease(ioSurface);
    }
    ioSurface = [IOSurfaceHolder createIOSurfaceWithSize:size];
  }
}

- (IOSurfaceRef)get:(size_t)index {
  return _ioSurfaces[index];
}

- (void)swapSurfacesAtIndex:(size_t)i andIndex:(size_t)j {
  std::swap(_ioSurfaces[i], _ioSurfaces[j]);
}

- (void)dealloc {
  for (const IOSurfaceRef& ioSurface : _ioSurfaces) {
    if (ioSurface) {
      CFRelease(ioSurface);
    }
  }
}

+ (IOSurfaceRef)createIOSurfaceWithSize:(CGSize)size {
  const unsigned pixelFormat = 'BGRA';
  const unsigned bytesPerElement = 4;

  size_t bytesPerRow = IOSurfaceAlignProperty(kIOSurfaceBytesPerRow, size.width * bytesPerElement);
  size_t totalBytes = IOSurfaceAlignProperty(kIOSurfaceAllocSize, size.height * bytesPerRow);
  NSDictionary* options = @{
    (id)kIOSurfaceWidth : @(size.width),
    (id)kIOSurfaceHeight : @(size.height),
    (id)kIOSurfacePixelFormat : @(pixelFormat),
    (id)kIOSurfaceBytesPerElement : @(bytesPerElement),
    (id)kIOSurfaceBytesPerRow : @(bytesPerRow),
    (id)kIOSurfaceAllocSize : @(totalBytes),
  };
  return IOSurfaceCreate((CFDictionaryRef)options);
}

@end

@implementation FlutterGLSurfaceManager {
  CGSize _surfaceSize;
  CALayer* _containingLayer;  // provided (parent layer)
  CALayer* _contentLayer;

  NSOpenGLContext* _openGLContext;
  IOSurfaceHolder* _ioSurfaceHolder;
  uint32_t _frameBufferId[kFlutterSurfaceManagerBufferCount];
  uint32_t _backingTexture[kFlutterSurfaceManagerBufferCount];
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

    _ioSurfaceHolder = [[IOSurfaceHolder alloc] init:kFlutterSurfaceManagerBufferCount];

    MacOSGLContextSwitch context_switch(openGLContext);

    glGenFramebuffers(kFlutterSurfaceManagerBufferCount, _frameBufferId);
    glGenTextures(kFlutterSurfaceManagerBufferCount, _backingTexture);

    [self createFramebuffer:_frameBufferId[0] withBackingTexture:_backingTexture[0]];
    [self createFramebuffer:_frameBufferId[1] withBackingTexture:_backingTexture[1]];
  }
  return self;
}

- (void)createFramebuffer:(uint32_t)fbo withBackingTexture:(uint32_t)texture {
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glBindTexture(GL_TEXTURE_RECTANGLE_ARB, texture);
  glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);
}

- (void)ensureSurfaceSize:(CGSize)size {
  if (CGSizeEqualToSize(size, _surfaceSize)) {
    return;
  }
  _surfaceSize = size;

  MacOSGLContextSwitch context_switch(_openGLContext);
  [_ioSurfaceHolder recreateIOSurfacesWithSize:size];

  for (int i = 0; i < kFlutterSurfaceManagerBufferCount; ++i) {
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, _backingTexture[i]);

    CGLTexImageIOSurface2D(CGLGetCurrentContext(), GL_TEXTURE_RECTANGLE_ARB, GL_RGBA,
                           int(size.width), int(size.height), GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV,
                           [_ioSurfaceHolder get:i], 0 /* plane */);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, _frameBufferId[i]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE_ARB,
                           _backingTexture[i], 0);

    NSAssert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE,
             @"Framebuffer status check failed");
  }
}

- (void)swapBuffers {
  _contentLayer.frame = _containingLayer.bounds;

  // The surface is an OpenGL texture, which means it has origin in bottom left corner
  // and needs to be flipped vertically
  _contentLayer.transform = CATransform3DMakeScale(1, -1, 1);
  IOSurfaceRef backBuffer = [_ioSurfaceHolder get:kFlutterSurfaceManagerBackBuffer];
  [_contentLayer setContents:(__bridge id)backBuffer];

  [_ioSurfaceHolder swapSurfacesAtIndex:kFlutterSurfaceManagerBackBuffer
                               andIndex:kFlutterSurfaceManagerFrontBuffer];
  std::swap(_frameBufferId[kFlutterSurfaceManagerBackBuffer],
            _frameBufferId[kFlutterSurfaceManagerFrontBuffer]);
  std::swap(_backingTexture[kFlutterSurfaceManagerBackBuffer],
            _backingTexture[kFlutterSurfaceManagerFrontBuffer]);
}

- (FlutterBackingStoreDescriptor*)getBackingStore {
  auto fboId = _frameBufferId[kFlutterSurfaceManagerBackBuffer];
  return [[FlutterBackingStoreDescriptor alloc] initOpenGLDescriptorWithFBOId:fboId];
}

@end

@implementation FlutterMetalSurfaceManager {
  CGSize _surfaceSize;
  id<MTLDevice> _mtlDevice;
  id<MTLCommandQueue> _mtlCommandQueue;
  CAMetalLayer* _containingLayer;  // provided (parent layer)
  CALayer* _contentLayer;
  IOSurfaceHolder* _ioSurfaceHolder;

  id<MTLTexture> _textures[kFlutterSurfaceManagerBufferCount];
}

- (nullable instancetype)initWithDevice:(nonnull id<MTLDevice>)device
                        andCommandQueue:(nonnull id<MTLCommandQueue>)commandQueue
                        andCAMetalLayer:(nonnull CAMetalLayer*)layer {
  self = [super init];
  if (self) {
    _mtlDevice = device;
    _mtlCommandQueue = commandQueue;

    // Layer for content. This is separate from provided layer, because it needs to be flipped
    // vertically if we render to Metal texture
    _contentLayer = [[CALayer alloc] init];
    [_containingLayer addSublayer:_contentLayer];

    _ioSurfaceHolder = [[IOSurfaceHolder alloc] init:kFlutterSurfaceManagerBufferCount];
  }
  return self;
}

- (void)ensureSurfaceSize:(CGSize)size {
  if (CGSizeEqualToSize(size, _surfaceSize)) {
    return;
  }
  _surfaceSize = size;
  [_ioSurfaceHolder recreateIOSurfacesWithSize:size];

  for (size_t i = 0; i < kFlutterSurfaceManagerBufferCount; i++) {
    IOSurfaceRef ioSurface = [_ioSurfaceHolder get:i];
    MTLTextureDescriptor* textureDescriptor =
        [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatBGRA8Unorm
                                                           width:size.width
                                                          height:size.height
                                                       mipmapped:NO];
    // plane = 0 for BGRA.
    _textures[i] = [_mtlDevice newTextureWithDescriptor:textureDescriptor
                                              iosurface:ioSurface
                                                  plane:0];
  }
}

- (void)swapBuffers {
  _contentLayer.frame = _containingLayer.bounds;

  // The surface is an Metal texture, which means it has origin in bottom left corner
  // and needs to be flipped vertically
  _contentLayer.transform = CATransform3DMakeScale(1, -1, 1);
  IOSurfaceRef backBuffer = [_ioSurfaceHolder get:kFlutterSurfaceManagerBackBuffer];
  [_contentLayer setContents:(__bridge id)backBuffer];

  [_ioSurfaceHolder swapSurfacesAtIndex:kFlutterSurfaceManagerBackBuffer
                               andIndex:kFlutterSurfaceManagerFrontBuffer];
  std::swap(_textures[kFlutterSurfaceManagerBackBuffer],
            _textures[kFlutterSurfaceManagerFrontBuffer]);
}

- (FlutterBackingStoreDescriptor*)getBackingStore {
  auto texture = _textures[kFlutterSurfaceManagerBackBuffer];
  return [[FlutterBackingStoreDescriptor alloc] initMetalDescriptorWithTexture:texture];
}

@end

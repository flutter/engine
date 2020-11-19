// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterSurfaceManager.h"

#include <OpenGL/gl.h>

#import "flutter/shell/platform/darwin/macos/framework/Source/MacOSGLContextSwitch.h"

enum {
  kFlutterSurfaceManagerFrontBuffer = 0,
  kFlutterSurfaceManagerBackBuffer = 1,
  kFlutterSurfaceManagerBufferCount,
};

@interface FlutterSurfaceManager () {
  CGSize _surfaceSize;
  CALayer* _containingLayer;  // provided (parent layer)
  CALayer* _contentLayer;

  NSOpenGLContext* _openGLContext;
  uint32_t _frameBufferId[kFlutterSurfaceManagerBufferCount];
  uint32_t _backingTexture[kFlutterSurfaceManagerBufferCount];
  IOSurfaceRef _ioSurface[kFlutterSurfaceManagerBufferCount];
}
@end

@implementation FlutterSurfaceManager

- (instancetype)initWithLayer:(CALayer*)containingLayer
                openGLContext:(NSOpenGLContext*)openGLContext
              numFramebuffers:(int)numFramebuffers {
  if (self = [super init]) {
    _containingLayer = containingLayer;
    _openGLContext = openGLContext;

    // Layer for content. This is separate from provided layer, because it needs to be flipped
    // vertically if we render to OpenGL texture
    _contentLayer = [[CALayer alloc] init];
    [_containingLayer addSublayer:_contentLayer];

    MacOSGLContextSwitch context_switch(openGLContext);

    glGenFramebuffers(numFramebuffers, _frameBufferId);
    glGenTextures(numFramebuffers, _backingTexture);

    for (int i = 0; i < numFramebuffers; ++i) {
      [self createFramebuffer:_frameBufferId[i] withBackingTexture:_backingTexture[1]];
    }
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

  for (int i = 0; i < kFlutterSurfaceManagerBufferCount; ++i) {
    [self recreateIOSurface:i size:size];
    [self backTextureWithIOSurface:i size:size];
  }
}

- (void)recreateIOSurface:(int)index size:(CGSize)size {
  if (_ioSurface[index]) {
    CFRelease(_ioSurface[index]);
  }

  unsigned pixelFormat = 'BGRA';
  unsigned bytesPerElement = 4;

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
  _ioSurface[index] = IOSurfaceCreate((CFDictionaryRef)options);
}

- (void)backTextureWithIOSurface:(int)index size:(CGSize)size {
  glBindTexture(GL_TEXTURE_RECTANGLE_ARB, _backingTexture[index]);

  CGLTexImageIOSurface2D(CGLGetCurrentContext(), GL_TEXTURE_RECTANGLE_ARB, GL_RGBA, int(size.width),
                         int(size.height), GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, _ioSurface[index],
                         0 /* plane */);
  glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);

  glBindFramebuffer(GL_FRAMEBUFFER, _frameBufferId[index]);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE_ARB,
                         _backingTexture[index], 0);

  NSAssert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE,
           @"Framebuffer status check failed");
}

- (void)setLayerContent {
  [self setLayerContentWithIOSurface:kFlutterSurfaceManagerBackBuffer];
}

- (void)setLayerContentWithIOSurface:(int)ioSurfaceNum {
  _contentLayer.frame = _containingLayer.bounds;

  // The surface is an OpenGL texture, which means it has origin in bottom left corner
  // and needs to be flipped vertically
  _contentLayer.transform = CATransform3DMakeScale(1, -1, 1);
  [_contentLayer setContents:(__bridge id)_ioSurface[ioSurfaceNum]];
}

- (void)swapBuffers {
  std::swap(_ioSurface[kFlutterSurfaceManagerBackBuffer],
            _ioSurface[kFlutterSurfaceManagerFrontBuffer]);
  std::swap(_frameBufferId[kFlutterSurfaceManagerBackBuffer],
            _frameBufferId[kFlutterSurfaceManagerFrontBuffer]);
  std::swap(_backingTexture[kFlutterSurfaceManagerBackBuffer],
            _backingTexture[kFlutterSurfaceManagerFrontBuffer]);
}

- (uint32_t)glFrameBufferBackId {
  return _frameBufferId[kFlutterSurfaceManagerBackBuffer];
}

- (uint32_t)glFrameBufferFrontId {
  return _frameBufferId[kFlutterSurfaceManagerFrontBuffer];
}

- (void)dealloc {
  [_contentLayer removeFromSuperlayer];
  glDeleteFramebuffers(kFlutterSurfaceManagerBufferCount, _frameBufferId);
  glDeleteTextures(kFlutterSurfaceManagerBufferCount, _backingTexture);
  for (int i = 0; i < kFlutterSurfaceManagerBufferCount; ++i) {
    if (_ioSurface[i]) {
      CFRelease(_ioSurface[i]);
    }
  }
}

@end

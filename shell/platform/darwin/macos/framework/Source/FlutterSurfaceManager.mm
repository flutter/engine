#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterSurfaceManager.h"

#include <OpenGL/gl.h>

enum {
  kFront = 0,
  kBack = 1,
  kBufferCount,
};

@interface FlutterSurfaceManager () {
  CGSize surfaceSize;
  CALayer* layer;
  NSOpenGLContext* openGLContext;
  uint32_t _frameBufferId[kBufferCount];
  uint32_t _backingTexture[kBufferCount];
  IOSurfaceRef _ioSurface[kBufferCount];
}
@end

@implementation FlutterSurfaceManager

- (instancetype)initWithLayer:(CALayer*)layer_ openGLContext:(NSOpenGLContext*)opengLContext_ {
  if (self = [super init]) {
    layer = layer_;
    openGLContext = opengLContext_;

    NSOpenGLContext* prev = [NSOpenGLContext currentContext];
    [openGLContext makeCurrentContext];
    glGenFramebuffers(2, _frameBufferId);
    glGenTextures(2, _backingTexture);

    glBindFramebuffer(GL_FRAMEBUFFER, _frameBufferId[0]);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, _backingTexture[0]);
    glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, _frameBufferId[1]);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, _backingTexture[1]);
    glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);

    if (prev) {
      [prev makeCurrentContext];
    } else {
      [NSOpenGLContext clearCurrentContext];
    }
  }
  return self;
}

- (void)ensureSurfaceSize:(CGSize)size {
  if (CGSizeEqualToSize(size, surfaceSize)) {
    return;
  }
  surfaceSize = size;
  NSOpenGLContext* prev = [NSOpenGLContext currentContext];
  [openGLContext makeCurrentContext];

  for (int i = 0; i < kBufferCount; ++i) {
    if (_ioSurface[i]) {
      CFRelease(_ioSurface[i]);
    }
    unsigned pixelFormat = 'BGRA';
    unsigned bytesPerElement = 4;

    size_t bytesPerRow =
        IOSurfaceAlignProperty(kIOSurfaceBytesPerRow, size.width * bytesPerElement);
    size_t totalBytes = IOSurfaceAlignProperty(kIOSurfaceAllocSize, size.height * bytesPerRow);
    NSDictionary* options = @{
      (id)kIOSurfaceWidth : @(size.width),
      (id)kIOSurfaceHeight : @(size.height),
      (id)kIOSurfacePixelFormat : @(pixelFormat),
      (id)kIOSurfaceBytesPerElement : @(bytesPerElement),
      (id)kIOSurfaceBytesPerRow : @(bytesPerRow),
      (id)kIOSurfaceAllocSize : @(totalBytes),
    };
    _ioSurface[i] = IOSurfaceCreate((CFDictionaryRef)options);

    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, _backingTexture[i]);

    CGLTexImageIOSurface2D(CGLGetCurrentContext(), GL_TEXTURE_RECTANGLE_ARB, GL_RGBA,
                           int(size.width), int(size.height), GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV,
                           _ioSurface[i], 0 /* plane */);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, _frameBufferId[i]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE_ARB,
                           _backingTexture[i], 0);
  }
  if (prev) {
    [prev makeCurrentContext];
  } else {
    [NSOpenGLContext clearCurrentContext];
  }
}

- (void)swapBuffers {
  [layer setContents:(__bridge id)_ioSurface[kBack]];
  std::swap(_ioSurface[kBack], _ioSurface[kFront]);
  std::swap(_frameBufferId[kBack], _frameBufferId[kFront]);
  std::swap(_backingTexture[kBack], _backingTexture[kFront]);
}

- (uint32_t)glFrameBufferId {
  return _frameBufferId[kBack];
}

- (void)dealloc {
  for (int i = 0; i < kBufferCount; ++i) {
    if (_ioSurface[i]) {
      CFRelease(_ioSurface[i]);
    }
  }
}

@end

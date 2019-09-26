// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterExternalTextureGL.h"

#import <AppKit/AppKit.h>
#import <CoreVideo/CoreVideo.h>
#import <OpenGL/gl.h>

static void OnCVOpenGLTextureRelease(CVOpenGLTextureRef cvOpenGLTexture) {
  CVOpenGLTextureRelease(cvOpenGLTexture);
}

@implementation FlutterExternalTextureGL {
  /**
   * OpenGL texture cache.
   */
  CVOpenGLTextureCacheRef _openGLTextureCache;
  /**
   * The pixel buffer copied from the user side will be released
   * when the flutter engine renders it.
   */
  CVPixelBufferRef _pixelBuffer;
  /**
   * User side texture object, used to copy pixel buffer.
   */
  id<FlutterTexture> _texture;
}

- (instancetype)initWithFlutterTexture:(id<FlutterTexture>)texture {
  self = [super init];
  if (self) {
    _texture = texture;
  }
  return self;
}

- (int64_t)textureID {
  return reinterpret_cast<int64_t>(self);
}

- (BOOL)populateTextureWithWidth:(size_t)width
                          height:(size_t)height
                   openGLTexture:(FlutterOpenGLTexture*)openGLTexture {
  // Copy the pixel buffer from the FlutterTexture instance implemented on the user side.
  _pixelBuffer = [_texture copyPixelBuffer];

  if (!_pixelBuffer) {
    return NO;
  }

  // Create the opengl texture cache if necessary.
  if (!_openGLTextureCache) {
    CGLContextObj context = [NSOpenGLContext currentContext].CGLContextObj;
    CGLPixelFormatObj format = CGLGetPixelFormat(context);
    if (CVOpenGLTextureCacheCreate(kCFAllocatorDefault, NULL, context, format, NULL,
                                   &_openGLTextureCache) != kCVReturnSuccess) {
      NSLog(@"Could not create texture cache.");
      CVPixelBufferRelease(_pixelBuffer);
      return NO;
    }
  }

  // Try to clear the cache of OpenGL textures to save memory.
  CVOpenGLTextureCacheFlush(_openGLTextureCache, 0);

  CVOpenGLTextureRef cvOpenGLTexture = NULL;
  if (CVOpenGLTextureCacheCreateTextureFromImage(kCFAllocatorDefault, _openGLTextureCache,
                                                 _pixelBuffer, NULL,
                                                 &cvOpenGLTexture) != kCVReturnSuccess) {
    CVPixelBufferRelease(_pixelBuffer);
    return NO;
  }

  openGLTexture->target = static_cast<uint32_t>(CVOpenGLTextureGetTarget(cvOpenGLTexture));
  openGLTexture->name = static_cast<uint32_t>(CVOpenGLTextureGetName(cvOpenGLTexture));
  openGLTexture->format = static_cast<uint32_t>(GL_RGBA8);
  openGLTexture->destruction_callback = (VoidCallback)OnCVOpenGLTextureRelease;
  openGLTexture->user_data = cvOpenGLTexture;
  openGLTexture->width = CVPixelBufferGetWidth(_pixelBuffer);
  openGLTexture->height = CVPixelBufferGetHeight(_pixelBuffer);

  CVPixelBufferRelease(_pixelBuffer);
  return YES;
}

- (void)dealloc {
  CVOpenGLTextureCacheRelease(_openGLTextureCache);
}

@end

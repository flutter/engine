// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/FLEExternalTextureGL.h"

#import <AppKit/AppKit.h>
#import <CoreVideo/CoreVideo.h>
#import <OpenGL/OpenGL.h>

@implementation FLEExternalTextureGL {
  CVOpenGLTextureCacheRef _textureCache;
  CVPixelBufferRef _pixelBuffer;
  id<FLETexture> _texture;
}

- (instancetype)initWithFLETexture:(id<FLETexture>)texture {
  self = [super init];
  if (self) {
    _texture = texture;
    _pixelBuffer = nil;
    _textureCache = nil;
  }
  return self;
}

static void OnGLTextureRelease(CVPixelBufferRef pixelBuffer) {
  CVPixelBufferRelease(pixelBuffer);
}

- (int64_t)textureId {
  return (NSInteger)(self);
}

- (BOOL)populateTextureWithWidth:(size_t)width
                          height:(size_t)height
                         texture:(FlutterOpenGLTexture*)texture {
  if (!_texture) {
    return NO;
  }

  // Copy image buffer from external texture.
  _pixelBuffer = [_texture copyPixelBuffer:width height:height];

  if (!_pixelBuffer) {
    return NO;
  }

  // Create the texture cache if necessary.
  if (!_textureCache) {
    CGLContextObj context = [NSOpenGLContext currentContext].CGLContextObj;
    CGLPixelFormatObj format = CGLGetPixelFormat(context);
    if (CVOpenGLTextureCacheCreate(kCFAllocatorDefault, NULL, context, format, NULL,
                                   &_textureCache) != kCVReturnSuccess) {
      NSLog(@"Could not create texture cache.");
      CVPixelBufferRelease(_pixelBuffer);
      return NO;
    }
  }

  // Try to clear the cache of OpenGL textures to save memory.
  CVOpenGLTextureCacheFlush(_textureCache, 0);

  CVOpenGLTextureRef openGLTexture = NULL;
  if (CVOpenGLTextureCacheCreateTextureFromImage(kCFAllocatorDefault, _textureCache, _pixelBuffer,
                                                 NULL, &openGLTexture) != kCVReturnSuccess) {
    CVPixelBufferRelease(_pixelBuffer);
    return NO;
  }

  texture->target = CVOpenGLTextureGetTarget(openGLTexture);
  texture->name = CVOpenGLTextureGetName(openGLTexture);
  texture->format = GL_RGBA8;
  texture->destruction_callback = (VoidCallback)&OnGLTextureRelease;
  texture->user_data = openGLTexture;

  CVPixelBufferRelease(_pixelBuffer);
  return YES;
}

- (void)dealloc {
  CVOpenGLTextureCacheRelease(_textureCache);
}

@end

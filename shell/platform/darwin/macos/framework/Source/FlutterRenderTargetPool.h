// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>
#import <OpenGL/OpenGL.h>
#import <OpenGL/gl.h>

#include "flutter/fml/logging.h"

NS_ASSUME_NONNULL_BEGIN

// TODO add docs for this class.
class FlutterFrameBuffer {
 public:
  // Call MakeCurrent on the GL context before initializing.
  FlutterFrameBuffer(NSOpenGLContext* glContext, uint32_t width, uint32_t height)
      : width_(width), height_(height) {
    FML_CHECK(glContext);
    glContext_ = glContext;
    fbo_ = GL_NONE;
    texture_ = GL_NONE;
    SetupOffscreenFrameBuffer();
  }

  ~FlutterFrameBuffer() { Destroy(); }

  GLuint GetFBOId() const { return fbo_; }

  uint32_t GetWidth() const { return width_; }

  uint32_t GetHeight() const { return height_; }

 private:
  void SetupOffscreenFrameBuffer() {
    [glContext_ makeCurrentContext];
    glGenFramebuffersEXT(1, &fbo_);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo_);

    AllocateRenderBuffer();

    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, texture_,
                              0);
    FML_CHECK(glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT == GL_FRAMEBUFFER_COMPLETE_EXT));
  }

  void AllocateRenderBuffer() {
    glBindTexture(GL_TEXTURE_2D, texture_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  }

  void Destroy() {
    [glContext_ makeCurrentContext];
    glDeleteTextures(1, &texture_);
    glDeleteFramebuffersEXT(1, &fbo_);
  }

  NSOpenGLContext* glContext_;
  const uint32_t width_;
  const uint32_t height_;
  GLuint fbo_;
  GLuint texture_;
};

/**
 * Manages the lifecycle of FBOs backing the frames inflight.
 */
@interface FlutterRenderTargetPool : NSObject

/**
 * Initializes a render target pool.
 */
- (nonnull instancetype)initWithGLContext:(NSOpenGLContext*)glContext;

/**
 * Returns the FBO ID for the frame buffer that engine renders to.
 */
- (uint32_t)getFrameBufferWithWidth:(uint32_t)width height:(uint32_t)height;

/**
 * Blits the FBO to window and returns it to the render target pool.
 */
- (bool)presentFrameBuffer:(uint32_t)fbo;

NS_ASSUME_NONNULL_END

@end

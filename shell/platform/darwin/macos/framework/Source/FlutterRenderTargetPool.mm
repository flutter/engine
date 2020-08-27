// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// FLUTTER_NOLINT

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterRenderTargetPool.h"

#include <map>
#include <memory>

@implementation FlutterRenderTargetPool {
  /**
   * Map containing the frame buffers in-flight, keyed on fbo id.
   */
  std::map<uint32_t, std::unique_ptr<FlutterFrameBuffer>> _fbos;
  /**
   * GLContext associated with the main window.
   */
  NSOpenGLContext* _glContext;
}

- (nonnull instancetype)initWithGLContext:(NSOpenGLContext*)glContext {
  self = [super init];
  if (self) {
    _glContext = glContext;
  }
  return self;
}

- (uint32_t)getFrameBufferWithWidth:(uint32_t)width height:(uint32_t)height {
  auto fbo = std::make_unique<FlutterFrameBuffer>(_glContext, width, height);
  const uint32_t fboId = fbo->GetFBOId();
  _fbos[fboId] = std::move(fbo);
  NSLog(@"got fbo %u", fboId);
  return fboId;
}

- (bool)presentFrameBuffer:(uint32_t)fbo windowWidth:(uint32_t)width windowHeight:(uint32_t)height {
  [_glContext makeCurrentContext];

  std::unique_ptr<FlutterFrameBuffer> flutterFrameBuffer = std::move(_fbos[fbo]);
  FML_CHECK(flutterFrameBuffer);
  _fbos.erase(_fbos.find(fbo));

  glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0u);  // window FBO.

  const uint32_t src_width = flutterFrameBuffer->GetWidth();
  const uint32_t src_height = flutterFrameBuffer->GetHeight();
  const uint32_t dst_height = height;

  if (src_height > dst_height) {
    uint32_t y_offset = src_height - dst_height;
    glBlitFramebuffer(0, y_offset, src_width, src_height, 0, 0, width, dst_height,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);
  } else {
    uint32_t y_offset = dst_height - src_height;
    glBlitFramebuffer(0, 0, src_width, src_height, 0, y_offset, width, dst_height,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);
  }

  NSLog(@"presenting %u - s: (%u, %u) to d: (%u, %u).", fbo, src_width, src_height, width, height);

  [_glContext flushBuffer];
  return true;
}

- (void)dealloc {
  _fbos.clear();
}

@end

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
  return fboId;
}

- (void)presentFrameBuffer:(uint32_t)fbo {
  // flush
}

- (void)dealloc {
  _fbos.clear();
}

@end

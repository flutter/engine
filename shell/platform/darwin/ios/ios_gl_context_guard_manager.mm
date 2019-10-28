// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ios_gl_context_guard_manager.h"

namespace flutter {

IOSGLContextGuardManager::IOSGLContextGuardManager() {
  resource_context_.reset([[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3]);
  stored_ = fml::scoped_nsobject<NSMutableArray>([[NSMutableArray new] retain]);
  resource_context_.reset([[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3]);
  if (resource_context_ != nullptr) {
    context_.reset([[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3
                                         sharegroup:resource_context_.get().sharegroup]);
  } else {
    resource_context_.reset([[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2]);
    context_.reset([[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2
                                         sharegroup:resource_context_.get().sharegroup]);
  }
};

IOSGLContextGuardManager::IOSGLContextAutoRelease IOSGLContextGuardManager::MakeCurrent() {
  return IOSGLContextAutoRelease(*this, context_);
}

IOSGLContextGuardManager::IOSGLContextAutoRelease IOSGLContextGuardManager::ResourceMakeCurrent() {
  return IOSGLContextAutoRelease(*this, resource_context_);
}

bool IOSGLContextGuardManager::PushContext(fml::scoped_nsobject<EAGLContext> context) {
  if ([EAGLContext currentContext] == nil) {
    [stored_.get() addObject:[NSNull null]];
  } else {
    [stored_.get() addObject:[EAGLContext currentContext]];
  }
  return [EAGLContext setCurrentContext:context.get()];
}

void IOSGLContextGuardManager::PopContext() {
  EAGLContext* last = [stored_.get() lastObject];
  [stored_.get() removeLastObject];
  if ([last isEqual:[NSNull null]]) {
    [EAGLContext setCurrentContext:nil];
    return;
  }
  [EAGLContext setCurrentContext:last];
}

}

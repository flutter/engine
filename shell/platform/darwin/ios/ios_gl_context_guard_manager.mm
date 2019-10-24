// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ios_gl_context_guard_manager.h"


namespace flutter {

void IOSGLContextGuardManager::SetOtherContextToCurrent() {
  EAGLContext* last = [stored_.get() lastObject];
  [stored_.get() removeLastObject];
  if ([last isEqual:[NSNull null]]) {
    [EAGLContext setCurrentContext:nil];
    return;
  }
  [EAGLContext setCurrentContext:last];
}

void IOSGLContextGuardManager::SaveOtherContext() {
  if ([EAGLContext currentContext] == nil) {
    [stored_.get() addObject:[NSNull null]];
    return;
  }
  [stored_.get() addObject:[EAGLContext currentContext]];
}

void IOSGLContextGuardManager::SetFlutterContextToCurrent() {
  [EAGLContext setCurrentContext:flutter_gl_context_.get()];
}

void IOSGLContextGuardManager::SaveFlutterContext() {
}
}

// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/darwin/ios/ios_switchable_gl_context.h"
#import <OpenGLES/EAGL.h>

namespace flutter {

IOSSwitchableGLContext::IOSSwitchableGLContext(const EAGLContext& context) : context_(context){};

bool IOSSwitchableGLContext::SetCurrent() {
  FML_DCHECK_CREATION_THREAD_IS_CURRENT(checker_.checker);
  EAGLContext* current_context = EAGLContext.currentContext;
  previous_context_ = current_context;
  return [EAGLContext setCurrentContext:&context_];
};

bool IOSSwitchableGLContext::RemoveCurrent() {
  FML_DCHECK_CREATION_THREAD_IS_CURRENT(checker_.checker);
  return [EAGLContext setCurrentContext:previous_context_];
};
}

// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_DARWIN_IOS_IOS_SWITCHABLE_GL_CONTEXT_H_
#define FLUTTER_SHELL_PLATFORM_DARWIN_IOS_IOS_SWITCHABLE_GL_CONTEXT_H_

#include "flutter/fml/macros.h"
#include "flutter/fml/memory/thread_checker.h"
#include "flutter/shell/common/gl_context_switch.h"

@class EAGLContext;

namespace flutter {

struct DebugThreadChecker {
  FML_DECLARE_THREAD_CHECKER(checker);
};

class IOSSwitchableGLContext final : public SwitchableGLContext {
 public:
  IOSSwitchableGLContext(EAGLContext& context);

  bool SetCurrent() override;

  bool RemoveCurrent() override;

 private:
  EAGLContext& context_;
  // This pointer is managed by IOSRendererTarget/IOSContextGL or a 3rd party
  // plugin that uses gl context. |IOSSwitchableGLContext| should never out live
  // those objects. Never release this pointer within this object.
  EAGLContext* previous_context_;

  DebugThreadChecker checker_;

  FML_DISALLOW_COPY_AND_ASSIGN(IOSSwitchableGLContext);
};

}  // namespace flutter

#endif

// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_ANDROID_ANDROID_SWITCHABLE_GL_CONTEXT_H_
#define FLUTTER_SHELL_PLATFORM_ANDROID_ANDROID_SWITCHABLE_GL_CONTEXT_H_

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "flutter/flow/gl_context_switch.h"
#include "flutter/fml/macros.h"
#include "flutter/fml/memory/thread_checker.h"
#include "flutter/fml/memory/weak_ptr.h"
#include "flutter/shell/platform/android/android_environment_gl.h"

namespace flutter {

class AndroidSwitchableGLContext final : public SwitchableGLContext {
 public:
  AndroidSwitchableGLContext(EGLContext context,
                             EGLSurface surface,
                             EGLDisplay display,
                             fml::RefPtr<AndroidEnvironmentGL> environment);

  bool SetCurrent() override;

  bool RemoveCurrent() override;

 private:
  const EGLContext context_;
  EGLContext previous_context_;
  const EGLSurface surface_;
  const EGLDisplay display_;
  fml::RefPtr<AndroidEnvironmentGL> environment_;

  FML_DECLARE_THREAD_CHECKER(checker);

  FML_DISALLOW_COPY_AND_ASSIGN(AndroidSwitchableGLContext);
};

}  // namespace flutter

#endif

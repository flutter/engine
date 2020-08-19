// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/android/android_switchable_gl_context.h"

namespace flutter {

AndroidSwitchableGLContext::AndroidSwitchableGLContext(EGLContext context,
                                                       EGLSurface surface,
                                                       EGLDisplay display,
                                                       fml::RefPtr<AndroidEnvironmentGL> environment) :
                                                       context_(context),
                                                       surface_(surface),
                                                       display_(display),
                                                       environment_(environment){};

bool AndroidSwitchableGLContext::SetCurrent() {
  FML_DCHECK_CREATION_THREAD_IS_CURRENT(checker);
  FML_DCHECK(context_ != nullptr);
  FML_DCHECK(surface_ != nullptr);
  FML_DCHECK(display_ != nullptr);
  FML_DCHECK(environment_.get() != nullptr);
  EGLContext current_context = eglGetCurrentContext();
  previous_context_ = current_context;
  EGLBoolean result = eglMakeCurrent(display_, surface_, surface_, context_);
  return result == EGL_TRUE;
};

bool AndroidSwitchableGLContext::RemoveCurrent() {
  FML_DCHECK_CREATION_THREAD_IS_CURRENT(checker);
  EGLBoolean result;
  if (previous_context_ == context_) {
      result = eglMakeCurrent(display_, surface_, surface_, context_);
  }
  result = eglMakeCurrent(environment_->Display(), EGL_NO_SURFACE, EGL_NO_SURFACE,
                     EGL_NO_CONTEXT);
  return result == EGL_TRUE;
    return true;
};
}

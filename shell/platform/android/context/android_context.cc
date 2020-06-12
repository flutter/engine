// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/android/android_context.h"

#if OS_ANDROID
#include "flutter/shell/platform/android/android_context_gl.h"
#include "flutter/shell/platform/android/android_environment_gl.h"
#endif

namespace flutter {

AndroidContext::AndroidContext(AndroidRenderingAPI rendering_api)
    : rendering_api_(rendering_api) {}

AndroidContext::~AndroidContext() = default;

AndroidRenderingAPI AndroidContext::RenderingApi() const {
  return rendering_api_;
}

std::shared_ptr<AndroidContext> AndroidContext::Create(
    AndroidRenderingAPI rendering_api) {
#if OS_ANDROID
  if (rendering_api == AndroidRenderingAPI::kOpenGLES) {
    auto context_gl = std::make_shared<AndroidContextGL>(
        AndroidRenderingAPI::kOpenGLES,
        fml::MakeRefCounted<AndroidEnvironmentGL>());
    FML_CHECK(context_gl->IsValid())
        << "Could not create an Android context GL.";
    return context_gl;
  }
#endif
  return std::make_shared<AndroidContext>(rendering_api);
}

}  // namespace flutter

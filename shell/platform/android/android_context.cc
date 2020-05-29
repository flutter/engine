// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/android/android_context.h"

#include "flutter/shell/platform/android/android_context_gl.h"
#include "flutter/shell/platform/android/android_environment_gl.h"

namespace flutter {

AndroidContext::AndroidContext(AndroidRenderingAPI rendering_api)
    : rendering_api_(rendering_api) {}

AndroidContext::~AndroidContext() = default;

AndroidRenderingAPI AndroidContext::RenderingApi() {
  return rendering_api_;
}

bool AndroidContext::IsValid() {
  return true;
}

std::shared_ptr<AndroidContext> AndroidContext::Create(
    AndroidRenderingAPI rendering_api) {
  std::shared_ptr<AndroidContext> context;
  switch (rendering_api) {
    case AndroidRenderingAPI::kSoftware:
      context = std::make_shared<AndroidContext>(rendering_api);
      break;
    case AndroidRenderingAPI::kOpenGLES:
      context = std::make_shared<AndroidContextGL>(
          rendering_api, fml::MakeRefCounted<AndroidEnvironmentGL>());
      break;
    case AndroidRenderingAPI::kVulkan:
      context = std::make_shared<AndroidContext>(rendering_api);
      break;
  }
  FML_CHECK(context);
  FML_CHECK(context->IsValid()) << "Could not create an Android context.";
  return nullptr;
}

}  // namespace flutter

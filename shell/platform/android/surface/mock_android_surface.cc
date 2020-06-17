// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/android/surface/mock_android_surface.h"

namespace flutter {

std::unique_ptr<GLContextResult> MockAndroidSurface::GLContextMakeCurrent() {
  return std::make_unique<GLContextDefaultResult>(/*static_result=*/true);
}

bool MockAndroidSurface::GLContextClearCurrent() {
  return true;
}

bool MockAndroidSurface::GLContextPresent() {
  return true;
}

intptr_t MockAndroidSurface::GLContextFBO() const {
  return 0;
}

ExternalViewEmbedder* MockAndroidSurface::GetExternalViewEmbedder() {
  return nullptr;
}

}  // namespace flutter

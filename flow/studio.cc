// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/studio.h"

namespace flutter {

Studio::Studio() = default;

Studio::~Studio() = default;

std::unique_ptr<GLContextResult> Studio::MakeRenderContextCurrent() {
  return std::make_unique<GLContextDefaultResult>(true);
}

bool Studio::ClearRenderContext() {
  return false;
}

bool Studio::AllowsDrawingWhenGpuDisabled() const {
  return true;
}

bool Studio::EnableRasterCache() const {
  return true;
}

std::shared_ptr<impeller::AiksContext> Studio::GetAiksContext() const {
  return nullptr;
}

}  // namespace flutter

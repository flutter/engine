// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/toolkit/android/surface_control.h"

namespace impeller::android {

SurfaceControl::SurfaceControl(ANativeWindow* window, const char* debug_name)
    : control_(GetProcTable().ASurfaceControl_createFromWindow(
          window,
          debug_name == nullptr ? "Impeller Layer" : debug_name)) {}

SurfaceControl::~SurfaceControl() = default;

bool SurfaceControl::IsValid() const {
  return control_.is_valid();
}

ASurfaceControl* SurfaceControl::GetHandle() const {
  return control_.get();
}

}  // namespace impeller::android

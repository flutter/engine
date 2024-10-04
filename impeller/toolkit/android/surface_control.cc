// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/toolkit/android/surface_control.h"

#include "impeller/base/validation.h"
#include "impeller/toolkit/android/proc_table.h"
#include "impeller/toolkit/android/surface_transaction.h"

namespace impeller::android {

SurfaceControl::SurfaceControl(ANativeWindow* window, const char* debug_name) {
  if (window == nullptr) {
    VALIDATION_LOG << "Parent window of surface was null.";
    return;
  }
  if (debug_name == nullptr) {
    debug_name = "Impeller Layer";
  }
  control_.reset(
      GetProcTable().ASurfaceControl_createFromWindow(window, debug_name));
}

SurfaceControl::~SurfaceControl() {
  if (IsValid() && !RemoveFromParent()) {
    VALIDATION_LOG << "Surface control could not be removed from its parent. "
                      "Expect a leak.";
  }
}

bool SurfaceControl::IsValid() const {
  return control_.is_valid();
}

ASurfaceControl* SurfaceControl::GetHandle() const {
  return control_.get();
}

bool SurfaceControl::RemoveFromParent() const {
  if (!IsValid()) {
    return false;
  }
  SurfaceTransaction transaction;
  if (!transaction.SetParent(*this, nullptr)) {
    return false;
  }
  return transaction.Apply();
}

bool SurfaceControl::IsAvailableOnPlatform() {
  // We must check all procs used by the AHB swapchain as some Android 29
  // devices do not correctly expose all required APIs. See:
  // https://github.com/flutter/flutter/issues/155877
  const ProcTable& proc_table = GetProcTable();
  return proc_table.IsValid() &&
         proc_table.ASurfaceControl_createFromWindow.IsAvailable() &&
         proc_table.ASurfaceControl_release.IsAvailable() &&
         proc_table.ASurfaceTransaction_apply.IsAvailable() &&
         proc_table.ASurfaceTransaction_create.IsAvailable() &&
         proc_table.ASurfaceTransaction_delete.IsAvailable() &&
         proc_table.ASurfaceTransaction_reparent.IsAvailable() &&
         proc_table.ASurfaceTransaction_setBuffer.IsAvailable() &&
         proc_table.ASurfaceTransaction_setOnComplete.IsAvailable() &&
         proc_table.ASurfaceTransactionStats_getPreviousReleaseFenceFd
             .IsAvailable();
}

}  // namespace impeller::android

// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/fml/logging.h"
#if FML_OS_ANDROID
#include "flutter/fml/platform/android/ndk_helpers.h"
#endif  // FML_OS_ANDROID
#include "flutter/common/constants.h"
#include "flutter/shell/platform/android/surface/android_surface_transaction.h"

namespace flutter {

AndroidSurfaceTransaction& AndroidSurfaceTransaction::GetInstance() {
  static AndroidSurfaceTransactionImpl instance;
  return instance;
}

void AndroidSurfaceTransactionImpl::Begin() {
#if FML_OS_ANDROID
  FML_DCHECK(!pending_transaction_);
  if (NDKHelpers::SurfaceControlAndTransactionSupported()) {
    pending_transaction_ = NDKHelpers::ASurfaceTransaction_create();
  }
#endif  // FML_OS_ANDROID
}

void AndroidSurfaceTransactionImpl::SetVsyncId(int64_t vsync_id) {
#if FML_OS_ANDROID
  if (NDKHelpers::SurfaceControlAndTransactionSupported()) {
    FML_DCHECK(pending_transaction_);
    if (vsync_id != kInvalidVSyncId) {
      NDKHelpers::ASurfaceTransaction_setFrameTimeline(pending_transaction_,
                                                       vsync_id);
    }
  }
#endif  // FML_OS_ANDROID
}

void AndroidSurfaceTransactionImpl::End() {
#if FML_OS_ANDROID
  if (NDKHelpers::SurfaceControlAndTransactionSupported()) {
    FML_DCHECK(pending_transaction_);
    NDKHelpers::ASurfaceTransaction_apply(pending_transaction_);
    NDKHelpers::ASurfaceTransaction_delete(pending_transaction_);
    pending_transaction_ = nullptr;
  }
#endif  // FML_OS_ANDROID
}

}  // namespace flutter

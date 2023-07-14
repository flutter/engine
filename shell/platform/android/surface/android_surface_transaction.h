// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_ANDROID_SURFACE_ANDROID_SURFACE_TRANSACTION_H_
#define FLUTTER_SHELL_PLATFORM_ANDROID_SURFACE_ANDROID_SURFACE_TRANSACTION_H_

#include "flutter/fml/macros.h"

#ifdef FML_OS_ANDROID
#include <android/surface_control.h>
#endif  // FML_OS_ANDROID

#include <cstdint>

namespace flutter {

//------------------------------------------------------------------------------
/// The Android SurfaceTransaction API is used by `AndroidSurface` to
/// synchronize frame presentation times. It's only available on API 29+.
///
class AndroidSurfaceTransaction {
 public:
  static AndroidSurfaceTransaction& GetInstance();
  virtual void Begin() = 0;
  virtual void SetVsyncId(int64_t vsync_id) = 0;
  virtual void End() = 0;
};

class AndroidSurfaceTransactionImpl : public AndroidSurfaceTransaction {
 public:
  void Begin() override;
  void SetVsyncId(int64_t vsync_id) override;
  void End() override;

 private:
  AndroidSurfaceTransactionImpl(){};

#if FML_OS_ANDROID
  ASurfaceTransaction* pending_transaction_ = nullptr;
#endif  // FML_OS_ANDROID

  friend class AndroidSurfaceTransaction;
  FML_DISALLOW_COPY_AND_ASSIGN(AndroidSurfaceTransactionImpl);
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_ANDROID_SURFACE_ANDROID_SURFACE_TRANSACTION_H_

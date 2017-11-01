// Copyright 2017 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_RUNTIME_DART_VM_H_
#define FLUTTER_RUNTIME_DART_VM_H_

#include <functional>
#include <string>
#include <vector>

#include "flutter/common/settings.h"
#include "flutter/runtime/dart_isolate.h"
#include "flutter/runtime/dart_snapshot.h"
#include "lib/fxl/build_config.h"
#include "lib/fxl/functional/closure.h"
#include "lib/fxl/macros.h"
#include "lib/fxl/memory/ref_counted.h"
#include "lib/fxl/memory/ref_ptr.h"
#include "lib/fxl/memory/weak_ptr.h"
#include "third_party/dart/runtime/include/dart_api.h"

namespace blink {

class DartVM : public fxl::RefCountedThreadSafe<DartVM> {
 public:
  class PlatformKernel;

  FXL_WARN_UNUSED_RESULT
  static fxl::RefPtr<DartVM> ForProcess(Settings settings);

  static fxl::RefPtr<DartVM> ForProcessIfInitialized();

  // Name of the kernel blob asset within the FLX bundle.
  static const char kKernelAssetKey[];

  // Name of the snapshot blob asset within the FLX bundle.
  static const char kSnapshotAssetKey[];

  // Name of the platform kernel blob asset within the FLX bundle.
  static const char kPlatformKernelAssetKey[];

  static bool IsRunningPrecompiledCode();

  const Settings& GetSettings() const;

  PlatformKernel* GetPlatformKernel() const;

  const DartSnapshot& GetDartSnapshot() const;

  fxl::WeakPtr<DartVM> GetWeakPtr();

 private:
  const Settings settings_;
  const std::unique_ptr<DartSnapshot> dart_snapshot_;
  std::vector<uint8_t> platform_kernel_data_;
  PlatformKernel* platform_kernel_ = nullptr;
  fxl::WeakPtrFactory<DartVM> weak_factory_;

  DartVM(const Settings& settings, std::unique_ptr<DartSnapshot> shapshot);

  ~DartVM();

  FRIEND_REF_COUNTED_THREAD_SAFE(DartVM);
  FRIEND_MAKE_REF_COUNTED(DartVM);
  FXL_DISALLOW_COPY_AND_ASSIGN(DartVM);
};

}  // namespace blink

#endif  // FLUTTER_RUNTIME_DART_VM_H_

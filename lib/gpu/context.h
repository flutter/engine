// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "dart_api.h"
#include "flutter/lib/gpu/export.h"
#include "flutter/lib/ui/dart_wrapper.h"
#include "impeller/renderer/context.h"

namespace flutter {

class GpuContext : public RefCountedDartWrappable<GpuContext> {
  DEFINE_WRAPPERTYPEINFO();
  FML_FRIEND_MAKE_REF_COUNTED(GpuContext);

 public:
  explicit GpuContext(std::shared_ptr<impeller::Context> context);
  ~GpuContext() override;

 private:
  std::shared_ptr<impeller::Context> context_;

  FML_DISALLOW_COPY_AND_ASSIGN(GpuContext);
};

}  // namespace flutter

//----------------------------------------------------------------------------
/// Exports
///

extern "C" {

FLUTTER_GPU_EXPORT
extern Dart_Handle InternalFlutterGpu_GpuContext_InitializeDefault(
    Dart_Handle wrapper);

}  // extern "C"

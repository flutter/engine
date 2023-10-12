// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <Metal/Metal.h>

#include <memory>
#include <optional>

namespace impeller {

class ContextMTL;

class GPUTracerMTL {
 public:
  explicit GPUTracerMTL(const std::weak_ptr<ContextMTL>& context);

  ~GPUTracerMTL() = default;

  /// @brief Record the approximate start time of the GPU workload for the
  ///        current frame.
  void RecordStartFrameTime();

  /// @brief Record the approximate end time of the GPU workload for the current
  ///        frame.
  void RecordEndFrameTime();

 private:
  std::weak_ptr<ContextMTL> context_;
  CFTimeInterval pending_interval_;
};

}  // namespace impeller
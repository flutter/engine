// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <thread>
#include "impeller/renderer/backend/gles/proc_table_gles.h"

namespace impeller {

class GPUTracerGLES {
 public:
  explicit GPUTracerGLES(const ProcTableGLES& gl);

  ~GPUTracerGLES() = default;

  bool HasStartedFrame() const {
    return has_started_frame_;
  }

  void MarkFrameStart(const ProcTableGLES& gl);

  void MarkFrameEnd(const ProcTableGLES& gl);

 private:
  std::vector<uint32_t> pending_traces_;
  std::optional<uint32_t> active_frame_ = std::nullopt;
  std::thread::id raster_thread_;
  bool has_started_frame_ = false;

  bool enabled_ = false;
};

}  // namespace impeller

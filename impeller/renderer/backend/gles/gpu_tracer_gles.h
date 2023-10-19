// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <deque>
#include <thread>

#include "impeller/renderer/backend/gles/proc_table_gles.h"

namespace impeller {

class GPUTracerGLES {
 public:
  explicit GPUTracerGLES(const ProcTableGLES& gl);

  ~GPUTracerGLES() = default;

  /// @brief Record the thread id of the raster thread.
  void RecordRasterThread();

  /// @brief Record the start of a frame workload, if one hasn't already been
  ///        started.
  void MarkFrameStart(const ProcTableGLES& gl);

  /// @brief Record the end of a frame workload.
  void MarkFrameEnd(const ProcTableGLES& gl);

 private:
  void ProcessQueries(const ProcTableGLES& gl);

  std::deque<uint32_t> pending_traces_;
  std::optional<uint32_t> active_frame_ = std::nullopt;
  std::thread::id raster_thread_;
  bool has_started_frame_ = false;

  bool enabled_ = false;
};

}  // namespace impeller

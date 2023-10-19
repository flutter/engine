// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/gles/gpu_tracer_gles.h"
#include <thread>
#include "fml/trace_event.h"

namespace impeller {

GPUTracerGLES::GPUTracerGLES(const ProcTableGLES& gl) {
#ifdef IMPELLER_DEBUG
  auto desc = gl.GetDescription();
  enabled_ = desc->HasExtension("GL_EXT_disjoint_timer_query");
#endif  // IMPELLER_DEBUG
}

void GPUTracerGLES::MarkFrameStart(const ProcTableGLES& gl) {
  if (!enabled_ || has_started_frame_ ||
      std::this_thread::get_id() != raster_thread_) {
    return;
  }

  // At the beginning of a frame, check the status of all pending
  // previous queries.
  ProcessQueries(gl);

  uint32_t query = 0;
  gl.GenQueriesEXT(1, &query);
  if (query == 0) {
    return;
  }

  has_started_frame_ = true;

  FML_DCHECK(!active_frame_.has_value());
  FML_DCHECK(query != 0);
  active_frame_ = query;
  gl.BeginQueryEXT(GL_TIME_ELAPSED_EXT, query);
}

void GPUTracerGLES::RecordRasterThread() {
  raster_thread_ = std::this_thread::get_id();
}

void GPUTracerGLES::ProcessQueries(const ProcTableGLES& gl) {
  if (pending_traces_.empty()) {
    return;
  }

  // For reasons unknown to me, querying the state of more than
  // on query object per frame causes crashes on a Pixel 6 pro.
  auto latest_query = pending_traces_.front();

  // First check if the query is complete without blocking
  // on the result. Incomplete results are left in the pending
  // trace vector and will not be checked again for another
  // frame.
  GLuint available = GL_FALSE;
  gl.GetQueryObjectuivEXT(latest_query, GL_QUERY_RESULT_AVAILABLE_EXT,
                          &available);

  if (available == GL_TRUE) {
    // Return the timer resolution in nanoseconds.
    uint64_t duration = 0;
    gl.GetQueryObjectui64vEXT(latest_query, GL_QUERY_RESULT_EXT, &duration);
    auto gpu_ms = duration / 1000000.0;

    FML_TRACE_COUNTER("flutter", "GPUTracer",
                      reinterpret_cast<int64_t>(this),  // Trace Counter ID
                      "FrameTimeMS", gpu_ms);

    gl.DeleteQueriesEXT(1, &latest_query);
    pending_traces_.pop_front();
  }
}

void GPUTracerGLES::MarkFrameEnd(const ProcTableGLES& gl) {
  if (!enabled_ || std::this_thread::get_id() != raster_thread_ ||
      !active_frame_.has_value() || !has_started_frame_) {
    return;
  }

  auto query = active_frame_.value();
  gl.EndQueryEXT(GL_TIME_ELAPSED_EXT);

  pending_traces_.push_back(query);
  active_frame_ = std::nullopt;
  has_started_frame_ = false;
}

}  // namespace impeller

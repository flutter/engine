// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/gles/gpu_tracer_gles.h"
#include "GLES2/gl2ext.h"
#include "impeller/renderer/backend/gles/context_gles.h"

namespace impeller {

static const constexpr char* kAngleTimerQuery = "GL_ANGLE_timer_query";
static const constexpr char* kExtTimerQuery = "GL_EXT_timer_query";
static const constexpr char* kExtDisjointTimerQuery =
    "GL_EXT_disjoint_timer_query";

GPUTracerGLES::GPUTracerGLES(const ProcTableGLES& gl) {
#ifdef IMPELLER_DEBUG
  auto desc = gl.GetDescription();
  enabled_ = desc->HasExtension(kAngleTimerQuery) ||
             desc->HasExtension(kExtTimerQuery) ||
             desc->HasExtension(kExtDisjointTimerQuery);
#endif  // IMPELLER_DEBUG
}

void GPUTracerGLES::MarkFrameStart(const ProcTableGLES& gl) {
  if (!enabled_) {
    return;
  }
  has_started_frame_ = true;
  if (queries_.empty()) {
    queries_ = std::vector<uint32_t>(32);
    gl.GenQueriesEXT(32, queries_.data());
  }

  // At the beginning of a frame, check the status of all pending
  // previous queries.
  if (!pending_traces_.empty()) {
    std::vector<uint32_t> pending_traces = pending_traces_;
    pending_traces_.clear();

    for (auto query : pending_traces) {
      // First check if the query is complete without blocking
      // on the result. Incomplete results are left in the pending
      // trace vector and will not be checked again for another
      // frame.
      uint available = 0;
      FML_LOG(ERROR) << "Checking: " << query;
      gl.GetQueryObjectuivEXT(query, GL_QUERY_RESULT_AVAILABLE_EXT, &available);

      if (available) {
        // Return the timer resolution in nanoseconds.
        uint64_t duration = 0;
        gl.GetQueryObjectui64vEXT(query, GL_QUERY_RESULT, &duration);
        auto gpu_ms = duration / 1000000.0;
        FML_LOG(ERROR) << "gpu_ms: " << gpu_ms;
      } else {
        pending_traces_.push_back(query);
      }
    }
  }

  // Allocate a single query object for the start and end of the frame.
  FML_CHECK(!active_frame_.has_value());
  uint32_t query = queries_.back();
  queries_.pop_back();
  FML_CHECK(query != 0);
  active_frame_ = query;
  FML_LOG(ERROR) << "query: " << query;
  gl.BeginQueryEXT(GL_TIME_ELAPSED_EXT, query);
}

void GPUTracerGLES::MarkFrameEnd(const ProcTableGLES& gl) {
  if (!enabled_ || !active_frame_.has_value()) {
    return;
  }
  auto query = active_frame_.value();
  FML_LOG(ERROR) << "END " << query;
  gl.EndQueryEXT(GL_TIME_ELAPSED_EXT);

  pending_traces_.push_back(query);
  active_frame_ = std::nullopt;
  has_started_frame_ = false;
}

}  // namespace impeller

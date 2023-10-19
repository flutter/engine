// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/gles/gpu_tracer_gles.h"
#include "GLES2/gl2ext.h"
#include "GLES3/gl3.h"

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
  if (!enabled_ || has_started_frame_) {
    return;
  }
  std::stringstream ss;
  ss << std::this_thread::get_id();
  uint64_t id = std::stoull(ss.str());

  FML_LOG(ERROR) << "Start: " << id;

  uint32_t query = 0;
  gl.GenQueriesEXT(1, &query);
  if (query == 0) {
    return;
  }

  has_started_frame_ = true;

  // At the beginning of a frame, check the status of all pending
  // previous queries.
  if (!pending_traces_.empty()) {
    std::vector<uint32_t> pending_traces = pending_traces_;
    pending_traces_.clear();

    bool done = false;
    for (auto query : pending_traces) {
      if (done) {
        pending_traces_.push_back(query);
        continue;
      }
      // First check if the query is complete without blocking
      // on the result. Incomplete results are left in the pending
      // trace vector and will not be checked again for another
      // frame.
      GLuint available = GL_FALSE;
      gl.GetQueryObjectuivEXT(query, GL_QUERY_RESULT_AVAILABLE_EXT, &available);

      if (available == GL_TRUE) {
        // Return the timer resolution in nanoseconds.
        uint64_t duration = 0;
        gl.GetQueryObjectui64vEXT(query, GL_QUERY_RESULT_EXT, &duration);
        auto gpu_ms = duration / 1000000.0;
        FML_LOG(ERROR) << "gpu_ms: " << gpu_ms;
        gl.DeleteQueriesEXT(1, &query);
        // Only ever check for one query a frame...
        done = true;
      } else {
        done = true;
        pending_traces_.push_back(query);
      }
    }
  }

  // Allocate a single query object for the start and end of the frame.
  FML_CHECK(!active_frame_.has_value());
  FML_CHECK(query != 0);
  active_frame_ = query;
  gl.BeginQueryEXT(GL_TIME_ELAPSED_EXT, query);
}

void GPUTracerGLES::MarkFrameEnd(const ProcTableGLES& gl) {
  if (!enabled_ || !active_frame_.has_value() || !has_started_frame_){
    return;
  }

  std::stringstream ss;
  ss << std::this_thread::get_id();
  uint64_t id = std::stoull(ss.str());

  FML_LOG(ERROR) << "End: " << id;

  auto query = active_frame_.value();
  gl.EndQueryEXT(GL_TIME_ELAPSED_EXT);

  pending_traces_.push_back(query);
  active_frame_ = std::nullopt;
  has_started_frame_ = false;
}

}  // namespace impeller

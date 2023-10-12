// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <Metal/Metal.h>
#include "fml/trace_event.h"
#include "impeller/renderer/backend/metal/context_mtl.h"
#include "impeller/renderer/backend/metal/formats_mtl.h"

#include <memory>

#include "impeller/renderer/backend/metal/gpu_tracer_mtl.h"

namespace impeller {

GPUTracerMTL::GPUTracerMTL(const std::weak_ptr<ContextMTL>& context)
    : context_(context) {}

void GPUTracerMTL::RecordStartFrameTime() {
  if (@available(ios 10.3, tvos 10.2, macos 10.15, macCatalyst 13.0, *)) {
    auto strong_context = context_.lock();
    if (!strong_context) {
      return;
    }
    auto cmd_buffer = strong_context->CreateMTLCommandBuffer("FrameStartTime");
    [cmd_buffer addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
      pending_interval_ = buffer.GPUStartTime;
    }];
    [cmd_buffer commit];
  }
}

void GPUTracerMTL::RecordEndFrameTime() {
  if (@available(ios 10.3, tvos 10.2, macos 10.15, macCatalyst 13.0, *)) {
    auto strong_context = context_.lock();
    if (!strong_context) {
      return;
    }
    auto cmd_buffer = strong_context->CreateMTLCommandBuffer("FrameEndTime");
    [cmd_buffer addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
      auto end = buffer.GPUEndTime;
      // Convert deltas in seconds into milliseconds.
      auto gpu_ms = (end - pending_interval_) * 1000;
      FML_TRACE_COUNTER("flutter", "GPUTracer",
                        1234,  // Trace Counter ID
                        "FrameTimeMS", gpu_ms);
    }];
    [cmd_buffer commit];
  }
}

}  // namespace impeller
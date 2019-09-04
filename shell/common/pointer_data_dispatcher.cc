// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/common/pointer_data_dispatcher.h"

namespace flutter {

void DefaultPointerDataDispatcher::DispatchPacket(
    std::unique_ptr<PointerDataPacket> packet,
    uint64_t trace_flow_id) {
  animator_.EnqueueTraceFlowId(trace_flow_id);
  runtime_controller_.DispatchPointerDataPacket(*packet);
}

void IosPointerDataDispatcher::DispatchPacket(
    std::unique_ptr<PointerDataPacket> packet,
    uint64_t trace_flow_id) {
  // Fix for https://github.com/flutter/flutter/issues/31086
  //
  // If a pointer data dispatch is still in progress (its frame isn't finished
  // yet), entering this function means that an input event is delivered to us
  // too fast. That potentially means a later event will be too late which could
  // cause the missing of a frame. Hence we'll cache it in `pending_packet_` for
  // the next frame to smooth it out.
  //
  // If the input event is sent to us regularly at the same rate of VSYNC (say
  // at 60Hz, once per 16ms), this would be identical to the non-ios cases where
  // `runtime_controller_->DispatchPointerDataPacket` is always called right
  // away. That's because `is_pointer_data_in_progress_` will always be false at
  // this point since it will be cleared by the end of a frame through
  // `Engine::Render`. This is the case for all Android/iOS devices before
  // iPhone X/XS.
  //
  // If the input event is irregular, but with a random latency of no more than
  // one frame, this would guarantee that we'll miss at most 1 frame. Without
  // this, we could miss half of the frames.
  //
  // If the input event is delivered at a higher rate than that of VSYNC, this
  // would at most add a latency of one event delivery. For example, if the
  // input event is delivered at 120Hz (this is only true for iPad pro, not even
  // iPhone X), this may delay the handling of an input event by 8ms.
  //
  // The assumption of this solution is that the sampling itself is still
  // regular. Only the event delivery is allowed to be irregular. So far this
  // assumption seems to hold on all devices. If it's changed in the future,
  // we'll need a different solution.
  //
  // See also input_events_unittests.cc where we test all our claims above.
  if (is_pointer_data_in_progress_) {
    if (pending_packet_ != nullptr) {
      DispatchPendingPacket();
    }
    pending_packet_ = std::move(packet);
    pending_trace_flow_id_ = trace_flow_id;
  } else {
    FML_DCHECK(pending_packet_ == nullptr);
    DefaultPointerDataDispatcher::DispatchPacket(std::move(packet),
                                                 trace_flow_id);
  }
  is_pointer_data_in_progress_ = true;
}

void IosPointerDataDispatcher::OnRender() {
  if (is_pointer_data_in_progress_) {
    if (pending_packet_ != nullptr) {
      // This is already in the UI thread. However, method `OnRender` is called
      // by `Engine::Render` (a part of the `VSYNC` UI thread task) which is in
      // Flutter framework's `SchedulerPhase.persistentCallbacks` phase. In that
      // phase, no pointer data packet is allowed to be fired because the
      // framework requires such phase to be executed synchronously without
      // being interrupted. Hence we'll post a new UI thread task to fire the
      // packet after `VSYNC` task is done. When a non-VSYNC UI thread task
      // (like the following one) is run, the Flutter framework is always in
      // `SchedulerPhase.idle` phase).
      task_runners_.GetUITaskRunner()->PostTask(
          // Use and validate a `fml::WeakPtr` because this dispatcher might
          // have been destructed with engine when the task is run.
          [dispatcher = weak_factory_.GetWeakPtr()]() {
            if (dispatcher) {
              dispatcher->DispatchPendingPacket();
            }
          });
    } else {
      is_pointer_data_in_progress_ = false;
    }
  }
}

void IosPointerDataDispatcher::DispatchPendingPacket() {
  FML_DCHECK(pending_packet_ != nullptr);
  FML_DCHECK(is_pointer_data_in_progress_);
  DefaultPointerDataDispatcher::DispatchPacket(std::move(pending_packet_),
                                               pending_trace_flow_id_);
  pending_packet_ = nullptr;
  pending_trace_flow_id_ = -1;
}

}  // namespace flutter

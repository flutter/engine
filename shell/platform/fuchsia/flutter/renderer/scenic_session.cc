// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/fuchsia/flutter/renderer/scenic_session.h"

#include <fuchsia/images/cpp/fidl.h>
#include <fuchsia/ui/scenic/cpp/fidl.h>
#include <lib/trace/event.h>
#include <zircon/status.h>
#include <zircon/types.h>

#include "flutter/shell/platform/embedder/embedder.h"

namespace flutter_runner {
namespace {

/// Returns the system time at which the next frame is likely to be presented.
///
/// Consider the following scenarios, where in both the
/// scenarious the result will be the same.
///
/// Scenario 1:
/// presentation_interval is 2
/// ^     ^     ^     ^     ^
/// +     +     +     +     +
/// 0--1--2--3--4--5--6--7--8--9--
/// +        +  +
/// |        |  +---------> result: next_presentation_time
/// |        v
/// v        now
/// last_presentation_time
///
/// Scenario 2:
/// presentation_interval is 2
/// ^     ^     ^     ^     ^
/// +     +     +     +     +
/// 0--1--2--3--4--5--6--7--8--9--
///       +  +  +
///       |  |  +--------->result: next_presentation_time
///       |  |
///       |  +>now
///       |
///       +->last_presentation_time
std::pair<uint64_t, uint64_t> SnapToNextPhase(
    const uint64_t now,
    const uint64_t last_presentation_time,
    const uint64_t presentation_interval) {
  if (last_presentation_time > now) {
    FX_LOG(ERROR) << "Last frame was presented in the future ("
                  << last_presentation_time << "). Clamping to now (" << now
                  << ").";
    return std::make_pair(now, now + presentation_interval);
  }

  const uint64_t time_since_last_presentation = now - last_presentation_time;
  if (time_since_last_presentation < presentation_interval) {
    // This will be the most likely scenario if we are rendering at a good
    // frame-rate; short circuiting the other checks in this case.
    return std::make_pair(time_since_last_presentation,
                          time_since_last_presentation + presentation_interval);
  } else {
    const uint64_t num_phases_passed =
        (time_since_last_presentation / presentation_interval);
    const uint64_t predicted_presentation_time =
        last_presentation_time +
        (presentation_interval * (num_phases_passed + 1));

    return std::make_pair(predicted_presentation_time - presentation_interval,
                          predicted_presentation_time);
  }
}

}  // end namespace

void ScenicSession::Connect(const Renderer::Context& context,
                            EventCallback session_event_callback) {
  FX_DCHECK(!connected());

  auto scenic =
      context.incoming_services->Connect<fuchsia::ui::scenic::Scenic>();
  session_.emplace(scenic::CreateScenicSessionPtrAndListenerRequest(
                       scenic.get(), context.raster_dispatcher),
                   context.input_dispatcher);
  session_->set_error_handler(
      [this, error_callback =
                 context.dispatch_table.error_callback](zx_status_t status) {
        session_connected_ = false;

        FX_LOG(ERROR) << "Interface error for fuchsia::ui::scenic::Session: "
                      << zx_status_get_string(status);
        error_callback();
      });
  session_->set_event_handler(session_event_callback);
  session_->SetDebugName(context.debug_label);
  get_current_time_callback_ = context.dispatch_table.get_current_time_callback;
  vsync_callback_ = context.dispatch_table.vsync_callback;
  session_connected_ = true;
}

void ScenicSession::AwaitVsync(intptr_t baton) {
  FX_LOG(ERROR) << "ScenicSession::AwaitPresent for " << baton;

  // Only one |AwaitPresent| call should be outstanding at a time.
  intptr_t old_baton = pending_present_baton_.exchange(baton);
  FX_DCHECK(old_baton == 0);

  if (session_connected_ && !presentation_callback_pending_ &&
      !present_session_pending_) {
    TriggerVsyncImmediately();
  }
}

void ScenicSession::TriggerVsyncImmediately() {
  const uint64_t now = get_current_time_callback_();

  FireVsyncCallback(fuchsia::images::PresentationInfo{now, now}, now);
}

void ScenicSession::QueuePresent() {
  TRACE_DURATION("gfx", "ScenicSession::QueuePresent");
  TRACE_FLOW_BEGIN("gfx", "ScenicSession::QueuePresent",
                   next_present_session_trace_id_);
  next_present_session_trace_id_++;

  // Throttle vsync if presentation callback is already pending. This allows
  // the paint tasks for this frame to execute in parallel with presentation
  // of last frame but still provides back-pressure to prevent us from
  // queuing even more work.
  if (presentation_callback_pending_) {
    present_session_pending_ = true;
  } else {
    Present();
  }
}

void ScenicSession::Present() {
  TRACE_DURATION("gfx", "ScenicSession::Present");
  while (processed_present_session_trace_id_ < next_present_session_trace_id_) {
    TRACE_FLOW_END("gfx", "ScenicSession::QueuePresent",
                   processed_present_session_trace_id_);
    processed_present_session_trace_id_++;
  }
  TRACE_FLOW_BEGIN("gfx", "Session::Present", next_present_trace_id_);
  next_present_trace_id_++;

  FX_DCHECK(connected());

  // Presentation callback is pending as a result of Present() call below.
  // Flush all session ops.
  presentation_callback_pending_ = true;
  session_->Present(
      0,  // presentation_time. (placeholder).
      [this](fuchsia::images::PresentationInfo presentation_info) {
        presentation_callback_pending_ = false;

        // Process pending Present() calls by queueing another Session Present.
        if (present_session_pending_) {
          present_session_pending_ = false;
          Present();
        }

        // Notify the embedder of the vsync.
        const uint64_t now = get_current_time_callback_();
        FireVsyncCallback(std::move(presentation_info), now);
      }  // callback
  );
}

void ScenicSession::FireVsyncCallback(
    fuchsia::images::PresentationInfo presentation_info,
    uint64_t now) {
  intptr_t baton = pending_present_baton_.exchange(0);
  if (baton != 0) {
    FX_LOG(ERROR) << "ScenicSession::FireVsyncCallback for " << baton;
    auto [previous_vsync, next_vsync] =
        SnapToNextPhase(now, presentation_info.presentation_time,
                        presentation_info.presentation_interval);

    vsync_callback_(baton, previous_vsync, next_vsync);
  } else {
    FX_LOG(ERROR) << "ScenicSession::FireVsyncCallback MISSED, baton was 0";
  }
}

}  // namespace flutter_runner

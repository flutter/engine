// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/fuchsia/flutter/compositor/scenic_session.h"

#include <zircon/errors.h>
#include <zircon/types.h>

#include "flutter/fml/logging.h"
#include "flutter/fml/time/time_point.h"
#include "flutter/fml/trace_event.h"

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
std::pair<int64_t, int64_t> SnapToNextPhase(
    const int64_t now,
    const int64_t last_frame_presentation_time,
    const int64_t presentation_interval) {
  if (presentation_interval <= 0) {
    FML_LOG(ERROR) << "Presentation interval must be positive. The value was: "
                   << presentation_interval / 1000000 << "ms.";
    return std::make_pair(now, now);
  }

  if (last_frame_presentation_time >= now) {
    FML_LOG(ERROR)
        << "Last frame was presented in the future. Clamping to now.";
    return std::make_pair(now, now + presentation_interval);
  }

  const int64_t time_since_last_presentation =
      now - last_frame_presentation_time;

  // this will be the most likely scenario if we are rendering at a good
  // frame rate, short circuiting the other checks in this case.
  if (time_since_last_presentation < presentation_interval) {
    return std::make_pair(time_since_last_presentation,
                          time_since_last_presentation + presentation_interval);
  } else {
    const int64_t num_phases_passed =
        (time_since_last_presentation / presentation_interval);
    const int64_t predicted_presentation_time =
        last_frame_presentation_time +
        (presentation_interval * (num_phases_passed + 1));

    return std::make_pair(predicted_presentation_time - presentation_interval,
                          predicted_presentation_time);
  }
}

}  // end namespace

ScenicSession::ScenicSession(const std::string& debug_label,
                             FlutterEngine& bound_engine,
                             SessionEventCallback session_event_callback,
                             SessionErrorCallback session_error_callback,
                             fuchsia::ui::scenic::Scenic* scenic)
    : flutter_engine_(bound_engine), session_(scenic) {
  session_.set_error_handler(session_error_callback);
  session_.set_event_handler(session_event_callback);
  session_.SetDebugName(debug_label);
  session_.Unbind();  // Unbind the SessionPtr so it can be transferred to
                      // another thread.  There is no way to maniulate the
                      // SessionListener in this way at the moment.
}

ScenicSession::~ScenicSession() = default;

void ScenicSession::AwaitPresent(intptr_t baton) {
  FML_DCHECK(flutter_engine_);
  // Only one |AwaitPresent| call can be outstanding at a time.
  FML_DCHECK(pending_present_baton_ == 0);

  pending_present_baton_ = baton;
}

void ScenicSession::QueuePresent() {
  TRACE_EVENT0("gfx", "ScenicSession::QueuePresent");
  TRACE_FLOW_BEGIN("gfx", "ScenicSession::QueuePresent",
                   next_present_session_trace_id_);
  next_present_session_trace_id_++;

  // Throttle vsync if presentation callback is already pending. This allows
  // the paint tasks for this frame to execute in parallel with presentation
  // of last frame but still provides back-pressure to prevent us from queuing
  // even more work.
  if (presentation_callback_pending_) {
    present_session_pending_ = true;
  } else {
    Present();
  }
}

void ScenicSession::FireVsyncCallback(
    fuchsia::images::PresentationInfo presentation_info,
    intptr_t baton) {
  FML_DCHECK(flutter_engine_);

  const int64_t now = fml::TimePoint::Now().ToEpochDelta().ToNanoseconds();
  auto [previous_vsync, next_vsync] =
      SnapToNextPhase(now, presentation_info.presentation_time,
                      presentation_info.presentation_interval);

  FlutterEngineOnVsync(flutter_engine_, baton, previous_vsync, next_vsync);
}

void ScenicSession::Present() {
  TRACE_EVENT0("gfx", "ScenicSession::Present");
  while (processed_present_session_trace_id_ < next_present_session_trace_id_) {
    TRACE_FLOW_END("gfx", "ScenicSession::QueuePresent",
                   processed_present_session_trace_id_);
    processed_present_session_trace_id_++;
  }
  TRACE_FLOW_BEGIN("gfx", "Session::Present", next_present_trace_id_);
  next_present_trace_id_++;

  // Presentation callback is pending as a result of Present() call below.
  // Flush all session ops.
  presentation_callback_pending_ = true;
  session_.Present(0,  // presentation_time. (placeholder).
                   [this](fuchsia::images::PresentationInfo presentation_info) {
                     presentation_callback_pending_ = false;

                     // Process pending Present() calls.
                     if (present_session_pending_) {
                       present_session_pending_ = false;
                       Present();
                     }

                     // Notify Flutter of the vsync.
                     intptr_t baton = pending_present_baton_;
                     pending_present_baton_ = 0;  // Clear before |OnVsync| to
                                                  // avoid races
                     if (baton != 0) {
                       FireVsyncCallback(std::move(presentation_info), baton);
                     }
                   }  // callback
  );
}

}  // namespace flutter_runner

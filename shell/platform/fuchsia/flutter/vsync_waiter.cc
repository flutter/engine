// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "vsync_waiter.h"

#include <cstdint>

#include <lib/async/default.h>

#include "flutter/fml/logging.h"
#include "flutter/fml/make_copyable.h"
#include "flutter/fml/synchronization/waitable_event.h"
#include "flutter/fml/time/time_delta.h"
#include "flutter/fml/trace_event.h"

#include "flutter_runner_product_configuration.h"
#include "vsync_recorder.h"

namespace flutter_runner {

/// Returns the system time at which the next frame is likely to be presented.
///
/// Consider the following scenarios, where in both the
/// scenarios the result will be the same.
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
fml::TimePoint VsyncWaiter::SnapToNextPhase(
    const fml::TimePoint now,
    const fml::TimePoint last_frame_presentation_time,
    const fml::TimeDelta presentation_interval) {
  if (presentation_interval <= fml::TimeDelta::Zero()) {
    FML_LOG(WARNING)
        << "Presentation interval must be positive. The value was: "
        << presentation_interval.ToMilliseconds() << "ms.";
    return now;
  }

  if (last_frame_presentation_time >= now) {
    FML_LOG(WARNING)
        << "Last frame was presented in the future. Clamping to now.";
    return now + presentation_interval;
  }

  const fml::TimeDelta time_since_last_presentation =
      now - last_frame_presentation_time;
  // this will be the most likely scenario if we are rendering at a good
  // frame rate, short circuiting the other checks in this case.
  if (time_since_last_presentation < presentation_interval) {
    return last_frame_presentation_time + presentation_interval;
  } else {
    const int64_t num_phases_passed =
        (time_since_last_presentation / presentation_interval);
    return last_frame_presentation_time +
           (presentation_interval * (num_phases_passed + 1));
  }
}

// This function takes in all relevant information to determining when should
// the next frame be scheduled. It returns a pair of (frame_start_time,
// frame_end_time) to be passed into FireCallback().
//
// Importantly, there are two invariants for correct and performant scheduling
// that this function upholds:
// 1. Schedule the next frame at least half a vsync interval from the previous
// one. In practice, this means that every vsync interval Flutter produces
// exactly one frame in steady state behavior.
// 2. Only produce frames beginning in the future.
//
// |vsync_offset| - the time from the next vsync that the animator should begin
// working on the next frame. For instance if vsyncs are at 0ms, 16ms, and 33ms,
// and the |vsync_offset| is 5ms, then frames should begin at 11ms and 28ms.
//
// |vsync_interval| - the interval between vsyncs. Would be 16.6ms for a 60Hz
// display.
//
// |last_targetted_vsync| - the last vsync targetted, which is usually the
// frame_end_time returned from the last invocation of this function
//
// |now| - the current time
//
// |next_vsync| - the next vsync after |now|. This can be generated using the
// SnapToNextPhase function.
FlutterFrameTimes VsyncWaiter::GetTargetTimes(
    fml::TimeDelta vsync_offset,
    fml::TimeDelta vsync_interval,
    fml::TimePoint last_targetted_vsync,
    fml::TimePoint now,
    fml::TimePoint next_vsync) {
  FML_DCHECK(vsync_offset <= vsync_interval);
  FML_DCHECK(vsync_interval > fml::TimeDelta::FromMilliseconds(0));
  FML_DCHECK(now < next_vsync && next_vsync < now + vsync_interval);

  // This makes the math much easier below, since we live in a (mod
  // vsync_interval) world.
  if (vsync_offset == fml::TimeDelta::FromNanoseconds(0)) {
    vsync_offset = vsync_interval;
  }

  // Start the frame after Scenic has finished its CPU work. This is
  // accomplished using the vsync_offset.
  fml::TimeDelta vsync_offset2 = vsync_interval - vsync_offset;
  fml::TimePoint frame_start_time =
      (next_vsync - vsync_interval) + vsync_offset2;

  fml::TimePoint frame_end_time = next_vsync;

  // Advance to next available slot, keeping in mind the two invariants.
  while (frame_end_time < (last_targetted_vsync + (vsync_interval / 2)) ||
         frame_start_time < now) {
    frame_start_time = frame_start_time + vsync_interval;
    frame_end_time = frame_end_time + vsync_interval;
  }

  // Useful knowledge for analyzing traces.
  fml::TimePoint previous_vsync = next_vsync - vsync_interval;
  TRACE_DURATION(
      "flutter", "VsyncWaiter::GetTargetTimes", "previous_vsync(ms)",
      previous_vsync.ToEpochDelta().ToMilliseconds(), "last_targetted(ms)",
      last_targetted_vsync.ToEpochDelta().ToMilliseconds(), "now(ms)",
      fml::TimePoint::Now().ToEpochDelta().ToMilliseconds(), "next_vsync(ms))",
      next_vsync.ToEpochDelta().ToMilliseconds(), "frame_start_time(ms)",
      frame_start_time.ToEpochDelta().ToMilliseconds(), "frame_end_time(ms)",
      frame_end_time.ToEpochDelta().ToMilliseconds());

  return {frame_start_time, frame_end_time};
}

VsyncWaiter::VsyncWaiter(std::shared_ptr<SessionConnection> session_connection,
                         flutter::TaskRunners task_runners,
                         fml::TimeDelta vsync_offset)
    : flutter::VsyncWaiter(task_runners),
      session_connection_(session_connection),
      vsync_offset_(vsync_offset),
      weak_factory_ui_(nullptr),
      weak_factory_(this) {
  FML_CHECK(session_connection_);

  // Generate a WeakPtrFactory for use with the UI thread. This does not need
  // to wait on a latch because we only ever use the WeakPtrFactory on the UI
  // thread so we have ordering guarantees (see ::AwaitVSync())
  fml::TaskRunner::RunNowOrPostTask(
      task_runners_.GetUITaskRunner(), fml::MakeCopyable([this]() mutable {
        this->weak_factory_ui_ =
            std::make_unique<fml::WeakPtrFactory<VsyncWaiter>>(this);
      }));

  // Initialize the callback that will run every time the SessionConnection
  // receives one or more frame presented events on Vsync.
  session_connection_->InitializeVsyncWaiterCallback([this]() {
    task_runners_.GetUITaskRunner()->PostTask([&weak_factory_ui =
                                                   this->weak_factory_ui_] {
      if (!weak_factory_ui) {
        FML_LOG(WARNING) << "WeakPtrFactory for VsyncWaiter is null, likely "
                            "due to the VsyncWaiter being destroyed.";
        return;
      }

      auto self = weak_factory_ui->GetWeakPtr();
      if (self) {
        self->OnVsync();
      }
    });
  });

  FML_LOG(INFO) << "Flutter VsyncWaiter: Set vsync_offset to "
                << vsync_offset_.ToMicroseconds() << "us";
}

VsyncWaiter::~VsyncWaiter() {
  fml::AutoResetWaitableEvent ui_latch;
  fml::TaskRunner::RunNowOrPostTask(
      task_runners_.GetUITaskRunner(),
      fml::MakeCopyable(
          [weak_factory_ui = std::move(weak_factory_ui_), &ui_latch]() mutable {
            weak_factory_ui.reset();
            ui_latch.Signal();
          }));
  ui_latch.Wait();
}

// This function is called when the Animator wishes to schedule a new frame.
void VsyncWaiter::AwaitVSync() {
  TRACE_DURATION("flutter", "VsyncWaiter::AwaitVsync()",
                 "request_already_pending", frame_request_pending_);
  FireCallbackMaybe();
}

// This function is called when the Animator wants to know about the next vsync,
// but not for frame scheduling purposes.
void VsyncWaiter::AwaitVSyncForSecondaryCallback() {
  TRACE_DURATION("flutter", "VsyncWaiter::AwaitVsyncForSecondaryCallback()",
                 "request_already_pending", frame_request_pending_);

  FlutterFrameTimes times = GetTargetTimesHelper(/*secondary_callback=*/true);
  FireCallback(times.frame_start, times.frame_target, false);
}

// Postcondition: Either a frame is scheduled or frame_request_pending_ is set
// to true, meaning we will attempt to schedule a frame on the next |OnVsync|.
void VsyncWaiter::FireCallbackMaybe() {
  TRACE_DURATION("flutter", "FireCallbackMaybe");

  if (session_connection_->CanRequestNewFrames()) {
    FlutterFrameTimes times =
        GetTargetTimesHelper(/*secondary_callback=*/false);

    last_targetted_vsync_ = times.frame_target;
    frame_request_pending_ = false;

    FireCallback(times.frame_start, times.frame_target, false);
  } else {
    frame_request_pending_ = true;
  }
}

// This function is called when the SessionConnection signals us to let us know
// that one or more frames Flutter has produced have been displayed. Note that
// in practice this will be called several milliseconds after vsync, due to
// CPU contention.
void VsyncWaiter::OnVsync() {
  TRACE_DURATION("flutter", "VsyncWaiter::OnVsync");

  if (frame_request_pending_) {
    FireCallbackMaybe();
  }
}

// A helper function for GetTargetTimes(), since many of the fields it takes
// have to be derived from other state.
FlutterFrameTimes VsyncWaiter::GetTargetTimesHelper(bool secondary_callback) {
  fml::TimeDelta presentation_interval =
      VsyncRecorder::GetInstance().GetCurrentVsyncInfo().presentation_interval;

  fml::TimePoint next_vsync =
      VsyncRecorder::GetInstance().GetCurrentVsyncInfo().presentation_time;
  fml::TimePoint now = fml::TimePoint::Now();
  fml::TimePoint last_presentation_time =
      VsyncRecorder::GetInstance().GetLastPresentationTime();
  if (next_vsync <= now) {
    next_vsync =
        SnapToNextPhase(now, last_presentation_time, presentation_interval);
  }

  fml::TimePoint last_targetted_vsync =
      secondary_callback ? fml::TimePoint::Min() : last_targetted_vsync_;
  return GetTargetTimes(vsync_offset_, presentation_interval,
                        last_targetted_vsync, now, next_vsync);
}

}  // namespace flutter_runner

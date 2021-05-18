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

VsyncWaiter::VsyncWaiter(
    AwaitVsyncCallback await_vsync,
    AwaitVsyncForSecondaryCallbackCallback await_vsync_for_secondary_callback,
    flutter::TaskRunners task_runners)
    : flutter::VsyncWaiter(task_runners),
      await_vsync_(await_vsync),
      await_vsync_for_secondary_callback_(await_vsync_for_secondary_callback),
      weak_factory_ui_(nullptr),
      weak_factory_(this) {
  fire_callback_callback_ = [this](fml::TimePoint frame_start,
                                   fml::TimePoint frame_end) {
    // Note: It is VERY important to set |pause_secondary_tasks| to false, else
    // Animator will almost immediately crash on Fuchsia.
    // FML_LOG(INFO) << "CRASH:: VsyncWaiter about to FireCallback";
    FireCallback(frame_start, frame_end, /*pause_secondary_tasks=*/false);
  };

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
  /*
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

  */

  FML_LOG(INFO) << "CRASH: VsyncWaiter init";
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
  // FML_LOG(INFO) << "CRASH: VsyncWaiter::AwaitVsync";
  await_vsync_(fire_callback_callback_);
}

// This function is called when the Animator wants to know about the next vsync,
// but not for frame scheduling purposes.
void VsyncWaiter::AwaitVSyncForSecondaryCallback() {
  // FML_LOG(INFO) << "CRASH: VsyncWaiter::AwaitVsyncForSecondaryCallback";
  await_vsync_for_secondary_callback_(fire_callback_callback_);
  //
  //
  // session_connection_->AwaitVsyncForSecondaryCallback();

  // FlutterFrameTimes times =
  // GetTargetTimesHelper(/*secondary_callback=*/true);
  // FireCallback(times.frame_start, times.frame_target, false);
}

/*
// Postcondition: Either a frame is scheduled or frame_request_pending_ is set
// to true, meaning we will attempt to schedule a frame on the next |OnVsync|.
void VsyncWaiter::FireCallbackMaybe() {
  TRACE_DURATION("flutter", "FireCallbackMaybe");

  if (session_connection_->CanRequestNewFrames()) {
    FlutterFrameTimes times =
        GetTargetTimesHelper(secondary_callback=false);

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
*/

}  // namespace flutter_runner

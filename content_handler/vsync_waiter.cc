// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/flutter/content_handler/vsync_waiter.h"

#include "lib/fsl/tasks/message_loop.h"

namespace flutter {

VsyncWaiter::VsyncWaiter(zx_handle_t session_present_handle,
                         blink::TaskRunners task_runners)
    : shell::VsyncWaiter(task_runners),
      session_wait_(session_present_handle, SessionPresentSignal) {
  auto wait_handler = [&](async_t* async,                   //
                          async::Wait* wait,                //
                          zx_status_t status,               //
                          const zx_packet_signal_t* signal  //
                      ) {
    if (status != ZX_OK) {
      FXL_LOG(ERROR) << "Vsync wait failed.";
      return;
    }

    wait->Cancel();

    status = zx_object_signal(wait->object(),        // object
                              SessionPresentSignal,  // clear mask
                              0                      // set mask
    );

    if (status != ZX_OK) {
      FXL_LOG(ERROR) << "Could not clear the vsync signal.";
      return;
    }

    FireCallbackNow();
  };

  session_wait_.set_handler(wait_handler);
}

VsyncWaiter::~VsyncWaiter() {
  session_wait_.Cancel();
}

void VsyncWaiter::AwaitVSync() {
  FXL_DCHECK(task_runners_.GetUITaskRunner()->RunsTasksOnCurrentThread());
  if (session_wait_.Begin(fsl::MessageLoop::GetCurrent()->async()) != ZX_OK) {
    FXL_LOG(ERROR) << "Could not begin wait for Vsync.";
  }
}

void VsyncWaiter::FireCallbackNow() {
  FXL_DCHECK(task_runners_.GetUITaskRunner()->RunsTasksOnCurrentThread());
  // We don't know the display refresh rate on this platform. Since the target
  // time is advisory, assume 60Hz.
  constexpr fxl::TimeDelta interval = fxl::TimeDelta::FromSecondsF(1.0 / 60.0);

  auto now = fxl::TimePoint::Now();
  auto next = now + interval;

  FireCallback(now, next);
}

}  // namespace flutter

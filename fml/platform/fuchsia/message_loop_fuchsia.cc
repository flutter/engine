// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/fml/platform/fuchsia/message_loop_fuchsia.h"

#include <lib/async-loop/default.h>
#include <lib/async/cpp/task.h>
#include <lib/zx/time.h>

namespace fml {

MessageLoopFuchsia::MessageLoopFuchsia()
  : loop_(&kAsyncLoopConfigAttachToCurrentThread),
    running_(false) {
  auto status = zx_timer_create(ZX_TIMER_SLACK_LATE, ZX_CLOCK_MONOTONIC, &timer_);
  FML_CHECK(status == ZX_OK);
  timer_wait_ = new async::Wait(timer_, ZX_TIMER_SIGNALED);

  auto wait_handler = [&](async_dispatcher_t* dispatcher,
                        async::Wait* wait,
                        zx_status_t status,
                        const zx_packet_signal_t* signal
                    ) {
    RunExpiredTasksNow();
    wait->Begin(loop_.dispatcher());
  };

  timer_wait_->set_handler(wait_handler);
  status = timer_wait_->Begin(loop_.dispatcher());
  FML_CHECK(status == ZX_OK);
}

MessageLoopFuchsia::~MessageLoopFuchsia() {
  delete timer_wait_;
  zx_handle_close(timer_);
}

void MessageLoopFuchsia::Run() {
  running_ = true;

  while (running_) {
    auto status = loop_.Run(zx::time::infinite(), true);

    // This handles the case where the loop is terminated using zx APIs.
    if (status == ZX_ERR_CANCELED) {
      running_ = false;
    }
  }
}

void MessageLoopFuchsia::Terminate() {
  running_ = false;
}

void MessageLoopFuchsia::WakeUp(fml::TimePoint time_point) {
  zx_time_t due_time = time_point.ToEpochDelta().ToNanoseconds();
  auto status = zx_timer_set(timer_, due_time, ZX_TIMER_SLACK_LATE);
  FML_CHECK(status == ZX_OK);
}

}  // namespace fml

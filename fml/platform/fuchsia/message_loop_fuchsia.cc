// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/fml/platform/fuchsia/message_loop_fuchsia.h"

#include <lib/async-loop/default.h>
#include <lib/async/cpp/task.h>
#include <lib/zx/time.h>

namespace fml {

MessageLoopFuchsia::MessageLoopFuchsia() : running_(false) {
  auto status = zx_timer_create(0, ZX_CLOCK_MONOTONIC, &timer_);
  FML_CHECK(status == ZX_OK);
}

MessageLoopFuchsia::~MessageLoopFuchsia() {
  zx_handle_close(timer_);
}

void MessageLoopFuchsia::Run() {
  zx_signals_t observed;
  running_ = true;

  while (running_) {
    auto status = zx_object_wait_one(timer_, ZX_TIMER_SIGNALED,
                                     ZX_TIME_INFINITE, &observed);
    FML_CHECK(observed == ZX_TIMER_SIGNALED ||
              observed == ZX_SIGNAL_HANDLE_CLOSED);

    RunExpiredTasksNow();

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
  auto status = zx_timer_set(timer_, due_time, ZX_TIMER_SLACK_CENTER);
  FML_CHECK(status == ZX_OK);
}

}  // namespace fml

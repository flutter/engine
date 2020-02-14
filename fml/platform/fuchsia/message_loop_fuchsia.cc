// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/fml/platform/fuchsia/message_loop_fuchsia.h"

#include <lib/async-loop/default.h>
#include <lib/async/cpp/task.h>

namespace fml {

MessageLoopFuchsia::MessageLoopFuchsia()
    : loop_(&kAsyncLoopConfigAttachToCurrentThread), running_(false) {
  auto status =
      zx::timer::create(ZX_TIMER_SLACK_LATE, ZX_CLOCK_MONOTONIC, &timer_);
  FML_CHECK(status == ZX_OK);
  timer_wait_ = std::make_unique<async::Wait>(timer_.get(), ZX_TIMER_SIGNALED);

  auto wait_handler = [this](async_dispatcher_t* dispatcher, async::Wait* wait,
                             zx_status_t status,
                             const zx_packet_signal_t* signal) {
    timer_.cancel();
    RunExpiredTasksNow();
    if (status != ZX_ERR_CANCELED) {
      FML_CHECK(signal->observed & ZX_TIMER_SIGNALED);
      auto result = wait->Begin(loop_.dispatcher());
      FML_CHECK(result == ZX_OK);
    }
  };

  timer_wait_->set_handler(wait_handler);
  status = timer_wait_->Begin(loop_.dispatcher());
  FML_CHECK(status == ZX_OK);
}

MessageLoopFuchsia::~MessageLoopFuchsia() = default;

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
  WakeUp(fml::TimePoint::Now());
}

void MessageLoopFuchsia::WakeUp(fml::TimePoint time_point) {
  zx::time due_time(time_point.ToEpochDelta().ToNanoseconds());
  auto status = timer_.set(due_time, zx::duration(1));
  FML_CHECK(status == ZX_OK);
}

}  // namespace fml

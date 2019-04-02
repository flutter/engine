// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/fml/platform/darwin/concurrent_message_loop_darwin.h"

namespace fml {

ConcurrentMessageLoopDarwin::ConcurrentMessageLoopDarwin()
    : queue_(dispatch_queue_create("io.flutter.worker", DISPATCH_QUEUE_CONCURRENT)),
      timer_(dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER,
                                    0 /* unused handle */,
                                    0 /* unused mask */,
                                    queue_)) {
  dispatch_source_set_event_handler_f(timer_, &ConcurrentMessageLoopDarwin::OnTimerFire);
  dispatch_set_context(timer_, this);
  dispatch_resume(timer_);
}

ConcurrentMessageLoopDarwin::~ConcurrentMessageLoopDarwin() {
  std::lock_guard<std::mutex> lock(timer_mutex_);
  dispatch_set_context(timer_, nullptr);
  dispatch_source_cancel(timer_);
  dispatch_release(queue_);
}

// |fml::MessageLoopImpl|
void ConcurrentMessageLoopDarwin::Run() {
  FML_CHECK(false);
}

// |fml::MessageLoopImpl|
void ConcurrentMessageLoopDarwin::Terminate() {
  std::lock_guard<std::mutex> lock(timer_mutex_);
  dispatch_source_cancel(timer_);
}

// |fml::MessageLoopImpl|
void ConcurrentMessageLoopDarwin::WakeUp(fml::TimePoint time_point) {
  std::lock_guard<std::mutex> lock(timer_mutex_);
  // Darwin timers are based on mach_absolute_time. dispatch_time must be used
  // so that the clocks match.
  auto wake_time = dispatch_time(time_point.ToEpochDelta().ToNanoseconds(),  // when
                                 0u                                          // delta
  );

  dispatch_source_set_timer(timer_,                 // source
                            wake_time,              // wake time
                            DISPATCH_TIME_FOREVER,  // interval (non-repeating)
                            0u                      // leeway
  );
}

void ConcurrentMessageLoopDarwin::OnTimerFire(void* user_data) {
  if (user_data == nullptr) {
    return;
  }

  reinterpret_cast<ConcurrentMessageLoopDarwin*>(user_data)->OnTimerFire();
}

void ConcurrentMessageLoopDarwin::OnTimerFire() {
  @autoreleasepool {
    RunSingleExpiredTaskNow();
  }
}

}  // namespace fml

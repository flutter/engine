// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/fml/platform/android/message_loop_android.h"
#include <unistd.h>
#include "flutter/fml/platform/linux/timerfd.h"
#include "lib/ftl/files/eintr_wrapper.h"

#ifndef NSEC_PER_SEC
#define NSEC_PER_SEC 1000000000
#endif

namespace fml {

static const int kClockType = CLOCK_MONOTONIC;

static ALooper* AcquireLooperForThread() {
  ALooper* looper = ALooper_forThread();

  if (looper == nullptr) {
    // No looper has been configured for the current thread. Create one and
    // return the same.
    looper = ALooper_prepare(0);
  }

  // The thread already has a looper. Acquire a reference to the same and return
  // it.
  ALooper_acquire(looper);
  return looper;
}

MessageLoopAndroid::MessageLoopAndroid()
    : looper_(AcquireLooperForThread()),
      timer_fd_(::timerfd_create(kClockType, TFD_NONBLOCK | TFD_CLOEXEC)),
      running_(false) {
  FTL_CHECK(looper_.is_valid());
  FTL_CHECK(timer_fd_.is_valid());

  static const int kWakeEvents = ALOOPER_EVENT_INPUT;

  ALooper_callbackFunc read_event_fd = [](int, int events, void* data) -> int {
    if (events & kWakeEvents) {
      reinterpret_cast<MessageLoopAndroid*>(data)->OnEventFired();
    }
    return 1;  // continue receiving callbacks
  };

  int add_result = ::ALooper_addFd(looper_.get(),          // looper
                                   timer_fd_.get(),        // fd
                                   ALOOPER_POLL_CALLBACK,  // ident
                                   kWakeEvents,            // events
                                   read_event_fd,          // callback
                                   this                    // baton
                                   );
  FTL_CHECK(add_result == 1);
}

MessageLoopAndroid::~MessageLoopAndroid() {
  int remove_result = ::ALooper_removeFd(looper_.get(), timer_fd_.get());
  FTL_CHECK(remove_result == 1);
}

void MessageLoopAndroid::Run() {
  FTL_DCHECK(looper_.get() == ALooper_forThread());

  running_ = true;

  while (running_) {
    int result = ::ALooper_pollOnce(-1,       // infinite timeout
                                    nullptr,  // out fd,
                                    nullptr,  // out events,
                                    nullptr   // out data
                                    );
    if (result == ALOOPER_POLL_TIMEOUT || result == ALOOPER_POLL_ERROR) {
      // This handles the case where the loop is terminated using ALooper APIs.
      running_ = false;
    }
  }
}

void MessageLoopAndroid::Terminate() {
  running_ = false;
  ALooper_wake(looper_.get());
}

void MessageLoopAndroid::WakeUp(ftl::TimePoint time_point) {
  const uint64_t nano_secs = time_point.ToEpochDelta().ToNanoseconds();

  struct itimerspec spec = {};
  spec.it_value.tv_sec = (time_t)(nano_secs / NSEC_PER_SEC);
  spec.it_value.tv_nsec = nano_secs % NSEC_PER_SEC;
  spec.it_interval = spec.it_value;  // single expiry.

  int result =
      ::timerfd_settime(timer_fd_.get(), TFD_TIMER_ABSTIME, &spec, nullptr);
  FTL_DCHECK(result == 0);
}

bool MessageLoopAndroid::DrainTimerFD() {
  // 8 bytes must be read from a signalled timer file descriptor when signalled.
  uint64_t fire_count = 0;
  ssize_t size =
      HANDLE_EINTR(::read(timer_fd_.get(), &fire_count, sizeof(uint64_t)));
  if (size != sizeof(uint64_t)) {
    return false;
  }
  return fire_count > 0;
}

void MessageLoopAndroid::OnEventFired() {
  if (DrainTimerFD()) {
    RunExpiredTasksNow();
  }
}

}  // namespace fml

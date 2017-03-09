// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/fml/platform/android/message_loop_android.h"
#include <unistd.h>
#include "lib/ftl/files/eintr_wrapper.h"

namespace fml {

static ALooper* AcquireLooperForThread() {
  ALooper* looper = ALooper_forThread();

  if (looper == nullptr) {
    // No looper has been configured for the current thread. Create one
    // reference it and return the same.
    looper = ALooper_prepare(0);
  }

  // The thread already has a looper. Acquire a reference to the same and return
  // it.
  ALooper_acquire(looper);
  return looper;
}

MessageLoopAndroid::MessageLoopAndroid()
    : looper_(AcquireLooperForThread()),
      event_fd_(::eventfd(0 /* initial value */,
                          EFD_CLOEXEC | EFD_NONBLOCK /* flags */)),
      running_(false) {
  static const int kWakeEvents = ALOOPER_EVENT_INPUT;

  ALooper_callbackFunc read_event_fd = [](int, int events, void* data) -> int {
    if (events & kWakeEvents) {
      reinterpret_cast<MessageLoopAndroid*>(data)->OnEventFired();
    }
    return 1;  // continue receiving callbacks
  };

  FTL_CHECK(looper_.is_valid());
  FTL_CHECK(event_fd_.is_valid());
  int add_result = ::ALooper_addFd(looper_.get(),          // looper
                                   event_fd_.get(),        // fd
                                   ALOOPER_POLL_CALLBACK,  // ident
                                   kWakeEvents,            // events
                                   read_event_fd,          // callback
                                   this                    // baton
                                   );
  FTL_CHECK(add_result == 1);
}

MessageLoopAndroid::~MessageLoopAndroid() {
  int remove_result = ::ALooper_removeFd(looper_.get(), event_fd_.get());
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
  // On signal, the 8 byte integer value is addded. We specify 1 for each wake.
  // The value itself does not matter since we never count the wakes on read.
  const uint64_t write_count = 1;
  ssize_t size =
      HANDLE_EINTR(::write(event_fd_.get(), &write_count, sizeof(write_count)));
  FTL_CHECK(size == sizeof(write_count));
}

bool MessageLoopAndroid::DrainEventFD() {
  // On read, the signal count is the number of times this descriptor was
  // woken up.
  uint64_t signal_count = 0;
  ssize_t size = HANDLE_EINTR(
      ::read(event_fd_.get(), &signal_count, sizeof(signal_count)));
  if (size != sizeof(signal_count)) {
    return false;
  }
  return signal_count > 0;
}

void MessageLoopAndroid::OnEventFired() {
  if (DrainEventFD()) {
    RunExpiredTasksNow();
  }
}

}  // namespace fml

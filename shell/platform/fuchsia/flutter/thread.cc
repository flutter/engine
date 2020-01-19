// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/fuchsia/flutter/thread.h"

#include <lib/async-loop/default.h>
#include <lib/async/cpp/task.h>
#include <lib/async/default.h>

namespace flutter_runner {

Thread::Thread()
  : loop_(&kAsyncLoopConfigNoAttachToCurrentThread),
    thread_([this]() {
      async_set_default_dispatcher(loop_.dispatcher());
      loop_.Run();
    }) {}

Thread::~Thread() {
  Join();
}

async_dispatcher_t* Thread::dispatcher() const {
  return loop_.dispatcher();
}

void Thread::Quit() {
  loop_.Quit();
}

void Thread::Join() {
  if (joined_) {
    return;
  }
  joined_ = true;

  async::PostTask(loop_.dispatcher(), [this]() {
    Quit();
  });
  thread_.join();
}

}  // namespace flutter_runner

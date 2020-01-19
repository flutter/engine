// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/fuchsia/utils/thread.h"

#include <condition_variable>
#include <mutex>
#include <utility>

#include <lib/async-loop/default.h>
#include <lib/async/cpp/task.h>
#include <lib/zx/process.h>
#include <lib/zx/thread.h>

#include "flutter/shell/platform/fuchsia/utils/logging.h"

namespace fx {
namespace {

std::thread CreateLoopThreadSync(std::string name, async::Loop*& loop_ptr) {
  std::pair<std::mutex, std::condition_variable> latch;

  std::thread loop_thread([&]() {
    Thread::SetCurrentThreadName(std::move(name));

    // Create loop and inform the waiting parent thread about it.
    async::Loop loop(&kAsyncLoopConfigAttachToCurrentThread);
    {
      // Guard the latch with a lock.  This lock must go out of scope before the
      // latch is truly signalled, so we must destroy it before calling |Run()|.
      std::lock_guard<std::mutex> locker(latch.first);
      loop_ptr = &loop;
      latch.second.notify_one();
    }

    loop.Run();
    loop_ptr = nullptr;
  });

  // Wait on the latch to make sure the loop is created.
  std::unique_lock<std::mutex> locker(latch.first);
  latch.second.wait(locker, [&]() -> bool { return loop_ptr; });

  return loop_thread;
}

}  // namespace

void Thread::SetProcessName(std::string process_name) {
  zx::process::self()->set_property(ZX_PROP_NAME, process_name.c_str(),
                                    process_name.size());
}

void Thread::SetCurrentThreadName(std::string thread_name) {
  zx::thread::self()->set_property(ZX_PROP_NAME, thread_name.c_str(),
                                   thread_name.size());
}

Thread::Thread(std::string name)
    : thread_(CreateLoopThreadSync(std::move(name), loop_)) {}

Thread::~Thread() {
  Join();
}

void Thread::TaskBarrier(std::function<void()> task) const {
  FX_DCHECK(loop_);

  std::pair<std::mutex, std::condition_variable> latch;
  bool signal = false;

  // Run the task on this Thread's dispatcher and signal the latch.
  async::PostTask(dispatcher(), [&]() {
    task();

    std::lock_guard<std::mutex> locker(latch.first);
    signal = true;
    latch.second.notify_one();
  });

  // Make the task synchronous by waiting on the latch.
  std::unique_lock<std::mutex> locker(latch.first);
  latch.second.wait(locker, [&signal]() -> bool { return signal; });
}

void Thread::Join() {
  if (joined_ || !thread_.joinable()) {
    return;
  }
  joined_ = true;

  async::PostTask(loop_->dispatcher(), [this]() { loop_->Quit(); });
  thread_.join();
}

}  // namespace fx

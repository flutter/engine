// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/fuchsia/flutter/tests/real_loop_fixture.h"

#include <lib/async/cpp/task.h>
#include <lib/zx/clock.h>

namespace gtest {

namespace {

bool RunGivenLoopWithTimeout(async::Loop* loop, zx::duration timeout) {
  // This cannot be a local variable because the delayed task below can execute
  // after this function returns.
  auto canceled = std::make_shared<bool>(false);
  bool timed_out = false;
  async::PostDelayedTask(
      loop->dispatcher(),
      [loop, canceled, &timed_out] {
        if (*canceled) {
          return;
        }
        timed_out = true;
        loop->Quit();
      },
      timeout);
  loop->Run();
  loop->ResetQuit();
  // Another task can call Quit() on the message loop, which exits the
  // message loop before the delayed task executes, in which case |timed_out| is
  // still false here because the delayed task hasn't run yet.
  // Since the message loop isn't destroyed then (as it usually would after
  // Quit()), and presumably can be reused after this function returns we
  // still need to prevent the delayed task to quit it again at some later time
  // using the canceled pointer.
  if (!timed_out) {
    *canceled = true;
  }
  return timed_out;
}

}  // namespace

RealLoopFixture::RealLoopFixture()
    : loop_(&kAsyncLoopConfigAttachToCurrentThread) {}

RealLoopFixture::~RealLoopFixture() = default;

async_dispatcher_t* RealLoopFixture::dispatcher() {
  return loop_.dispatcher();
}

void RealLoopFixture::RunLoop() {
  loop_.Run();
  loop_.ResetQuit();
}

bool RealLoopFixture::RunLoopWithTimeout(zx::duration timeout) {
  return RunGivenLoopWithTimeout(&loop_, timeout);
}

bool RealLoopFixture::RunLoopWithTimeoutOrUntil(fit::function<bool()> condition,
                                                zx::duration timeout,
                                                zx::duration step) {
  const zx::time timeout_deadline = zx::deadline_after(timeout);

  while (zx::clock::get_monotonic() < timeout_deadline &&
         loop_.GetState() == ASYNC_LOOP_RUNNABLE) {
    if (condition()) {
      loop_.ResetQuit();
      return true;
    }

    if (step == zx::duration::infinite()) {
      // Performs a single unit of work, possibly blocking until there is work
      // to do or the timeout deadline arrives.
      loop_.Run(timeout_deadline, true);
    } else {
      // Performs work until the step deadline arrives.
      RunGivenLoopWithTimeout(&loop_, step);
    }
  }

  loop_.ResetQuit();
  return condition();
}

void RealLoopFixture::RunLoopUntil(fit::function<bool()> condition,
                                   zx::duration step) {
  RunLoopWithTimeoutOrUntil(std::move(condition), zx::duration::infinite(),
                            step);
}

void RealLoopFixture::RunLoopUntilIdle() {
  loop_.RunUntilIdle();
  loop_.ResetQuit();
}

void RealLoopFixture::QuitLoop() {
  loop_.Quit();
}

fit::closure RealLoopFixture::QuitLoopClosure() {
  return [this] { loop_.Quit(); };
}

}  // namespace gtest

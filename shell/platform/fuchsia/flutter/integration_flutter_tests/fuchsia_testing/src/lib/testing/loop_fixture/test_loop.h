// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_LIB_TESTING_LOOP_FIXTURE_INTERNAL_TEST_LOOP_FIXTURE_H_
#define SRC_LIB_TESTING_LOOP_FIXTURE_INTERNAL_TEST_LOOP_FIXTURE_H_

#include <lib/async-testing/test_loop.h>
#include <lib/fit/function.h>

#include <functional>

#include "src/lib/fxl/macros.h"

namespace loop_fixture {

class TestLoop {
 protected:
  TestLoop();
  ~TestLoop();

  async_dispatcher_t* dispatcher() { return loop_.dispatcher(); }

  // Returns the current fake clock time.
  zx::time Now() { return loop_.Now(); }

  // Dispatches all waits and all tasks posted to the message loop with
  // deadlines up until |deadline|, progressively advancing the fake clock.
  // Returns true iff any tasks or waits were invoked during the run.
  bool RunLoopUntil(zx::time deadline) { return loop_.RunUntil(deadline); }

  // Dispatches all waits and all tasks posted to the message loop with
  // deadlines up until |duration| from the current time, progressively
  // advancing the fake clock.
  // Returns true iff any tasks or waits were invoked during the run.
  bool RunLoopFor(zx::duration duration) { return loop_.RunFor(duration); };

  // Dispatches all waits and all tasks posted to the message loop with
  // deadlines up until the current time, progressively advancing the fake
  // clock.
  // Returns true iff any tasks or waits were invoked during the run.
  bool RunLoopUntilIdle() { return loop_.RunUntilIdle(); }

  // Repeatedly runs the loop by |increment| until nothing further is left to
  // dispatch.
  void RunLoopRepeatedlyFor(zx::duration increment);

  // Quits the message loop. If called while running, it will immediately
  // exit and dispatch no further tasks or waits; if called before running,
  // then next call to run will immediately exit. Further calls to run will
  // continue to dispatch.
  void QuitLoop() { loop_.Quit(); }

  // A callback that quits the message loop when called.
  fit::closure QuitLoopClosure() {
    return [this] { loop_.Quit(); };
  }

  // Accessor for the underlying TestLoop.
  async::TestLoop& test_loop() { return loop_; }

 private:
  // The test message loop for the test fixture.
  async::TestLoop loop_;

  FXL_DISALLOW_COPY_AND_ASSIGN(TestLoop);
};

}  // namespace loop_fixture

#endif  // SRC_LIB_TESTING_LOOP_FIXTURE_INTERNAL_TEST_LOOP_FIXTURE_H_

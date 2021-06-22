// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "lib/gtest/real_loop_fixture.h"

#include <lib/async/cpp/task.h>
#include <lib/async/dispatcher.h>
#include <lib/fit/bridge.h>
#include <lib/fit/promise.h>
#include <lib/syslog/cpp/macros.h>
#include <lib/zx/time.h>

#include <gtest/gtest.h>

namespace gtest {
namespace {

using RealLoopFixtureTest = RealLoopFixture;

TEST_F(RealLoopFixtureTest, Timeout) {
  bool called = false;
  async::PostDelayedTask(
      dispatcher(), [&called] { called = true; }, zx::msec(100));
  RunLoopWithTimeout(zx::msec(10));
  EXPECT_FALSE(called);
  RunLoopWithTimeout(zx::msec(100));
  EXPECT_TRUE(called);
}

TEST_F(RealLoopFixtureTest, NoTimeout) {
  // Check that the first run loop doesn't hit the timeout.
  QuitLoop();
  EXPECT_FALSE(RunLoopWithTimeout(zx::msec(10)));
  // But the second does.
  EXPECT_TRUE(RunLoopWithTimeout(zx::msec(10)));
}

TEST_F(RealLoopFixtureTest, RunPromiseResolved) {
  {
    auto res = RunPromise(fit::make_ok_promise("hello"));
    ASSERT_TRUE(res.is_ok());
    EXPECT_EQ(res.value(), "hello");
  }
  {
    auto res = RunPromise(fit::make_error_promise(1234));
    ASSERT_TRUE(!res.is_ok());
    EXPECT_EQ(res.error(), 1234);
  }
}

TEST_F(RealLoopFixtureTest, RunPromiseRequiresMultipleLoops) {
  // Make a promise whose closure needs to run 5 times to complete, and which
  // wakes itself up after each loop.
  fit::result<std::string> res = RunPromise(
      fit::make_promise([count = 0](fit::context& ctx) mutable -> fit::result<std::string> {
        if (count < 5) {
          count++;
          // Tell the executor to call us again.
          ctx.suspend_task().resume_task();
          return fit::pending();
        }
        return fit::ok("finished");
      }));
  ASSERT_TRUE(res.is_ok());
  EXPECT_EQ(res.value(), "finished");
}

// Returns a promise that completes after |delay|.
fit::promise<> DelayedPromise(async_dispatcher_t* dispatcher, zx::duration delay) {
  fit::bridge<> bridge;
  async::PostDelayedTask(dispatcher, bridge.completer.bind(), delay);
  return bridge.consumer.promise();
}

TEST_F(RealLoopFixtureTest, RunPromiseDelayed) {
  fit::result<> res = RunPromise(DelayedPromise(dispatcher(), zx::msec(100)));
  EXPECT_TRUE(res.is_ok());
}

}  // namespace
}  // namespace gtest

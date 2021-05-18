// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>
#include <lib/async-loop/default.h>
#include <lib/zx/event.h>
#include <zircon/syscalls.h>

#include <array>

#include "flutter/fml/time/time_delta.h"
#include "flutter/fml/time/time_point.h"
#include "flutter/shell/common/thread_host.h"
#include "flutter/shell/common/vsync_waiter.h"
#include "flutter/shell/platform/fuchsia/flutter/flutter_runner_product_configuration.h"
#include "flutter/shell/platform/fuchsia/flutter/task_runner_adapter.h"
#include "flutter/shell/platform/fuchsia/flutter/thread.h"
#include "flutter/shell/platform/fuchsia/flutter/vsync_waiter.h"

namespace flutter_runner_test {

static fml::TimePoint TimePointFromInt(int i) {
  return fml::TimePoint::FromEpochDelta(fml::TimeDelta::FromNanoseconds(i));
}
static fml::TimeDelta TimeDeltaFromInt(int i) {
  return fml::TimeDelta::FromNanoseconds(i);
}
static int TimePointToInt(fml::TimePoint time) {
  return time.ToEpochDelta().ToNanoseconds();
}

class VsyncWaiterTest : public testing::Test {
 public:
  VsyncWaiterTest() {
    for (auto& thread : threads_) {
      thread.reset(new flutter_runner::Thread());
    }

    const flutter::TaskRunners task_runners(
        "VsyncWaiterTests",  // Dart thread labels
        flutter_runner::CreateFMLTaskRunner(
            async_get_default_dispatcher()),  // platform
        flutter_runner::CreateFMLTaskRunner(
            threads_[0]->dispatcher()),  // raster
        flutter_runner::CreateFMLTaskRunner(threads_[1]->dispatcher()),  // ui
        flutter_runner::CreateFMLTaskRunner(threads_[2]->dispatcher())   // io
    );

    session_connection_ = std::make_shared<FakeSessionConnection>();
    vsync_waiter_ = std::make_unique<flutter_runner::VsyncWaiter>(
        session_connection_, task_runners, fml::TimeDelta::Zero());
  }

  flutter_runner::VsyncWaiter* get_vsync_waiter() {
    return vsync_waiter_.get();
  }
  FakeSessionConnection* get_session_connection() {
    return session_connection_.get();
  }

  ~VsyncWaiterTest() {
    vsync_waiter_.reset();
    for (const auto& thread : threads_) {
      thread->Quit();
    }
  }

 private:
  async::Loop loop_ = async::Loop(&kAsyncLoopConfigAttachToCurrentThread);
  std::shared_ptr<FakeSessionConnection> session_connection_;
  std::unique_ptr<flutter_runner::VsyncWaiter> vsync_waiter_;
  std::array<std::unique_ptr<flutter_runner::Thread>, 3> threads_;
};

TEST_F(VsyncWaiterTest, SimpleVsyncCallback) {
  auto vsync_waiter = get_vsync_waiter();
  auto session_connection = get_session_connection();

  bool flag = false;
  vsync_waiter->AsyncWaitForVsync([&flag](auto) { flag = true; });

  session_connection->FireVsyncWaiterCallback();

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  EXPECT_TRUE(flag);
}

TEST_F(VsyncWaiterTest, EnsureBackPressure) {
  auto vsync_waiter = get_vsync_waiter();
  auto session_connection = get_session_connection();

  session_connection->SetCanRequestNewFrames(false);
  bool flag = false;
  vsync_waiter->AsyncWaitForVsync([&flag](auto) { flag = true; });

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  EXPECT_FALSE(flag);
}

TEST_F(VsyncWaiterTest, BackPressureRelief) {
  auto vsync_waiter = get_vsync_waiter();
  auto session_connection = get_session_connection();

  session_connection->SetCanRequestNewFrames(false);
  bool flag = false;
  vsync_waiter->AsyncWaitForVsync([&flag](auto) { flag = true; });

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  EXPECT_FALSE(flag);

  session_connection->SetCanRequestNewFrames(true);
  session_connection->FireVsyncWaiterCallback();

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  EXPECT_TRUE(flag);
}

TEST_F(VsyncWaiterTest, SteadyStateBehavior) {
  auto vsync_waiter = get_vsync_waiter();
  auto session_connection = get_session_connection();

  session_connection->SetCanRequestNewFrames(true);

  int count = 0;
  const int expected_iterations = 100;

  for (int i = 0; i < expected_iterations; ++i) {
    vsync_waiter->AsyncWaitForVsync([&count](auto) { count++; });
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  EXPECT_EQ(count, expected_iterations);
}

}  // namespace flutter_runner_test

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

#include "fake_session_connection.h"

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

TEST(SnapToNextPhaseTest, SnapOverlapsWithNow) {
  const auto now = fml::TimePoint::Now();
  const auto last_presentation_time = now - fml::TimeDelta::FromNanoseconds(10);
  const auto delta = fml::TimeDelta::FromNanoseconds(10);
  const auto next_vsync = flutter_runner::VsyncWaiter::SnapToNextPhase(
      now, last_presentation_time, delta);

  EXPECT_EQ(now + delta, next_vsync);
}

TEST(SnapToNextPhaseTest, SnapAfterNow) {
  const auto now = fml::TimePoint::Now();
  const auto last_presentation_time = now - fml::TimeDelta::FromNanoseconds(9);
  const auto delta = fml::TimeDelta::FromNanoseconds(10);
  const auto next_vsync = flutter_runner::VsyncWaiter::SnapToNextPhase(
      now, last_presentation_time, delta);

  // math here: 10 - 9 = 1
  EXPECT_EQ(now + fml::TimeDelta::FromNanoseconds(1), next_vsync);
}

TEST(SnapToNextPhaseTest, SnapAfterNowMultiJump) {
  const auto now = fml::TimePoint::Now();
  const auto last_presentation_time = now - fml::TimeDelta::FromNanoseconds(34);
  const auto delta = fml::TimeDelta::FromNanoseconds(10);
  const auto next_vsync = flutter_runner::VsyncWaiter::SnapToNextPhase(
      now, last_presentation_time, delta);

  // zeroes: -34, -24, -14, -4, 6, ...
  EXPECT_EQ(now + fml::TimeDelta::FromNanoseconds(6), next_vsync);
}

TEST(SnapToNextPhaseTest, SnapAfterNowMultiJumpAccountForCeils) {
  const auto now = fml::TimePoint::Now();
  const auto last_presentation_time = now - fml::TimeDelta::FromNanoseconds(20);
  const auto delta = fml::TimeDelta::FromNanoseconds(16);
  const auto next_vsync = flutter_runner::VsyncWaiter::SnapToNextPhase(
      now, last_presentation_time, delta);

  // zeroes: -20, -4, 12, 28, ...
  EXPECT_EQ(now + fml::TimeDelta::FromNanoseconds(12), next_vsync);
}

TEST(GetTargetTimesTest, ScheduleForNextVsync) {
  const fml::TimeDelta vsync_offset = TimeDeltaFromInt(0);
  const fml::TimeDelta vsync_interval = TimeDeltaFromInt(10);
  const fml::TimePoint last_targetted_vsync = TimePointFromInt(10);
  const fml::TimePoint now = TimePointFromInt(9);
  const fml::TimePoint next_vsync = TimePointFromInt(10);

  const auto target_times = flutter_runner::VsyncWaiter::GetTargetTimes(
      vsync_offset, vsync_interval, last_targetted_vsync, now, next_vsync);

  EXPECT_EQ(TimePointToInt(target_times.frame_start), 10);
  EXPECT_EQ(TimePointToInt(target_times.frame_target), 20);
}

TEST(GetTargetTimesTest, ScheduleForCurrentVsync_DueToOffset) {
  const fml::TimeDelta vsync_offset = TimeDeltaFromInt(3);
  const fml::TimeDelta vsync_interval = TimeDeltaFromInt(10);
  const fml::TimePoint last_targetted_vsync = TimePointFromInt(0);
  const fml::TimePoint now = TimePointFromInt(6);
  const fml::TimePoint next_vsync = TimePointFromInt(10);

  const auto target_times = flutter_runner::VsyncWaiter::GetTargetTimes(
      vsync_offset, vsync_interval, last_targetted_vsync, now, next_vsync);

  EXPECT_EQ(TimePointToInt(target_times.frame_start), 7);
  EXPECT_EQ(TimePointToInt(target_times.frame_target), 10);
}

TEST(GetTargetTimesTest, ScheduleForFollowingVsync_BecauseOfNow) {
  const fml::TimeDelta vsync_offset = TimeDeltaFromInt(0);
  const fml::TimeDelta vsync_interval = TimeDeltaFromInt(10);
  const fml::TimePoint last_targetted_vsync = TimePointFromInt(10);
  const fml::TimePoint now = TimePointFromInt(15);
  const fml::TimePoint next_vsync = TimePointFromInt(10);

  const auto target_times = flutter_runner::VsyncWaiter::GetTargetTimes(
      vsync_offset, vsync_interval, last_targetted_vsync, now, next_vsync);

  EXPECT_EQ(TimePointToInt(target_times.frame_start), 20);
  EXPECT_EQ(TimePointToInt(target_times.frame_target), 30);
}

TEST(GetTargetTimesTest, ScheduleForFollowingVsync_BecauseOfTargettedTime) {
  const fml::TimeDelta vsync_offset = TimeDeltaFromInt(0);
  const fml::TimeDelta vsync_interval = TimeDeltaFromInt(10);
  const fml::TimePoint last_targetted_vsync = TimePointFromInt(20);
  const fml::TimePoint now = TimePointFromInt(9);
  const fml::TimePoint next_vsync = TimePointFromInt(10);

  const auto target_times = flutter_runner::VsyncWaiter::GetTargetTimes(
      vsync_offset, vsync_interval, last_targetted_vsync, now, next_vsync);

  EXPECT_EQ(TimePointToInt(target_times.frame_start), 20);
  EXPECT_EQ(TimePointToInt(target_times.frame_target), 30);
}

TEST(GetTargetTimesTest, ScheduleForDistantVsync_BecauseOfTargettedTime) {
  const fml::TimeDelta vsync_offset = TimeDeltaFromInt(0);
  const fml::TimeDelta vsync_interval = TimeDeltaFromInt(10);
  const fml::TimePoint last_targetted_vsync = TimePointFromInt(60);
  const fml::TimePoint now = TimePointFromInt(9);
  const fml::TimePoint next_vsync = TimePointFromInt(10);

  const auto target_times = flutter_runner::VsyncWaiter::GetTargetTimes(
      vsync_offset, vsync_interval, last_targetted_vsync, now, next_vsync);

  EXPECT_EQ(TimePointToInt(target_times.frame_start), 60);
  EXPECT_EQ(TimePointToInt(target_times.frame_target), 70);
}

TEST(GetTargetTimesTest, ScheduleForFollowingVsync_WithSlightVsyncDrift) {
  const fml::TimeDelta vsync_offset = TimeDeltaFromInt(0);
  const fml::TimeDelta vsync_interval = TimeDeltaFromInt(10);

  // Even though it appears as if the next vsync is at time 40, we should still
  // present at time 50.
  const fml::TimePoint last_targetted_vsync = TimePointFromInt(37);
  const fml::TimePoint now = TimePointFromInt(9);
  const fml::TimePoint next_vsync = TimePointFromInt(10);

  const auto target_times = flutter_runner::VsyncWaiter::GetTargetTimes(
      vsync_offset, vsync_interval, last_targetted_vsync, now, next_vsync);

  EXPECT_EQ(TimePointToInt(target_times.frame_start), 40);
  EXPECT_EQ(TimePointToInt(target_times.frame_target), 50);
}

TEST(GetTargetTimesTest, ScheduleForAnOffsetFromVsync) {
  const fml::TimeDelta vsync_offset = TimeDeltaFromInt(4);
  const fml::TimeDelta vsync_interval = TimeDeltaFromInt(10);
  const fml::TimePoint last_targetted_vsync = TimePointFromInt(10);
  const fml::TimePoint now = TimePointFromInt(9);
  const fml::TimePoint next_vsync = TimePointFromInt(10);

  const auto target_times = flutter_runner::VsyncWaiter::GetTargetTimes(
      vsync_offset, vsync_interval, last_targetted_vsync, now, next_vsync);

  EXPECT_EQ(TimePointToInt(target_times.frame_start), 16);
  EXPECT_EQ(TimePointToInt(target_times.frame_target), 20);
}

TEST(GetTargetTimesTest, ScheduleMultipleTimes) {
  const fml::TimeDelta vsync_offset = TimeDeltaFromInt(0);
  const fml::TimeDelta vsync_interval = TimeDeltaFromInt(10);

  fml::TimePoint last_targetted_vsync = TimePointFromInt(0);
  fml::TimePoint now = TimePointFromInt(5);
  fml::TimePoint next_vsync = TimePointFromInt(10);

  for (int i = 0; i < 100; ++i) {
    const auto target_times = flutter_runner::VsyncWaiter::GetTargetTimes(
        vsync_offset, vsync_interval, last_targetted_vsync, now, next_vsync);

    EXPECT_EQ(TimePointToInt(target_times.frame_start), 10 * (i + 1));
    EXPECT_EQ(TimePointToInt(target_times.frame_target), 10 * (i + 2));

    // Simulate the passage of time.
    now = now + vsync_interval;
    next_vsync = next_vsync + vsync_interval;
    last_targetted_vsync = target_times.frame_target;
  }
}

TEST(GetTargetTimesTest, ScheduleMultipleTimes_WithDelayedWakeups) {
  // It is often the case that Flutter does not wake up when it intends to due
  // to CPU contention. This test has VsyncWaiter wake up to schedule 0-4ns
  // after when |now| should be - and we verify that the results should be the
  // same as if there were no delay.
  const fml::TimeDelta vsync_offset = TimeDeltaFromInt(0);
  const fml::TimeDelta vsync_interval = TimeDeltaFromInt(10);

  fml::TimePoint last_targetted_vsync = TimePointFromInt(0);
  fml::TimePoint now = TimePointFromInt(5);
  fml::TimePoint next_vsync = TimePointFromInt(10);

  for (int i = 0; i < 100; ++i) {
    const auto target_times = flutter_runner::VsyncWaiter::GetTargetTimes(
        vsync_offset, vsync_interval, last_targetted_vsync, now, next_vsync);

    const auto target_times_delay = flutter_runner::VsyncWaiter::GetTargetTimes(
        vsync_offset, vsync_interval, last_targetted_vsync,
        now + TimeDeltaFromInt(i % 5), next_vsync);

    EXPECT_EQ(TimePointToInt(target_times.frame_start),
              TimePointToInt(target_times_delay.frame_start));
    EXPECT_EQ(TimePointToInt(target_times.frame_target),
              TimePointToInt(target_times_delay.frame_target));

    // Simulate the passage of time.
    now = now + vsync_interval;
    next_vsync = next_vsync + vsync_interval;
    last_targetted_vsync = target_times.frame_target;
  }
}

}  // namespace flutter_runner_test

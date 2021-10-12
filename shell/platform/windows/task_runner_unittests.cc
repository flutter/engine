// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <chrono>

#include "flutter/fml/time/time_point.h"

#include "flutter/shell/platform/windows/task_runner.h"

#include "gtest/gtest.h"

namespace flutter {
namespace testing {

namespace {
class MockTaskRunner : public TaskRunner {
 public:
  MockTaskRunner(CurrentTimeProc get_current_time,
                 const TaskExpiredCallback& on_task_expired)
      : TaskRunner(get_current_time, on_task_expired) {}

  virtual bool RunsTasksOnCurrentThread() const override { return true; }

  void DoProcessTasks() { ProcessTasks(); }

 protected:
  virtual void WakeUp() override {
    // Do nothing to avoid processing tasks immediately.
  }
};

uint64_t MockGetCurrentTime() {
  return static_cast<uint64_t>(
      fml::TimePoint::Now().ToEpochDelta().ToNanoseconds());
}
}  // namespace

TEST(TaskRunnerTest, MaybeExecuteTaskWithExactOrder) {
  std::vector<uint64_t> executed_order;
  auto runner = MockTaskRunner(
      MockGetCurrentTime, [&executed_order](const FlutterTask* expired_task) {
        executed_order.push_back(expired_task->task);
      });

  uint64_t time_1 = MockGetCurrentTime();
  runner.PostFlutterTask(FlutterTask{nullptr, 1}, time_1);
  runner.PostFlutterTask(FlutterTask{nullptr, 2}, time_1);
  runner.PostTask([&executed_order]() { executed_order.push_back(3); });
  runner.PostTask([&executed_order]() { executed_order.push_back(4); });

  runner.DoProcessTasks();

  std::vector<uint64_t> expect{1, 2, 3, 4};
  EXPECT_EQ(executed_order, expect);
}

TEST(TaskRunnerTest, MaybeExecuteTaskOnlyExpired) {
  std::vector<uint64_t> executed_order;
  auto runner = MockTaskRunner(
      MockGetCurrentTime, [&executed_order](const FlutterTask* expired_task) {
        executed_order.push_back(expired_task->task);
      });

  uint64_t time_1 = MockGetCurrentTime();
  runner.PostFlutterTask(FlutterTask{nullptr, 1}, time_1);
  runner.PostFlutterTask(FlutterTask{nullptr, 2}, time_1 + 100000);

  runner.DoProcessTasks();

  std::vector<uint64_t> expect{1};
  EXPECT_EQ(executed_order, expect);
}

// TEST(TaskRunnerTest, TimerAwakeAfterTheDuration) {}

}  // namespace testing
}  // namespace flutter

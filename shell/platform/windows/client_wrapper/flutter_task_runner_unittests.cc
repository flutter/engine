// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include <string>

#include "flutter/shell/platform/windows/client_wrapper/include/flutter/flutter_task_runner.h"
#include "flutter/shell/platform/windows/client_wrapper/testing/stub_flutter_windows_api.h"
#include "gtest/gtest.h"

namespace flutter {
namespace {

// Stub implementation to validate calls to the API.
class TestWindowsApi : public testing::StubFlutterWindowsApi {
 public:
  TestWindowsApi(bool runs_tasks_on_current_thread)
      : runs_tasks_on_current_thread_(runs_tasks_on_current_thread) {}

  void TaskRunnerPostTask(VoidCallback callback, void* user_data) override {
    post_task_called_ = true;
    callback(user_data);
  }

  bool TaskRunnerRunsTasksOnCurrentThread() override {
    return runs_tasks_on_current_thread_;
  }

  bool post_task_called() { return post_task_called_; }

 private:
  bool runs_tasks_on_current_thread_;
  bool post_task_called_ = false;
};

}  // namespace

TEST(FlutterTaskRunnerTest, RunsTasksOnCurrentThreadPassesThrough) {
  testing::ScopedStubFlutterWindowsApi scoped_api_stub(
      std::make_unique<TestWindowsApi>(false));
  auto test_api = static_cast<TestWindowsApi*>(scoped_api_stub.stub());
  FlutterTaskRunner task_runner(
      reinterpret_cast<FlutterDesktopTaskRunnerRef>(2));

  EXPECT_FALSE(task_runner.RunsTasksOnCurrentThread());
}

TEST(FlutterTaskRunnerTest, PostTaskInline) {
  testing::ScopedStubFlutterWindowsApi scoped_api_stub(
      std::make_unique<TestWindowsApi>(true));
  auto test_api = static_cast<TestWindowsApi*>(scoped_api_stub.stub());
  FlutterTaskRunner task_runner(
      reinterpret_cast<FlutterDesktopTaskRunnerRef>(2));

  bool task_executed = false;
  task_runner.PostTask([&task_executed]() { task_executed = true; });

  EXPECT_TRUE(task_runner.RunsTasksOnCurrentThread());
  EXPECT_TRUE(task_executed);
  EXPECT_FALSE(test_api->post_task_called());
}

TEST(FlutterTaskRunnerTest, PostTaskFromOtherThread) {
  testing::ScopedStubFlutterWindowsApi scoped_api_stub(
      std::make_unique<TestWindowsApi>(false));
  auto test_api = static_cast<TestWindowsApi*>(scoped_api_stub.stub());
  FlutterTaskRunner task_runner(
      reinterpret_cast<FlutterDesktopTaskRunnerRef>(2));

  bool task_executed = false;
  task_runner.PostTask([&task_executed]() { task_executed = true; });

  EXPECT_FALSE(task_runner.RunsTasksOnCurrentThread());
  EXPECT_TRUE(task_executed);
  EXPECT_TRUE(test_api->post_task_called());
}

}  // namespace flutter

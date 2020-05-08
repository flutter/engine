// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#define FML_USED_ON_EMBEDDER


#include <gtest/gtest.h>

#include <thread>
#include "flutter/fml/memory/task_runner_checker.h"

#include "flutter/fml/message_loop.h"
#include "flutter/fml/raster_thread_merger.h"
#include "flutter/fml/synchronization/count_down_latch.h"
#include "flutter/fml/synchronization/waitable_event.h"

namespace fml {
namespace testing {

TEST(TaskRunnerCheckerTests, RunsOnCurrentTaskRunner) {
  TaskRunnerChecker checker;
  EXPECT_EQ(checker.RunsOnCreationTaskRunner(), true);
}

TEST(TaskRunnerCheckerTests, FailsTheCheckIfOnDifferentTaskRunner) {
  TaskRunnerChecker checker;
  EXPECT_EQ(checker.RunsOnCreationTaskRunner(), true);
  fml::MessageLoop* loop;
  fml::AutoResetWaitableEvent latch;
  std::thread anotherThread([&]() {
    fml::MessageLoop::EnsureInitializedForCurrentThread();
    loop = &fml::MessageLoop::GetCurrent();
    loop->GetTaskRunner()->PostTask([&]() {
      EXPECT_EQ(checker.RunsOnCreationTaskRunner(), false);
      latch.Signal();
    });
    loop->Run();
  });
  latch.Wait();
  loop->Terminate();
  anotherThread.join();
  EXPECT_EQ(checker.RunsOnCreationTaskRunner(), true);
}

TEST(TaskRunnerCheckerTests, SameTaskRunnerRunsOnTheSameThread) {
  fml::MessageLoop& loop1 = fml::MessageLoop::GetCurrent();
  fml::MessageLoop& loop2 = fml::MessageLoop::GetCurrent();
  TaskQueueId a = loop1.GetTaskRunner()->GetTaskQueueId();
  TaskQueueId b = loop2.GetTaskRunner()->GetTaskQueueId();
  EXPECT_EQ(TaskRunnerChecker::RunsOnTheSameThread(a, b), true);
}

TEST(TaskRunnerCheckerTests, RunsOnDifferentThreadsReturnsFalse) {
  fml::MessageLoop& loop1 = fml::MessageLoop::GetCurrent();
  TaskQueueId a = loop1.GetTaskRunner()->GetTaskQueueId();
  fml::AutoResetWaitableEvent latch;
  std::thread anotherThread([&]() {
    fml::MessageLoop::EnsureInitializedForCurrentThread();
    fml::MessageLoop& loop2 = fml::MessageLoop::GetCurrent();
    TaskQueueId b = loop2.GetTaskRunner()->GetTaskQueueId();
    EXPECT_EQ(TaskRunnerChecker::RunsOnTheSameThread(a, b), false);
    latch.Signal();
  });
  latch.Wait();
  anotherThread.join();
}

}  // namespace testing
}  // namespace fml

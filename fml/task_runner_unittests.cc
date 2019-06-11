// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#define FML_USED_ON_EMBEDDER

#include "gtest/gtest.h"

#include "flutter/fml/mergeable_task_runner.h"
#include "flutter/fml/message_loop.h"
#include "flutter/fml/synchronization/count_down_latch.h"
#include "flutter/fml/synchronization/waitable_event.h"
#include "flutter/fml/thread.h"

TEST(MergeableTaskRunner, MergeMessageLoop) {
  std::unique_ptr<fml::Thread> thread_1 =
      std::make_unique<fml::Thread>("MergeableTaskRunner_Test.thread_1");
  std::unique_ptr<fml::Thread> thread_2 =
      std::make_unique<fml::Thread>("MergeableTaskRunner_Test.thread_2");

  auto task_runner_1 = thread_1->GetTaskRunner();
  auto task_runner_2 = thread_2->GetTaskRunner();
  fml::RefPtr<fml::MergeableTaskRunner> runner =
      fml::MergeableTaskRunner::Create(task_runner_1, task_runner_2);

  fml::AutoResetWaitableEvent latch_1, latch_2;

  runner->PostTask([&task_runner_1, &latch_1]() {
    ASSERT_TRUE(task_runner_1->RunsTasksOnCurrentThread());
    latch_1.Signal();
  });

  latch_1.Wait();
  runner->MergeLoops();

  runner->PostTask([&task_runner_2, &latch_2]() {
    ASSERT_TRUE(task_runner_2->RunsTasksOnCurrentThread());
    latch_2.Signal();
  });

  latch_2.Wait();
}

TEST(MergeableTaskRunner, SameMessageLoopMerge) {
  std::unique_ptr<fml::Thread> thread =
      std::make_unique<fml::Thread>("MergeableTaskRunner_Test.thread");

  auto task_runner = thread->GetTaskRunner();
  fml::RefPtr<fml::MergeableTaskRunner> runner =
      fml::MergeableTaskRunner::Create(task_runner, task_runner);

  fml::AutoResetWaitableEvent latch_1, latch_2;
  runner->PostTask([&task_runner, &latch_1]() {
    ASSERT_TRUE(task_runner->RunsTasksOnCurrentThread());
    latch_1.Signal();
  });

  latch_1.Wait();
  runner->MergeLoops();

  runner->PostTask([&task_runner, &latch_2]() {
    ASSERT_TRUE(task_runner->RunsTasksOnCurrentThread());
    latch_2.Signal();
  });

  latch_2.Wait();
}

TEST(MergeableTaskRunner, MergeLoopBlocks) {
  std::unique_ptr<fml::Thread> thread_1 =
      std::make_unique<fml::Thread>("MergeableTaskRunner_Test.thread_1");
  std::unique_ptr<fml::Thread> thread_2 =
      std::make_unique<fml::Thread>("MergeableTaskRunner_Test.thread_2");
  std::unique_ptr<fml::Thread> thread_3 =
      std::make_unique<fml::Thread>("MergeableTaskRunner_Test.thread_3");

  auto task_runner_1 = thread_1->GetTaskRunner();
  auto task_runner_2 = thread_2->GetTaskRunner();
  auto task_runner_3 = thread_3->GetTaskRunner();
  fml::RefPtr<fml::MergeableTaskRunner> runner =
      fml::MergeableTaskRunner::Create(task_runner_1, task_runner_2);

  int test_val = 0;
  fml::AutoResetWaitableEvent latch_1, latch_2, latch_3, latch_4;

  runner->PostTask([&latch_1, &latch_2, &test_val]() {
    test_val = 1;
    latch_1.Signal();
    latch_2.Wait();
    test_val = 2;
  });

  // test_val will be set to 1
  latch_1.Wait();
  ASSERT_TRUE(test_val == 1);

  // merge loop will be blocked until latch_2
  // is signalled.
  task_runner_3->PostTask([&runner, &latch_3, &test_val]() {
    runner->MergeLoops();
    ASSERT_TRUE(test_val == 2);
    test_val = 3;
    latch_3.Signal();
  });

  latch_2.Signal();
  latch_3.Wait();
  ASSERT_TRUE(test_val == 3);

  runner->PostTask([&latch_4, &test_val, &task_runner_2]() {
    ASSERT_TRUE(task_runner_2->RunsTasksOnCurrentThread());
    test_val = 4;
    latch_4.Signal();
  });

  latch_4.Wait();
  ASSERT_TRUE(test_val == 4);
}

TEST(MergeableTaskRunner, UnMerge) {
  std::unique_ptr<fml::Thread> thread_1 =
      std::make_unique<fml::Thread>("MergeableTaskRunner_Test.thread_1");
  std::unique_ptr<fml::Thread> thread_2 =
      std::make_unique<fml::Thread>("MergeableTaskRunner_Test.thread_2");

  auto task_runner_1 = thread_1->GetTaskRunner();
  auto task_runner_2 = thread_2->GetTaskRunner();
  fml::RefPtr<fml::MergeableTaskRunner> runner =
      fml::MergeableTaskRunner::Create(task_runner_1, task_runner_2);

  fml::AutoResetWaitableEvent latch_1, latch_2, latch_3;

  runner->PostTask([&task_runner_1, &latch_1]() {
    ASSERT_TRUE(task_runner_1->RunsTasksOnCurrentThread());
    latch_1.Signal();
  });

  latch_1.Wait();
  runner->MergeLoops();

  runner->PostTask([&task_runner_2, &latch_2]() {
    ASSERT_TRUE(task_runner_2->RunsTasksOnCurrentThread());
    latch_2.Signal();
  });

  latch_2.Wait();
  runner->UnMergeLoops();

  runner->PostTask([&task_runner_1, &latch_3]() {
    ASSERT_TRUE(task_runner_1->RunsTasksOnCurrentThread());
    latch_3.Signal();
  });

  latch_3.Wait();
}

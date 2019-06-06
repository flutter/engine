// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#define FML_USED_ON_EMBEDDER

#include "gtest/gtest.h"

#include "flutter/fml/message_loop.h"
#include "flutter/fml/msg_loop_reconfigurable_task_runner.h"
#include "flutter/fml/synchronization/count_down_latch.h"
#include "flutter/fml/synchronization/waitable_event.h"
#include "flutter/fml/thread.h"

TEST(MsgLoopReconfigurableTaskRunner, SwitchMessageLoop) {
  fml::MessageLoop::EnsureInitializedForCurrentThread();
  auto current_task_runner = fml::MessageLoop::GetCurrent().GetTaskRunner();

  std::unique_ptr<fml::Thread> thread_1 = std::make_unique<fml::Thread>(
      "MsgLoopReconfigurableTaskRunner_Test.thread_1");
  std::unique_ptr<fml::Thread> thread_2 = std::make_unique<fml::Thread>(
      "MsgLoopReconfigurableTaskRunner_Test.thread_2");

  auto task_runner_1 = thread_1->GetTaskRunner();
  auto task_runner_2 = thread_2->GetTaskRunner();
  fml::RefPtr<fml::MsgLoopReconfigurableTaskRunner> runner =
      fml::MsgLoopReconfigurableTaskRunner::Create(task_runner_1,
                                                   task_runner_2);

  fml::CountDownLatch latch(2);
  runner->PostTask([&task_runner_1, &latch]() {
    ASSERT_TRUE(task_runner_1->RunsTasksOnCurrentThread());
    latch.CountDown();
  });

  runner->SwitchMessageLoop();

  runner->PostTask([&task_runner_2, &latch]() {
    ASSERT_TRUE(task_runner_2->RunsTasksOnCurrentThread());
    latch.CountDown();
  });

  latch.Wait();
}

TEST(MsgLoopReconfigurableTaskRunner, SameMessageLoopSwitch) {
  fml::MessageLoop::EnsureInitializedForCurrentThread();
  auto current_task_runner = fml::MessageLoop::GetCurrent().GetTaskRunner();

  std::unique_ptr<fml::Thread> thread = std::make_unique<fml::Thread>(
      "MsgLoopReconfigurableTaskRunner_Test.thread");

  auto task_runner = thread->GetTaskRunner();
  fml::RefPtr<fml::MsgLoopReconfigurableTaskRunner> runner =
      fml::MsgLoopReconfigurableTaskRunner::Create(task_runner, task_runner);

  fml::CountDownLatch latch(2);
  runner->PostTask([&task_runner, &latch]() {
    ASSERT_TRUE(task_runner->RunsTasksOnCurrentThread());
    latch.CountDown();
  });

  runner->SwitchMessageLoop();

  runner->PostTask([&task_runner, &latch]() {
    ASSERT_TRUE(task_runner->RunsTasksOnCurrentThread());
    latch.CountDown();
  });

  latch.Wait();
}

TEST(MsgLoopReconfigurableTaskRunner, SwitchLoopBlocks) {
  fml::MessageLoop::EnsureInitializedForCurrentThread();
  auto current_task_runner = fml::MessageLoop::GetCurrent().GetTaskRunner();

  std::unique_ptr<fml::Thread> thread_1 = std::make_unique<fml::Thread>(
      "MsgLoopReconfigurableTaskRunner_Test.thread_1");
  std::unique_ptr<fml::Thread> thread_2 = std::make_unique<fml::Thread>(
      "MsgLoopReconfigurableTaskRunner_Test.thread_2");

  auto task_runner_1 = thread_1->GetTaskRunner();
  auto task_runner_2 = thread_2->GetTaskRunner();
  fml::RefPtr<fml::MsgLoopReconfigurableTaskRunner> runner =
      fml::MsgLoopReconfigurableTaskRunner::Create(task_runner_1,
                                                   task_runner_2);

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

  // switch message loop will be blocked until latch_2
  // is signalled.
  task_runner_2->PostTask([&test_val, &runner, &latch_3]() {
    runner->SwitchMessageLoop();
    ASSERT_TRUE(test_val == 2);
    test_val = 3;
    latch_3.Signal();
  });

  latch_2.Signal();
  latch_3.Wait();
  ASSERT_TRUE(test_val == 3);

  runner->PostTask([&latch_4, &test_val]() {
    test_val = 4;
    latch_4.Signal();
  });

  latch_4.Wait();
  ASSERT_TRUE(test_val == 4);
}

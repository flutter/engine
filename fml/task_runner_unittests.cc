// Copyright 2018 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#define FML_USED_ON_EMBEDDER

#include <thread>

#include "flutter/fml/message_loop.h"
#include "flutter/fml/synchronization/count_down_latch.h"
#include "flutter/fml/synchronization/waitable_event.h"
#include "flutter/fml/task_runner.h"
#include "flutter/fml/thread.h"
#include "flutter/fml/thread_local.h"
#include "flutter/testing/testing.h"

namespace fml {

TEST(TaskRunnerTest, RunNowOrPostTaskOnDifferentThread) {
  FML_THREAD_LOCAL ThreadLocal count;
  Thread thread;

  thread.GetTaskRunner()->PostTask([]() { count.Set(999); });

  ASSERT_NE(count.Get(), 999);

  bool did_assert = false;

  AutoResetWaitableEvent latch;
  thread.GetTaskRunner()->RunNowOrPostTask([&did_assert, &latch]() {
    // Make sure the utility method created its own activation.
    ASSERT_EQ(MessageLoop::GetCurrent().GetActivationCount(), 2u);
    ASSERT_EQ(count.Get(), 999);
    did_assert = true;
    latch.Signal();
  });

  latch.Wait();
  ASSERT_TRUE(did_assert);
}

TEST(TaskRunnerTest, RunNowOrPostTaskOnSameThread) {
  FML_THREAD_LOCAL ThreadLocal count;
  Thread thread;

  thread.GetTaskRunner()->PostTask([]() { count.Set(999); });

  ASSERT_NE(count.Get(), 999);

  size_t assertions_checked = 0;

  AutoResetWaitableEvent latch;
  thread.GetTaskRunner()->PostTask([&assertions_checked, &latch]() {
    // No new activation is pused because it is not a RunNow variant.
    ASSERT_EQ(MessageLoop::GetCurrent().GetActivationCount(), 1u);
    ASSERT_EQ(count.Get(), 999);
    assertions_checked++;
    latch.Signal();

    MessageLoop::GetCurrent().GetTaskRunner()->RunNowOrPostTask(
        [&assertions_checked]() {
          // No new activation is pushed because we are already on the right
          // thread.
          ASSERT_EQ(MessageLoop::GetCurrent().GetActivationCount(), 1u);
          ASSERT_EQ(count.Get(), 999);
          assertions_checked++;
        });
  });

  latch.Wait();
  ASSERT_EQ(assertions_checked, 2u);
}

}  // namespace fml

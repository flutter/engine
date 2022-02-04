// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/fml/thread.h"

#define FLUTTER_PTHREAD_SUPPORTED \
  defined(OS_MACOSX) || defined(OS_LINUX) || defined(OS_ANDROID)

#ifdef FLUTTER_PTHREAD_SUPPORTED
#include <pthread.h>
#else
#error "Doesn't has pthead.h"
#endif

#include <memory>
#include "gtest/gtest.h"

TEST(Thread, CanStartAndEnd) {
  fml::Thread thread;
  ASSERT_TRUE(thread.GetTaskRunner());
}

TEST(Thread, CanStartAndEndWithExplicitJoin) {
  fml::Thread thread;
  ASSERT_TRUE(thread.GetTaskRunner());
  thread.Join();
}

TEST(Thread, HasARunningMessageLoop) {
  fml::Thread thread;
  bool done = false;
  thread.GetTaskRunner()->PostTask([&done]() { done = true; });
  thread.Join();
  ASSERT_TRUE(done);
}

#ifdef FLUTTER_PTHREAD_SUPPORTED
TEST(Thread, ThreadNameCreatedWithConfig) {
  const std::string name = "Thread1";
  fml::Thread thread(fml::Thread::ThreadConfig::MakeDefaultConfigure(name));

  bool done = false;
  constexpr int NAMELEN = 8;
  thread.GetTaskRunner()->PostTask([&done, &name]() {
    done = true;
    char thread_name[NAMELEN];
    pthread_t current_thread = pthread_self();
    pthread_getname_np(current_thread, thread_name, NAMELEN);
    ASSERT_EQ(thread_name, name);
  });
  thread.Join();
  ASSERT_TRUE(done);
}

class MockThreadConfig : public fml::Thread::ThreadConfig {
 public:
  using fml::Thread::ThreadConfig::ThreadConfig;

  void SetCurrentThreadPriority() const override {
    pthread_t tid = pthread_self();
    struct sched_param param;
    int policy = SCHED_OTHER;
    switch (GetThreadPriority()) {
      case fml::Thread::ThreadPriority::DISPLAY:
        param.sched_priority = 10;
        break;
      default:
        param.sched_priority = 1;
    }
    pthread_setschedparam(tid, policy, &param);
  }
};

TEST(Thread, ThreadPriorityCreatedWithConfig) {
  const std::string thread1_name = "Thread1";
  const std::string thread2_name = "Thread2";
  fml::Thread thread(std::make_unique<MockThreadConfig>(
      thread1_name, fml::Thread::ThreadPriority::NORMAL));

  bool done = false;
  constexpr int NAMELEN = 8;
  struct sched_param param;
  int policy;
  thread.GetTaskRunner()->PostTask([&]() {
    done = true;
    char thread_name[NAMELEN];
    pthread_t current_thread = pthread_self();
    pthread_getname_np(current_thread, thread_name, NAMELEN);
    pthread_getschedparam(current_thread, &policy, &param);
    ASSERT_EQ(thread_name, thread1_name);
    ASSERT_EQ(policy, SCHED_OTHER);
    ASSERT_EQ(param.sched_priority, 1);
  });

  fml::Thread thread2(std::make_unique<MockThreadConfig>(
      thread2_name, fml::Thread::ThreadPriority::DISPLAY));
  thread2.GetTaskRunner()->PostTask([&]() {
    done = true;
    char thread_name[NAMELEN];
    pthread_t current_thread = pthread_self();
    pthread_getname_np(current_thread, thread_name, NAMELEN);
    pthread_getschedparam(current_thread, &policy, &param);
    ASSERT_EQ(thread_name, thread2_name);
    ASSERT_EQ(policy, SCHED_OTHER);
    ASSERT_EQ(param.sched_priority, 10);
  });
  thread.Join();
  ASSERT_TRUE(done);
}
#endif

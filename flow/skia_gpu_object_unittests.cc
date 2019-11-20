// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/skia_gpu_object.h"

#include "flutter/fml/message_loop.h"
#include "flutter/fml/synchronization/waitable_event.h"
#include "flutter/fml/task_runner.h"
#include "flutter/testing/thread_test.h"
#include "gtest/gtest.h"
#include "third_party/skia/include/core/SkRefCnt.h"

namespace flutter {
namespace testing {

class TestSkObject : public SkRefCnt {
 public:
  TestSkObject(std::shared_ptr<fml::AutoResetWaitableEvent> latch,
               fml::TaskQueueId* dtor_task_queue_id)
      : latch_(latch), dtor_task_queue_id_(dtor_task_queue_id) {}

  ~TestSkObject() {
    if (dtor_task_queue_id_) {
      *dtor_task_queue_id_ = fml::MessageLoop::GetCurrentTaskQueueId();
    }
    latch_->Signal();
  }

 private:
  std::shared_ptr<fml::AutoResetWaitableEvent> latch_;
  fml::TaskQueueId* dtor_task_queue_id_;
};

// Death tests and threads do not get along.  This fixture gives access to the
// current thread context but doesn't start any other threads.
using SkiaGpuObjectDeathTest = ThreadTest;

// This fixture starts a 2nd thread by default.  The |SkiaUnrefQueue| uses the
// 2nd thread to perform unref() operations on its data items.
class SkiaGpuObjectTest : public SkiaGpuObjectDeathTest {
 public:
  SkiaGpuObjectTest()
      : unref_task_runner_(CreateNewThread()),
        unref_queue_(fml::MakeRefCounted<SkiaUnrefQueue>(
            unref_task_runner(),
            fml::TimeDelta::FromSeconds(0))),
        delayed_unref_queue_(fml::MakeRefCounted<SkiaUnrefQueue>(
            unref_task_runner(),
            fml::TimeDelta::FromSeconds(3))) {}
  ~SkiaGpuObjectTest() override = default;

  fml::RefPtr<fml::TaskRunner> unref_task_runner() {
    return unref_task_runner_;
  }
  fml::RefPtr<SkiaUnrefQueue> unref_queue() { return unref_queue_; }
  fml::RefPtr<SkiaUnrefQueue> delayed_unref_queue() {
    return delayed_unref_queue_;
  }

 private:
  fml::RefPtr<fml::TaskRunner> unref_task_runner_;
  fml::RefPtr<SkiaUnrefQueue> unref_queue_;
  fml::RefPtr<SkiaUnrefQueue> delayed_unref_queue_;
};

TEST_F(SkiaGpuObjectDeathTest, CreateObjectWithNoQueueDies) {
  SkiaGPUObject<TestSkObject> object;

  EXPECT_DEATH_IF_SUPPORTED(
      object = SkiaGPUObject<TestSkObject>(
          sk_make_sp<TestSkObject>(
              std::make_shared<fml::AutoResetWaitableEvent>(), nullptr),
          nullptr),
      "queue_ && object_");
}

TEST_F(SkiaGpuObjectDeathTest, CreateObjectWithNoSkSpDies) {
  SkiaGPUObject<TestSkObject> object;
  auto unref_queue = fml::MakeRefCounted<SkiaUnrefQueue>(
      GetCurrentTaskRunner(), fml::TimeDelta::FromSeconds(0));

  EXPECT_DEATH_IF_SUPPORTED(
      object = SkiaGPUObject<TestSkObject>(nullptr, unref_queue),
      "queue_ && object_");
}

TEST_F(SkiaGpuObjectTest, QueueSimple) {
  std::shared_ptr<fml::AutoResetWaitableEvent> latch =
      std::make_shared<fml::AutoResetWaitableEvent>();
  fml::TaskQueueId dtor_task_queue_id(0);
  SkRefCnt* ref_object = new TestSkObject(latch, &dtor_task_queue_id);

  unref_queue()->Unref(ref_object);
  latch->Wait();
  ASSERT_EQ(dtor_task_queue_id, unref_task_runner()->GetTaskQueueId());
}

TEST_F(SkiaGpuObjectTest, ObjectDestructor) {
  std::shared_ptr<fml::AutoResetWaitableEvent> latch =
      std::make_shared<fml::AutoResetWaitableEvent>();
  fml::TaskQueueId dtor_task_queue_id(0);

  {
    auto object = sk_make_sp<TestSkObject>(latch, &dtor_task_queue_id);
    SkiaGPUObject<TestSkObject> sk_object(object, unref_queue());
    ASSERT_EQ(sk_object.get(), object);
    ASSERT_EQ(dtor_task_queue_id, 0);
  }

  latch->Wait();
  ASSERT_EQ(dtor_task_queue_id, unref_task_runner()->GetTaskQueueId());
}

TEST_F(SkiaGpuObjectTest, ObjectReset) {
  std::shared_ptr<fml::AutoResetWaitableEvent> latch =
      std::make_shared<fml::AutoResetWaitableEvent>();
  fml::TaskQueueId dtor_task_queue_id(0);
  SkiaGPUObject<TestSkObject> sk_object(
      sk_make_sp<TestSkObject>(latch, &dtor_task_queue_id), unref_queue());

  sk_object.reset();
  ASSERT_EQ(sk_object.get(), nullptr);

  latch->Wait();
  ASSERT_EQ(dtor_task_queue_id, unref_task_runner()->GetTaskQueueId());
}

TEST_F(SkiaGpuObjectTest, ObjectResetBeforeDestructor) {
  std::shared_ptr<fml::AutoResetWaitableEvent> latch =
      std::make_shared<fml::AutoResetWaitableEvent>();
  fml::TaskQueueId dtor_task_queue_id(0);

  {
    auto object = sk_make_sp<TestSkObject>(latch, &dtor_task_queue_id);
    SkiaGPUObject<TestSkObject> sk_object(object, unref_queue());
    ASSERT_EQ(sk_object.get(), object);
    ASSERT_EQ(dtor_task_queue_id, 0);

    sk_object.reset();
    ASSERT_EQ(sk_object.get(), nullptr);
  }

  latch->Wait();
  ASSERT_EQ(dtor_task_queue_id, unref_task_runner()->GetTaskQueueId());
}

TEST_F(SkiaGpuObjectTest, ObjectResetTwice) {
  std::shared_ptr<fml::AutoResetWaitableEvent> latch =
      std::make_shared<fml::AutoResetWaitableEvent>();
  fml::TaskQueueId dtor_task_queue_id(0);
  SkiaGPUObject<TestSkObject> sk_object(
      sk_make_sp<TestSkObject>(latch, &dtor_task_queue_id), unref_queue());

  sk_object.reset();
  ASSERT_EQ(sk_object.get(), nullptr);
  sk_object.reset();
  ASSERT_EQ(sk_object.get(), nullptr);

  latch->Wait();
  ASSERT_EQ(dtor_task_queue_id, unref_task_runner()->GetTaskQueueId());
}

}  // namespace testing
}  // namespace flutter

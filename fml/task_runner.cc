// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#define FML_USED_ON_EMBEDDER

#include "flutter/fml/task_runner.h"

#include <utility>

#include "flutter/fml/logging.h"
#include "flutter/fml/message_loop.h"
#include "flutter/fml/message_loop_impl.h"
#include "flutter/fml/synchronization/waitable_event.h"

namespace fml {

TaskRunner::TaskRunner(fml::RefPtr<MessageLoop> loop)
    : loop_(std::move(loop)) {}

TaskRunner::~TaskRunner() = default;

void TaskRunner::PostTask(fml::closure task) {
  loop_->GetLoopImpl()->PostTask(std::move(task), fml::TimePoint::Now());
}

void TaskRunner::PostTaskForTime(fml::closure task,
                                 fml::TimePoint target_time) {
  loop_->GetLoopImpl()->PostTask(std::move(task), target_time);
}

void TaskRunner::PostDelayedTask(fml::closure task, fml::TimeDelta delay) {
  loop_->GetLoopImpl()->PostTask(std::move(task),
                                 fml::TimePoint::Now() + delay);
}

bool TaskRunner::RunsTasksOnCurrentThread() {
  if (!fml::MessageLoop::IsInitializedForCurrentThread()) {
    return false;
  }
  return &MessageLoop::GetCurrent() == loop_.get();
}

void TaskRunner::RunNowOrPostTask(fml::closure task) {
  FML_DCHECK(task);

  if (RunsTasksOnCurrentThread()) {
    task();
    return;
  }

  AutoResetWaitableEvent latch;

  // Legacy path for platforms on which we do not have access to the message
  // loop implementation (Fuchsia and Desktop Linux).
  if (!loop_) {
    PostTask([task, &latch]() {
      task();
      latch.Signal();
    });
    latch.Wait();
    return;
  }

  auto task_in_loop_activation = [loop = loop_, task, &latch]() {
    loop->Run([loop, task, &latch]() {
      task();
      loop->Terminate();
      latch.Signal();
    });
  };

  PostTask(task_in_loop_activation);
  latch.Wait();
}

}  // namespace fml

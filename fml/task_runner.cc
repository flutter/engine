// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#define FML_USED_ON_EMBEDDER

#include "flutter/fml/task_runner.h"

#include <utility>

#include "flutter/fml/logging.h"
#include "flutter/fml/message_loop.h"
#include "flutter/fml/message_loop_impl.h"

namespace fml {

TaskRunner::TaskRunner(fml::RefPtr<MessageLoopImpl> loop)
    : loop_(std::move(loop)) {}

TaskRunner::~TaskRunner() = default;

void TaskRunner::PostTask(fml::closure task) {
  loop_->PostTask(std::move(task), fml::TimePoint::Now());
}

void TaskRunner::PostTaskForTime(fml::closure task,
                                 fml::TimePoint target_time) {
  loop_->PostTask(std::move(task), target_time);
}

void TaskRunner::PostDelayedTask(fml::closure task, fml::TimeDelta delay) {
  loop_->PostTask(std::move(task), fml::TimePoint::Now() + delay);
}

TaskQueueId TaskRunner::GetTaskQueueId() {
  FML_DCHECK(loop_);
  return loop_->GetTaskQueueId();
}

bool TaskRunner::RunsTasksOnCurrentThread() {
  if (!fml::MessageLoop::IsInitializedForCurrentThread()) {
    return false;
  }
  const auto current_loop_id =
      MessageLoop::GetCurrent().GetLoopImpl()->GetTaskQueueId();
  return current_loop_id == loop_->GetTaskQueueId();
}

void TaskRunner::RunNowOrPostTask(fml::RefPtr<fml::TaskRunner> runner,
                                  fml::closure task) {
  FML_DCHECK(runner);
  if (runner->RunsTasksOnCurrentThread()) {
    task();
  } else {
    runner->PostTask(std::move(task));
  }
}

}  // namespace fml

// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#define FML_USED_ON_EMBEDDER

#include "flutter/fml/msg_loop_reconfigurable_task_runner.h"
#include "flutter/fml/message_loop_impl.h"
#include "flutter/fml/synchronization/waitable_event.h"

namespace fml {

RefPtr<MsgLoopReconfigurableTaskRunner>
MsgLoopReconfigurableTaskRunner::CreateFromSingleTaskRunner(
    const RefPtr<TaskRunner>& task_runner) {
  if (!task_runner) {
    return nullptr;
  }
  auto loop = task_runner->loop_;
  return MakeRefCounted<MsgLoopReconfigurableTaskRunner>(loop, loop);
}

RefPtr<MsgLoopReconfigurableTaskRunner> MsgLoopReconfigurableTaskRunner::Create(
    const RefPtr<TaskRunner>& task_runner_1,
    const RefPtr<TaskRunner>& task_runner_2) {
  if (!task_runner_1 || !task_runner_2) {
    return nullptr;
  }
  auto loop_1 = task_runner_1->loop_;
  auto loop_2 = task_runner_2->loop_;
  return MakeRefCounted<MsgLoopReconfigurableTaskRunner>(loop_1, loop_2);
}

MsgLoopReconfigurableTaskRunner::MsgLoopReconfigurableTaskRunner(
    const fml::RefPtr<MessageLoopImpl>& loop_1,
    const fml::RefPtr<MessageLoopImpl>& loop_2)
    : TaskRunner(nullptr /* loop implemenation*/),
      shared_mutex_(fml::SharedMutex::Create()),
      merged_(false) {
  loops_[0] = fml::RefPtr<MessageLoopImpl>(loop_1);
  loops_[1] = fml::RefPtr<MessageLoopImpl>(loop_2);
}

MsgLoopReconfigurableTaskRunner::~MsgLoopReconfigurableTaskRunner() = default;

void MsgLoopReconfigurableTaskRunner::PostTask(fml::closure task) {
  PostTaskForTime(task, fml::TimePoint::Now());
}

void MsgLoopReconfigurableTaskRunner::PostTaskForTime(
    fml::closure task,
    fml::TimePoint target_time) {
  if (!task) {
    return;
  }
  fml::SharedLock lock(*shared_mutex_);
  int cur_loop = merged_ ? 1 : 0;
  loops_[cur_loop]->PostTask(std::move(task), target_time);
}

void MsgLoopReconfigurableTaskRunner::PostDelayedTask(fml::closure task,
                                                      fml::TimeDelta delay) {
  PostTaskForTime(task, fml::TimePoint::Now() + delay);
}

bool MsgLoopReconfigurableTaskRunner::RunsTasksOnCurrentThread() {
  if (!fml::MessageLoop::IsInitializedForCurrentThread()) {
    return false;
  }
  fml::SharedLock lock(*shared_mutex_);
  int cur_loop = merged_ ? 1 : 0;
  return MessageLoop::GetCurrent().GetLoopImpl() == loops_[cur_loop];
}

void MsgLoopReconfigurableTaskRunner::MergeLoops() {
  fml::UniqueLock lock(*shared_mutex_);
  if (merged_) {
    return;
  }
  loops_[1]->InheritAllTasks(loops_[0]);
  merged_ = true;
}

void MsgLoopReconfigurableTaskRunner::UnMergeLoops() {
  fml::UniqueLock lock(*shared_mutex_);
  merged_ = false;
}

}  // namespace fml

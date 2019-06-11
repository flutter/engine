// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FML_MERGEABLE_TASK_RUNNER_H_
#define FLUTTER_FML_MERGEABLE_TASK_RUNNER_H_

#include "flutter/fml/synchronization/shared_mutex.h"
#include "flutter/fml/task_runner.h"

namespace fml {

// TaskRunner that allows for merging and un-merging loops.
// We currently support instantiating this with two configurations, backed
// by the same task runner or backed by two task runners.
//
// Threading: active loop, determined by merged_ is guarded by a shared
// mutex. When MergeLoops gets called, we move the tasks in loop_1
// to loop_2.
//
// Note: Unmerge does not re-transfer the ownership of prior submissions.
// Only the tasks that are submitted in the future are assigned to loop_1.
// We start with an empty task queue for loop_1 after Merge -> UnMerge.
class MergeableTaskRunner : public TaskRunner {
 public:
  // Both loops are backed by the task_runner's loop.
  static RefPtr<MergeableTaskRunner> CreateFromSingleTaskRunner(
      const RefPtr<TaskRunner>& task_runner);

  static RefPtr<MergeableTaskRunner> Create(
      const RefPtr<TaskRunner>& task_runner_1,
      const RefPtr<TaskRunner>& task_runner_2);

  ~MergeableTaskRunner() override;

  // We will default to loop_1 to execute tasks.
  // Use |MergeableTaskRunner::SwitchLoop| to change
  // the active loop.
  MergeableTaskRunner(const RefPtr<MessageLoopImpl>& loop_1,
                      const RefPtr<MessageLoopImpl>& loop_2);

  // |TaskRunner|
  void PostTask(closure task) override;

  // |TaskRunner|
  void PostTaskForTime(closure task, TimePoint target_time) override;

  // |TaskRunner|
  void PostDelayedTask(closure task, TimeDelta delay) override;

  // |TaskRunner|
  bool RunsTasksOnCurrentThread() override;

  // Changes the active message loop.
  void MergeLoops();

  void UnMergeLoops();

 private:
  std::unique_ptr<fml::SharedMutex> shared_mutex_;
  std::atomic_bool merged_;  // guarded by shared_mutex_
  RefPtr<MessageLoopImpl> loops_[2];

  FML_DISALLOW_COPY_AND_ASSIGN(MergeableTaskRunner);
};

}  // namespace fml

#endif  // FLUTTER_FML_MERGEABLE_TASK_RUNNER_H_

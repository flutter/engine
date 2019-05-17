// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FML_MSG_LOOP_RECONFIGURABLE_TASK_RUNNER_H_
#define FLUTTER_FML_MSG_LOOP_RECONFIGURABLE_TASK_RUNNER_H_

#include "flutter/fml/synchronization/shared_mutex.h"
#include "flutter/fml/task_runner.h"

namespace fml {

// TaskRunner that allows for "switching" message loops backing it.
// We currently support instantiating this with two configurations, backed
// by the same task runner or backed by two task runners.
//
// Threading: active loop, indexed by current_loop_ is guarded by a shared
// mutex. When SwitchLoop gets called, we wait for all the expired tasks
// on the active loop to flush, then we switch the loop.
class MsgLoopReconfigurableTaskRunner : public TaskRunner {
 public:
  // Both loops are backed by the task_runner's loop.
  static RefPtr<MsgLoopReconfigurableTaskRunner> CreateFromSingleTaskRunner(
      const RefPtr<TaskRunner>& task_runner);

  static RefPtr<MsgLoopReconfigurableTaskRunner> Create(
      const RefPtr<TaskRunner>& task_runner_1,
      const RefPtr<TaskRunner>& task_runner_2);

  ~MsgLoopReconfigurableTaskRunner() override;

  // We will default to loop_1 to execute tasks.
  // Use |MsgLoopReconfigurableTaskRunner::SwitchLoop| to change
  // the active loop.
  MsgLoopReconfigurableTaskRunner(const RefPtr<MessageLoopImpl>& loop_1,
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
  void SwitchMessageLoop();

 private:
  std::unique_ptr<fml::SharedMutex> shared_mutex_;
  int current_loop_;
  RefPtr<MessageLoopImpl> loops_[2];

  FML_DISALLOW_COPY_AND_ASSIGN(MsgLoopReconfigurableTaskRunner);
};

}  // namespace fml

#endif  // FLUTTER_FML_MSG_LOOP_RECONFIGURABLE_TASK_RUNNER_H_

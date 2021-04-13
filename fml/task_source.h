// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FML_TASK_SOURCE_H_
#define FLUTTER_FML_TASK_SOURCE_H_

#include "flutter/fml/delayed_task.h"
#include "flutter/fml/task_queue_id.h"
#include "flutter/fml/task_source_grade.h"

namespace fml {

class MessageLoopTaskQueues;

class TaskSource {
 public:
  struct TopTask {
    TaskQueueId task_queue_id;
    const DelayedTask& task;
  };

  explicit TaskSource(TaskQueueId task_queue_id);

  ~TaskSource();

  void ShutDown();

  void RegisterTask(const DelayedTask& task);

  void PopTask(TaskSourceGrade grade);

  size_t GetNumPendingTasks() const;

  bool IsEmpty() const;

  TopTask Top() const;

 private:
  friend class MessageLoopTaskQueues;

  void PauseSecondary();

  void ResumeSecondary();

  const fml::TaskQueueId task_queue_id_;
  fml::DelayedTaskQueue primary_task_queue_;
  fml::DelayedTaskQueue secondary_task_queue_;
  int secondary_pause_requests_ = 0;

  FML_DISALLOW_COPY_ASSIGN_AND_MOVE(TaskSource);
};

}  // namespace fml

#endif  // FLUTTER_FML_TASK_SOURCE_H_

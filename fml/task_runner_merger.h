// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FML_SHELL_COMMON_TASK_RUNNER_MERGER_H_
#define FML_SHELL_COMMON_TASK_RUNNER_MERGER_H_

#include "flutter/fml/macros.h"
#include "flutter/fml/memory/ref_counted.h"
#include "flutter/fml/message_loop_task_queues.h"

namespace fml {

class MessageLoopImpl;

class TaskRunnerMerger : public fml::RefCountedThreadSafe<TaskRunnerMerger> {
 public:
  void MergeWithLease(size_t lease_term);

  void ExtendLease(size_t lease_term);

  // Returns true if we unmerged this turn.
  bool DecrementLease();

  bool AreMerged() const;

  TaskRunnerMerger(fml::TaskQueueId platform_queue_id,
                   fml::TaskQueueId gpu_queue_id);

  bool OnWrongThread();

 private:
  static const int kLeaseNotSet;
  fml::TaskQueueId platform_queue_id_;
  fml::TaskQueueId gpu_queue_id_;
  fml::RefPtr<fml::MessageLoopTaskQueues> task_queues_;
  std::atomic_int lease_term_;
  bool is_merged_;

  FML_FRIEND_REF_COUNTED_THREAD_SAFE(TaskRunnerMerger);
  FML_FRIEND_MAKE_REF_COUNTED(TaskRunnerMerger);
  FML_DISALLOW_COPY_AND_ASSIGN(TaskRunnerMerger);
};

}  // namespace fml

#endif  // FML_SHELL_COMMON_TASK_RUNNER_MERGER_H_

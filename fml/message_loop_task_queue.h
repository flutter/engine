// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FML_MESSAGE_LOOP_TASK_QUEUE_H_
#define FLUTTER_FML_MESSAGE_LOOP_TASK_QUEUE_H_

#include <atomic>
#include <deque>
#include <map>
#include <mutex>
#include <queue>
#include <utility>

#include "flutter/fml/closure.h"
#include "flutter/fml/logging.h"
#include "flutter/fml/macros.h"
#include "flutter/fml/synchronization/thread_annotations.h"
#include "flutter/fml/time/time_point.h"

typedef size_t MessageLoopId;

namespace fml {

struct DelayedTask {
  int order;
  fml::closure task;
  fml::TimePoint target_time;

  DelayedTask(int p_order, fml::closure p_task, fml::TimePoint p_target_time);

  DelayedTask(const DelayedTask& other);

  ~DelayedTask();
};

enum class FlushType {
  kSingle,
  kAll,
};

class MessageLoopTaskQueue {
 public:
  struct TasksToRun {
    fml::TimePoint wake_up_time;
    std::vector<fml::closure> invocations;
    std::vector<MessageLoopId> loop_ids;

    TasksToRun(fml::TimePoint p_wake_up,
               std::vector<fml::closure> p_invocations,
               std::vector<MessageLoopId> p_loop_ids);

    ~TasksToRun();
  };

  static MessageLoopTaskQueue* GetInstance();

  MessageLoopId CreateMessageLoopId();

  fml::TimePoint RegisterTask(MessageLoopId loop_id,
                              fml::closure task,
                              fml::TimePoint target_time);

  void AddTaskObserver(MessageLoopId loop_id,
                       intptr_t key,
                       fml::closure callback);

  void RemoveTaskObserver(MessageLoopId loop_id, intptr_t key);

  bool MergeQueues(MessageLoopId owner, MessageLoopId subsumed);

  bool UnmergeQueues(MessageLoopId owner);

  void DisposeTasks(MessageLoopId owner);

  TasksToRun GetTasksToRunNow(MessageLoopId owner, FlushType flush_type);

  void InvokeAndNotifyObservers(MessageLoopId owner,
                                const TasksToRun& tasks_to_run);

  std::vector<std::mutex> flush_tasks_mutexes;

 private:
  static std::mutex creation_mutex_;
  static MessageLoopTaskQueue* instance_;

  MessageLoopTaskQueue();

  ~MessageLoopTaskQueue();

  bool HasMoreTasks(MessageLoopId owner);

  DelayedTask PeekNextTask(MessageLoopId owner, MessageLoopId& loop);

  std::atomic_int message_loop_id_counter_;

  std::mutex merge_mutex_;

  std::map<MessageLoopId, MessageLoopId> owner_to_subsumed_;
  std::map<MessageLoopId, MessageLoopId> subsumed_to_owner_;

  struct DelayedTaskCompare {
    bool operator()(const DelayedTask& a, const DelayedTask& b) {
      return a.target_time == b.target_time ? a.order > b.order
                                            : a.target_time > b.target_time;
    }
  };

  using DelayedTaskQueue = std::
      priority_queue<DelayedTask, std::deque<DelayedTask>, DelayedTaskCompare>;
  using TaskObservers = std::map<intptr_t, fml::closure>;

  std::vector<std::mutex> observers_mutexes_;
  std::vector<TaskObservers> task_observers_;

  std::vector<std::mutex> delayed_tasks_mutexes_;
  std::vector<DelayedTaskQueue> delayed_tasks_;

  std::atomic_int order_;

  FML_DISALLOW_COPY_ASSIGN_AND_MOVE(MessageLoopTaskQueue);
};

}  // namespace fml

#endif
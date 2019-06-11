// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#define FML_USED_ON_EMBEDDER

#include "flutter/fml/message_loop_task_queue.h"

namespace fml {

// delayed tasks
DelayedTask::DelayedTask(int p_order,
                         fml::closure p_task,
                         fml::TimePoint p_target_time)
    : order(p_order), task(std::move(p_task)), target_time(p_target_time) {}

DelayedTask::DelayedTask(const DelayedTask& other) = default;

DelayedTask::~DelayedTask() = default;

MessageLoopTaskQueue::MessageLoopTaskQueue()
    : message_loop_id_counter_(0), order_(0) {}

MessageLoopTaskQueue::~MessageLoopTaskQueue() = default;

MessageLoopId MessageLoopTaskQueue::CreateMessageLoopId() {
  std::lock_guard<std::mutex> creation(creation_mutex_);
  MessageLoopId loop_id = message_loop_id_counter_;
  ++message_loop_id_counter_;

  observers_mutexes_.push_back(std::mutex());
  delayed_tasks_mutexes_.push_back(std::mutex());

  task_observers_.push_back(TaskObservers());
  delayed_tasks_.push_back(DelayedTaskQueue());

  return loop_id;
}

fml::TimePoint MessageLoopTaskQueue::RegisterTask(MessageLoopId loop_id,
                                                  fml::closure task,
                                                  fml::TimePoint target_time) {
  std::lock_guard<std::mutex> lock(delayed_tasks_mutexes_[loop_id]);
  delayed_tasks_[loop_id].push({++order_, std::move(task), target_time});
  return delayed_tasks_[loop_id].top().target_time;
}

bool MessageLoopTaskQueue::MergeQueues(MessageLoopId owner,
                                       MessageLoopId subsumed) {
  std::lock_guard<std::mutex> lock(merge_mutex_);

  // these are already merged.
  if (owner_to_subsumed_[owner] == subsumed) {
    return subsumed_to_owner_[subsumed] == owner;
  }

  // these are already merged with some other things.
  if (owner_to_subsumed_.count(owner) || subsumed_to_owner_.count(subsumed)) {
    return false;
  }

  // block any running tasks
  // no more task adding or removing
  // no more task observers

  owner_to_subsumed_[owner] = subsumed;
  subsumed_to_owner_[subsumed] = owner;

  return true;
}

bool MessageLoopTaskQueue::UnmergeQueues(MessageLoopId owner) {
  std::lock_guard<std::mutex> lock(merge_mutex_);

  // is not merged.
  if (owner_to_subsumed_.count(owner) == 0) {
    return true;
  }

  // block any running tasks
  // no more task adding or removing
  // no more task observers

  MessageLoopId subsumed = owner_to_subsumed_[owner];
  owner_to_subsumed_.erase(owner);
  subsumed_to_owner_.erase(subsumed);

  return true;
}


}  // namespace fml
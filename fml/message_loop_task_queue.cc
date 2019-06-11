// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#define FML_USED_ON_EMBEDDER

#include "flutter/fml/message_loop_task_queue.h"

namespace fml {

std::mutex MessageLoopTaskQueue::creation_mutex_;
fml::RefPtr<MessageLoopTaskQueue> MessageLoopTaskQueue::instance_;

// delayed tasks
DelayedTask::DelayedTask(int p_order,
                         fml::closure p_task,
                         fml::TimePoint p_target_time)
    : order(p_order), task(std::move(p_task)), target_time(p_target_time) {}

DelayedTask::DelayedTask(const DelayedTask& other) = default;

DelayedTask::~DelayedTask() = default;

fml::RefPtr<MessageLoopTaskQueue> MessageLoopTaskQueue::GetInstance() {
  std::lock_guard<std::mutex> creation(creation_mutex_);
  if (!instance_) {
    instance_ = fml::MakeRefCounted<MessageLoopTaskQueue>();
  }
  return instance_;
}

MessageLoopTaskQueue::MessageLoopTaskQueue()
    : message_loop_id_counter_(0), order_(0) {}

MessageLoopTaskQueue::~MessageLoopTaskQueue() = default;

MessageLoopId MessageLoopTaskQueue::CreateMessageLoopId() {
  std::lock_guard<std::mutex> creation(creation_mutex_);
  MessageLoopId loop_id = message_loop_id_counter_;
  ++message_loop_id_counter_;

  observers_mutexes_.push_back(std::make_unique<std::mutex>());
  delayed_tasks_mutexes_.push_back(std::make_unique<std::mutex>());
  flush_tasks_mutexes.push_back(std::make_unique<std::mutex>());

  task_observers_.push_back(TaskObservers());
  delayed_tasks_.push_back(DelayedTaskQueue());

  return loop_id;
}

void MessageLoopTaskQueue::AddTaskObserver(MessageLoopId loop_id,
                                           intptr_t key,
                                           fml::closure callback) {
  std::lock_guard<std::mutex> lock(*observers_mutexes_[loop_id]);
  task_observers_[loop_id][key] = std::move(callback);
}

void MessageLoopTaskQueue::RemoveTaskObserver(MessageLoopId loop_id,
                                              intptr_t key) {
  std::lock_guard<std::mutex> lock(*observers_mutexes_[loop_id]);
  task_observers_[loop_id].erase(key);
}

fml::TimePoint MessageLoopTaskQueue::RegisterTask(MessageLoopId loop_id,
                                                  fml::closure task,
                                                  fml::TimePoint target_time) {
  std::lock_guard<std::mutex> lock(*delayed_tasks_mutexes_[loop_id]);
  delayed_tasks_[loop_id].push({++order_, std::move(task), target_time});
  MessageLoopId dummy;
  return PeekNextTask(loop_id, dummy).target_time;
}

#define MLTQ_DEF_LOCK(l, mutexes, loop) \
  std::unique_lock<std::mutex> l(*mutexes[loop], std::defer_lock)

bool MessageLoopTaskQueue::MergeQueues(MessageLoopId owner,
                                       MessageLoopId subsumed) {
  // task_flushing locks
  MLTQ_DEF_LOCK(t1, flush_tasks_mutexes, owner);
  MLTQ_DEF_LOCK(t2, flush_tasks_mutexes, subsumed);

  // task observers mutexes
  MLTQ_DEF_LOCK(o1, observers_mutexes_, owner);
  MLTQ_DEF_LOCK(o2, observers_mutexes_, subsumed);

  // delayed_tasks locks
  MLTQ_DEF_LOCK(d1, delayed_tasks_mutexes_, owner);
  MLTQ_DEF_LOCK(d2, delayed_tasks_mutexes_, subsumed);

  std::lock(t1, t2, o1, o2, d1, d2);

  if (owner == subsumed) {
    return true;
  }

  // these are already merged.
  if (owner_to_subsumed_[owner] == subsumed) {
    return subsumed_to_owner_[subsumed] == owner;
  }

  // these are already merged with some other things.
  if (owner_to_subsumed_.count(owner) || subsumed_to_owner_.count(subsumed)) {
    return false;
  }

  owner_to_subsumed_[owner] = subsumed;
  subsumed_to_owner_[subsumed] = owner;

  return true;
}

bool MessageLoopTaskQueue::UnmergeQueues(MessageLoopId owner) {
  // task_flushing locks
  MLTQ_DEF_LOCK(t, flush_tasks_mutexes, owner);
  MLTQ_DEF_LOCK(o, observers_mutexes_, owner);
  MLTQ_DEF_LOCK(d, delayed_tasks_mutexes_, owner);

  std::lock(t, o, d);

  // is not merged.
  if (owner_to_subsumed_.count(owner) == 0) {
    return true;
  }

  MessageLoopId subsumed = owner_to_subsumed_[owner];
  owner_to_subsumed_.erase(owner);
  subsumed_to_owner_.erase(subsumed);

  return true;
}

MessageLoopTaskQueue::TasksToRun MessageLoopTaskQueue::GetTasksToRunNow(
    MessageLoopId owner,
    FlushType flush_type) {
  std::lock_guard<std::mutex> lock(*delayed_tasks_mutexes_[owner]);
  std::vector<MessageLoopId> loop_ids;
  std::vector<fml::closure> invocations;

  // if the owner has been subsumed.
  if (subsumed_to_owner_.count(owner)) {
    return TasksToRun(fml::TimePoint::Max(), invocations, loop_ids);
  }

  if (!HasMoreTasks(owner)) {
    return TasksToRun(fml::TimePoint::Max(), invocations, loop_ids);
  }

  auto now = fml::TimePoint::Now();
  // this is the loop that has a higher priority task.
  MessageLoopId task_loop;
  while (HasMoreTasks(owner)) {
    const auto& top = PeekNextTask(owner, task_loop);
    if (top.target_time > now) {
      break;
    }
    invocations.emplace_back(std::move(top.task));
    delayed_tasks_[task_loop].pop();
    if (flush_type == FlushType::kSingle) {
      break;
    }
  }

  fml::TimePoint wake_up = !HasMoreTasks(owner)
                               ? fml::TimePoint::Max()
                               : PeekNextTask(owner, task_loop).target_time;
  return TasksToRun(wake_up, invocations, loop_ids);
}

void MessageLoopTaskQueue::InvokeAndNotifyObservers(
    MessageLoopId owner,
    const MessageLoopTaskQueue::TasksToRun& tasks_to_run) {
  std::lock_guard<std::mutex> observers_lock(*observers_mutexes_[owner]);

  for (const auto& invocation : tasks_to_run.invocations) {
    invocation();
    for (const auto& observer : task_observers_[owner]) {
      observer.second();
    }
  }
}

void MessageLoopTaskQueue::DisposeTasks(MessageLoopId owner) {
  std::lock_guard<std::mutex> lock(*delayed_tasks_mutexes_[owner]);
  delayed_tasks_[owner] = {};
  // remove all subsumed tasks too.
  if (owner_to_subsumed_.count(owner)) {
    delayed_tasks_[owner_to_subsumed_[owner]] = {};
  }
}

bool MessageLoopTaskQueue::HasMoreTasks(MessageLoopId owner) {
  if (!delayed_tasks_[owner].empty()) {
    return true;
  }
  if (owner_to_subsumed_.count(owner)) {
    MessageLoopId subsumed = owner_to_subsumed_[owner];
    return !delayed_tasks_[subsumed].empty();
  }
  return false;
}

DelayedTask MessageLoopTaskQueue::PeekNextTask(MessageLoopId owner,
                                               MessageLoopId& loop) {
  FML_CHECK(HasMoreTasks(owner));

  // no subsumed
  if (owner_to_subsumed_.count(owner) == 0) {
    loop = owner;
    return delayed_tasks_[owner].top();
  }

  MessageLoopId subsumed = owner_to_subsumed_[owner];

  bool owner_has_task = !delayed_tasks_[owner].empty();
  bool subsumed_has_task = !delayed_tasks_[subsumed].empty();

  if (owner_has_task && subsumed_has_task) {
    DelayedTask owner_top = delayed_tasks_[owner].top();
    DelayedTask subsumed_top = delayed_tasks_[subsumed].top();
    if (owner_top.target_time < subsumed_top.target_time) {
      loop = owner;
      return owner_top;
    } else {
      loop = subsumed;
      return subsumed_top;
    }
  } else if (owner_has_task) {
    loop = owner;
    return delayed_tasks_[owner].top();
  } else {
    loop = subsumed;
    return delayed_tasks_[subsumed].top();
  }
}

MessageLoopTaskQueue::TasksToRun::TasksToRun(
    fml::TimePoint p_wake_up,
    std::vector<fml::closure> p_invocations,
    std::vector<MessageLoopId> p_loop_ids) {
  wake_up_time = p_wake_up;
  invocations = std::move(p_invocations);
  loop_ids = std::move(p_loop_ids);
}

MessageLoopTaskQueue::TasksToRun::~TasksToRun() = default;

}  // namespace fml

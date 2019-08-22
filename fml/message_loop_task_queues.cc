// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#define FML_USED_ON_EMBEDDER

#include "flutter/fml/message_loop_task_queues.h"
#include "flutter/fml/message_loop_impl.h"

namespace fml {

std::mutex MessageLoopTaskQueues::creation_mutex_;

const size_t TaskQueueId::kUnmerged = ULONG_MAX;

fml::RefPtr<MessageLoopTaskQueues> MessageLoopTaskQueues::instance_;

TaskQueueEntry::TaskQueueEntry()
    : owner_of(_kUnmerged), subsumed_by(_kUnmerged) {
  wakeable = NULL;
  task_observers = TaskObservers();
  delayed_tasks = DelayedTaskQueue();

  tasks_mutex = std::make_unique<std::mutex>();
  observers_mutex = std::make_unique<std::mutex>();
  wakeable_mutex = std::make_unique<std::mutex>();
}

fml::RefPtr<MessageLoopTaskQueues> MessageLoopTaskQueues::GetInstance() {
  std::scoped_lock creation(creation_mutex_);
  if (!instance_) {
    instance_ = fml::MakeRefCounted<MessageLoopTaskQueues>();
  }
  return instance_;
}

TaskQueueId MessageLoopTaskQueues::CreateTaskQueue() {
  fml::UniqueLock lock(*queue_meta_mutex_);
  TaskQueueId loop_id = TaskQueueId(task_queue_id_counter_);
  ++task_queue_id_counter_;

  queue_entries[loop_id] = TaskQueueEntry();
  return loop_id;
}

MessageLoopTaskQueues::MessageLoopTaskQueues()
    : queue_meta_mutex_(fml::SharedMutex::Create()),
      task_queue_id_counter_(0),
      order_(0) {}

MessageLoopTaskQueues::~MessageLoopTaskQueues() = default;

void MessageLoopTaskQueues::Dispose(TaskQueueId queue_id) {
  fml::UniqueLock queue_wirter(*queue_meta_mutex_);
  TaskQueueId subsumed = queue_entries.at(queue_id).owner_of;
  queue_entries.erase(queue_id);
  if (subsumed != _kUnmerged) {
    queue_entries.erase(subsumed);
  }
}

void MessageLoopTaskQueues::RegisterTask(TaskQueueId queue_id,
                                         fml::closure task,
                                         fml::TimePoint target_time) {
  fml::SharedLock queue_reader(*queue_meta_mutex_);
  std::scoped_lock tasks_lock(*queue_entries.at(queue_id).tasks_mutex);

  size_t order = order_++;
  TaskQueueEntry& queue_entry = queue_entries[queue_id];
  queue_entry.delayed_tasks.push({order, std::move(task), target_time});
  TaskQueueId loop_to_wake = queue_id;
  if (queue_entry.subsumed_by != _kUnmerged) {
    loop_to_wake = queue_entry.subsumed_by;
  }
  WakeUp(loop_to_wake, queue_entry.delayed_tasks.top().GetTargetTime());
}

bool MessageLoopTaskQueues::HasPendingTasks(TaskQueueId queue_id) const {
  fml::SharedLock queue_reader(*queue_meta_mutex_);
  std::scoped_lock tasks_lock(*queue_entries.at(queue_id).tasks_mutex);

  return HasPendingTasksUnlocked(queue_id);
}

void MessageLoopTaskQueues::GetTasksToRunNow(
    TaskQueueId queue_id,
    FlushType type,
    std::vector<fml::closure>& invocations) {
  fml::SharedLock queue_reader(*queue_meta_mutex_);
  std::scoped_lock tasks_lock(*queue_entries.at(queue_id).tasks_mutex);

  if (!HasPendingTasksUnlocked(queue_id)) {
    return;
  }

  const auto now = fml::TimePoint::Now();

  while (HasPendingTasksUnlocked(queue_id)) {
    TaskQueueId top_queue = _kUnmerged;
    const auto& top = PeekNextTaskUnlocked(queue_id, top_queue);
    if (top.GetTargetTime() > now) {
      break;
    }
    invocations.emplace_back(std::move(top.GetTask()));
    queue_entries[top_queue].delayed_tasks.pop();
    if (type == FlushType::kSingle) {
      break;
    }
  }

  if (!HasPendingTasksUnlocked(queue_id)) {
    WakeUp(queue_id, fml::TimePoint::Max());
  } else {
    WakeUp(queue_id, GetNextWakeTimeUnlocked(queue_id));
  }
}

void MessageLoopTaskQueues::WakeUp(TaskQueueId queue_id,
                                   fml::TimePoint time) const {
  fml::SharedLock queue_reader(*queue_meta_mutex_);
  std::scoped_lock wakeable_lock(*queue_entries.at(queue_id).wakeable_mutex);

  if (queue_entries.at(queue_id).wakeable) {
    queue_entries.at(queue_id).wakeable->WakeUp(time);
  }
}

size_t MessageLoopTaskQueues::GetNumPendingTasks(TaskQueueId queue_id) const {
  fml::SharedLock queue_reader(*queue_meta_mutex_);
  std::scoped_lock tasks_lock(*queue_entries.at(queue_id).tasks_mutex);

  size_t total_tasks = 0;
  total_tasks += queue_entries.at(queue_id).delayed_tasks.size();

  TaskQueueId subsumed = queue_entries.at(queue_id).owner_of;
  if (subsumed != _kUnmerged) {
    std::scoped_lock tasks_lock(*queue_entries.at(subsumed).tasks_mutex);
    total_tasks += queue_entries.at(subsumed).delayed_tasks.size();
  }
  return total_tasks;
}

void MessageLoopTaskQueues::AddTaskObserver(TaskQueueId queue_id,
                                            intptr_t key,
                                            fml::closure callback) {
  fml::SharedLock queue_reader(*queue_meta_mutex_);
  std::scoped_lock observers_lock(*queue_entries.at(queue_id).observers_mutex);

  FML_DCHECK(callback != nullptr) << "Observer callback must be non-null.";
  queue_entries[queue_id].task_observers[key] = std::move(callback);
}

void MessageLoopTaskQueues::RemoveTaskObserver(TaskQueueId queue_id,
                                               intptr_t key) {
  fml::SharedLock queue_reader(*queue_meta_mutex_);
  std::scoped_lock observers_lock(*queue_entries.at(queue_id).observers_mutex);

  queue_entries[queue_id].task_observers.erase(key);
}

void MessageLoopTaskQueues::NotifyObservers(TaskQueueId queue_id) const {
  fml::SharedLock queue_reader(*queue_meta_mutex_);
  std::scoped_lock observers_lock(*queue_entries.at(queue_id).observers_mutex);

  for (const auto& observer : queue_entries.at(queue_id).task_observers) {
    observer.second();
  }

  TaskQueueId subsumed = queue_entries.at(queue_id).owner_of;
  if (subsumed != _kUnmerged) {
    for (const auto& observer : queue_entries.at(subsumed).task_observers) {
      observer.second();
    }
  }
}

void MessageLoopTaskQueues::SetWakeable(TaskQueueId queue_id,
                                        fml::Wakeable* wakeable) {
  fml::SharedLock queue_reader(*queue_meta_mutex_);
  std::scoped_lock wakeable_lock(*queue_entries.at(queue_id).wakeable_mutex);

  FML_CHECK(!queue_entries[queue_id].wakeable)
      << "Wakeable can only be set once.";
  queue_entries.at(queue_id).wakeable = wakeable;
}

bool MessageLoopTaskQueues::Merge(TaskQueueId owner, TaskQueueId subsumed) {
  fml::UniqueLock queue_wirter(*queue_meta_mutex_);

  if (owner == subsumed) {
    return true;
  }

  if (queue_entries[owner].owner_of == subsumed) {
    return true;
  }

  std::vector<TaskQueueId> owner_subsumed_keys = {
      queue_entries[owner].owner_of, queue_entries[owner].subsumed_by,
      queue_entries[subsumed].owner_of, queue_entries[subsumed].subsumed_by};

  for (auto key : owner_subsumed_keys) {
    if (key != _kUnmerged) {
      return false;
    }
  }

  queue_entries[owner].owner_of = subsumed;
  queue_entries[subsumed].subsumed_by = owner;

  if (HasPendingTasksUnlocked(owner)) {
    WakeUp(owner, GetNextWakeTimeUnlocked(owner));
  }

  return true;
}

bool MessageLoopTaskQueues::Unmerge(TaskQueueId owner) {
  fml::UniqueLock queue_wirter(*queue_meta_mutex_);

  const TaskQueueId subsumed = queue_entries[owner].owner_of;
  if (subsumed == _kUnmerged) {
    return false;
  }

  queue_entries[subsumed].subsumed_by = _kUnmerged;
  queue_entries[owner].owner_of = _kUnmerged;

  if (HasPendingTasksUnlocked(owner)) {
    WakeUp(owner, GetNextWakeTimeUnlocked(owner));
  }

  if (HasPendingTasksUnlocked(subsumed)) {
    WakeUp(subsumed, GetNextWakeTimeUnlocked(subsumed));
  }

  return true;
}

bool MessageLoopTaskQueues::Owns(TaskQueueId owner,
                                 TaskQueueId subsumed) const {
  fml::SharedLock queue_reader(*queue_meta_mutex_);
  return subsumed == queue_entries.at(owner).owner_of || owner == subsumed;
}

// Subsumed queues will never have pending tasks.
// Owning queues will consider both their and their subsumed tasks.
bool MessageLoopTaskQueues::HasPendingTasksUnlocked(
    TaskQueueId queue_id) const {
  const TaskQueueEntry& entry = queue_entries.at(queue_id);
  bool is_subsumed = entry.subsumed_by != _kUnmerged;
  if (is_subsumed) {
    return false;
  }

  if (!entry.delayed_tasks.empty()) {
    return true;
  }

  const TaskQueueId subsumed = entry.owner_of;
  if (subsumed == _kUnmerged) {
    // this is not an owner and queue is empty.
    return false;
  } else {
    return !queue_entries.at(subsumed).delayed_tasks.empty();
  }
}

fml::TimePoint MessageLoopTaskQueues::GetNextWakeTimeUnlocked(
    TaskQueueId queue_id) const {
  TaskQueueId tmp = _kUnmerged;
  return PeekNextTaskUnlocked(queue_id, tmp).GetTargetTime();
}

const DelayedTask& MessageLoopTaskQueues::PeekNextTaskUnlocked(
    TaskQueueId owner,
    TaskQueueId& top_queue_id) const {
  FML_DCHECK(HasPendingTasksUnlocked(owner));
  const TaskQueueEntry& entry = queue_entries.at(owner);
  const TaskQueueId subsumed = entry.owner_of;
  if (subsumed == _kUnmerged) {
    top_queue_id = owner;
    return entry.delayed_tasks.top();
  }

  const auto& owner_tasks = entry.delayed_tasks;
  const auto& subsumed_tasks = queue_entries.at(subsumed).delayed_tasks;

  // we are owning another task queue
  const bool subsumed_has_task = !subsumed_tasks.empty();
  const bool owner_has_task = !owner_tasks.empty();
  if (owner_has_task && subsumed_has_task) {
    const auto owner_task = owner_tasks.top();
    const auto subsumed_task = subsumed_tasks.top();
    if (owner_task > subsumed_task) {
      top_queue_id = subsumed;
    } else {
      top_queue_id = owner;
    }
  } else if (owner_has_task) {
    top_queue_id = owner;
  } else {
    top_queue_id = subsumed;
  }
  return queue_entries.at(top_queue_id).delayed_tasks.top();
}

}  // namespace fml

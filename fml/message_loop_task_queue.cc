// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#define FML_USED_ON_EMBEDDER

#include "flutter/fml/message_loop_task_queue.h"
#include "flutter/fml/message_loop_impl.h"

namespace fml {

std::mutex MessageLoopTaskQueue::creation_mutex_;
fml::RefPtr<MessageLoopTaskQueue> MessageLoopTaskQueue::instance_;

fml::RefPtr<MessageLoopTaskQueue> MessageLoopTaskQueue::GetInstance() {
  std::lock_guard<std::mutex> creation(creation_mutex_);
  if (!instance_) {
    instance_ = fml::MakeRefCounted<MessageLoopTaskQueue>();
  }
  return instance_;
}

TaskQueueId MessageLoopTaskQueue::CreateTaskQueue() {
  std::lock_guard<std::mutex> creation(queue_meta_mutex_);
  TaskQueueId loop_id = task_queue_id_counter_;
  ++task_queue_id_counter_;

  observers_mutexes_.push_back(std::make_unique<std::mutex>());
  delayed_tasks_mutexes_.push_back(std::make_unique<std::mutex>());
  wakeable_mutexes_.push_back(std::make_unique<std::mutex>());

  task_observers_.push_back(TaskObservers());
  delayed_tasks_.push_back(DelayedTaskQueue());
  wakeables_.push_back(NULL);

  return loop_id;
}

MessageLoopTaskQueue::MessageLoopTaskQueue()
    : task_queue_id_counter_(0), order_(0) {}

MessageLoopTaskQueue::~MessageLoopTaskQueue() = default;

void MessageLoopTaskQueue::Dispose(TaskQueueId queue_id) {
  std::lock_guard<std::mutex> lock(GetMutex(queue_id, MutexType::kTasks));
  delayed_tasks_[queue_id] = {};
}

void MessageLoopTaskQueue::RegisterTask(TaskQueueId queue_id,
                                        fml::closure task,
                                        fml::TimePoint target_time) {
  std::lock_guard<std::mutex> lock(GetMutex(queue_id, MutexType::kTasks));
  size_t order = order_++;
  delayed_tasks_[queue_id].push({order, std::move(task), target_time});
  WakeUp(queue_id, delayed_tasks_[queue_id].top().GetTargetTime());
}

bool MessageLoopTaskQueue::HasPendingTasks(TaskQueueId queue_id) {
  std::lock_guard<std::mutex> lock(GetMutex(queue_id, MutexType::kTasks));
  return !delayed_tasks_[queue_id].empty();
}

void MessageLoopTaskQueue::GetTasksToRunNow(
    TaskQueueId queue_id,
    FlushType type,
    std::vector<fml::closure>& invocations) {
  std::lock_guard<std::mutex> lock(GetMutex(queue_id, MutexType::kTasks));

  const auto now = fml::TimePoint::Now();
  DelayedTaskQueue& tasks = delayed_tasks_[queue_id];

  while (!tasks.empty()) {
    const auto& top = tasks.top();
    if (top.GetTargetTime() > now) {
      break;
    }
    invocations.emplace_back(std::move(top.GetTask()));
    tasks.pop();
    if (type == FlushType::kSingle) {
      break;
    }
  }

  if (tasks.empty()) {
    WakeUp(queue_id, fml::TimePoint::Max());
  } else {
    WakeUp(queue_id, tasks.top().GetTargetTime());
  }
}

void MessageLoopTaskQueue::WakeUp(TaskQueueId queue_id, fml::TimePoint time) {
  std::lock_guard<std::mutex> lock(GetMutex(queue_id, MutexType::kWakeables));
  if (wakeables_[queue_id]) {
    wakeables_[queue_id]->WakeUp(time);
  }
}

size_t MessageLoopTaskQueue::GetNumPendingTasks(TaskQueueId queue_id) {
  std::lock_guard<std::mutex> lock(GetMutex(queue_id, MutexType::kTasks));
  return delayed_tasks_[queue_id].size();
}

void MessageLoopTaskQueue::AddTaskObserver(TaskQueueId queue_id,
                                           intptr_t key,
                                           fml::closure callback) {
  std::lock_guard<std::mutex> lock(GetMutex(queue_id, MutexType::kObservers));
  task_observers_[queue_id][key] = std::move(callback);
}

void MessageLoopTaskQueue::RemoveTaskObserver(TaskQueueId queue_id,
                                              intptr_t key) {
  std::lock_guard<std::mutex> lock(GetMutex(queue_id, MutexType::kObservers));
  task_observers_[queue_id].erase(key);
}

void MessageLoopTaskQueue::NotifyObservers(TaskQueueId queue_id) {
  std::lock_guard<std::mutex> lock(GetMutex(queue_id, MutexType::kObservers));
  for (const auto& observer : task_observers_[queue_id]) {
    observer.second();
  }
}

// Thread safety analysis disabled as it does not account for defered locks.
void MessageLoopTaskQueue::Swap(TaskQueueId primary, TaskQueueId secondary)
    FML_NO_THREAD_SAFETY_ANALYSIS {
  // task_observers locks
  std::unique_lock<std::mutex> o1(GetMutex(primary, MutexType::kObservers),
                                  std::defer_lock);
  std::unique_lock<std::mutex> o2(GetMutex(secondary, MutexType::kObservers),
                                  std::defer_lock);

  // delayed_tasks locks
  std::unique_lock<std::mutex> t1(GetMutex(primary, MutexType::kTasks),
                                  std::defer_lock);
  std::unique_lock<std::mutex> t2(GetMutex(secondary, MutexType::kTasks),
                                  std::defer_lock);

  std::lock(o1, o2, t1, t2);

  std::swap(task_observers_[primary], task_observers_[secondary]);
  std::swap(delayed_tasks_[primary], delayed_tasks_[secondary]);
}

void MessageLoopTaskQueue::SetWakeable(TaskQueueId queue_id,
                                       fml::Wakeable* wakeable) {
  std::lock_guard<std::mutex> lock(GetMutex(queue_id, MutexType::kWakeables));
  wakeables_[queue_id] = wakeable;
}

std::mutex& MessageLoopTaskQueue::GetMutex(TaskQueueId queue_id,
                                           MutexType type) {
  std::lock_guard<std::mutex> lock(queue_meta_mutex_);
  if (type == MutexType::kTasks) {
    return *delayed_tasks_mutexes_[queue_id];
  } else if (type == MutexType::kObservers) {
    return *observers_mutexes_[queue_id];
  } else {
    return *wakeable_mutexes_[queue_id];
  }
}

}  // namespace fml

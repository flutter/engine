// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/fml/concurrent_message_loop.h"

#include <algorithm>

#include "flutter/fml/thread.h"
#include "flutter/fml/trace_event.h"

namespace fml {

std::unique_ptr<ConcurrentMessageLoop> ConcurrentMessageLoop::Create(
    size_t worker_count) {
  return std::unique_ptr<ConcurrentMessageLoop>{
      new ConcurrentMessageLoop(worker_count)};
}

ConcurrentMessageLoop::ConcurrentMessageLoop(size_t worker_count)
    : worker_count_(std::max<size_t>(worker_count, 1ul)),
      task_runner_(new ConcurrentTaskRunner(this)) {
  for (size_t i = 0; i < worker_count_; ++i) {
    workers_.emplace_back([i, this]() {
      fml::Thread::SetCurrentThreadName(fml::Thread::ThreadConfig(
          std::string{"io.worker." + std::to_string(i + 1)}));
      WorkerMain();
    });
  }

  for (const auto& worker : workers_) {
    worker_thread_ids_.emplace_back(worker.get_id());
  }
}

ConcurrentMessageLoop::~ConcurrentMessageLoop() {
  {
    std::scoped_lock lock(task_runner_->weak_loop_mutex_);
    task_runner_->weak_loop_ = nullptr;
  }
  Terminate();
  for (auto& worker : workers_) {
    worker.join();
  }
}

size_t ConcurrentMessageLoop::GetWorkerCount() const {
  return worker_count_;
}

void ConcurrentMessageLoop::PostTask(const fml::closure& task) {
  if (!task) {
    return;
  }

  // Unlock the mutex before notifying the condition variable because that mutex
  // has to be acquired on the other thread anyway. Waiting in this scope till
  // it is acquired there is a pessimization.
  {
    std::unique_lock lock(tasks_mutex_);
    tasks_.push(task);
  }

  tasks_condition_.notify_one();
}

void ConcurrentMessageLoop::WorkerMain() {
  while (true) {
    std::unique_lock lock(tasks_mutex_);
    tasks_condition_.wait(lock, [&]() {
      return !tasks_.empty() || shutdown_ || HasThreadTasksLocked();
    });

    // Shutdown cannot be read with the task mutex unlocked.
    bool shutdown_now = shutdown_;
    fml::closure task;
    std::vector<fml::closure> thread_tasks;

    if (!tasks_.empty()) {
      task = tasks_.front();
      tasks_.pop();
    }

    if (HasThreadTasksLocked()) {
      thread_tasks = GetThreadTasksLocked();
      FML_DCHECK(!HasThreadTasksLocked());
    }

    // Don't hold onto the mutex while tasks are being executed as they could
    // themselves try to post more tasks to the message loop.
    lock.unlock();

    TRACE_EVENT0("flutter", "ConcurrentWorkerWake");
    // Execute the primary task we woke up for.
    if (task) {
      task();
    }

    // Execute any thread tasks.
    for (const auto& thread_task : thread_tasks) {
      thread_task();
    }

    if (shutdown_now) {
      break;
    }
  }
}

void ConcurrentMessageLoop::Terminate() {
  std::scoped_lock lock(tasks_mutex_);
  shutdown_ = true;
  tasks_condition_.notify_all();
}

void ConcurrentMessageLoop::PostTaskToAllWorkers(const fml::closure& task) {
  if (!task) {
    return;
  }

  std::scoped_lock lock(tasks_mutex_);
  for (const auto& worker_thread_id : worker_thread_ids_) {
    thread_tasks_[worker_thread_id].emplace_back(task);
  }
  tasks_condition_.notify_all();
}

bool ConcurrentMessageLoop::HasThreadTasksLocked() const {
  return thread_tasks_.count(std::this_thread::get_id()) > 0;
}

std::vector<fml::closure> ConcurrentMessageLoop::GetThreadTasksLocked() {
  auto found = thread_tasks_.find(std::this_thread::get_id());
  FML_DCHECK(found != thread_tasks_.end());
  std::vector<fml::closure> pending_tasks;
  std::swap(pending_tasks, found->second);
  thread_tasks_.erase(found);
  return pending_tasks;
}

ConcurrentTaskRunner::ConcurrentTaskRunner(ConcurrentMessageLoop* weak_loop)
    : weak_loop_(weak_loop) {}

ConcurrentTaskRunner::~ConcurrentTaskRunner() = default;

void ConcurrentTaskRunner::PostTask(const fml::closure& task) {
  if (!task) {
    return;
  }

  {
    std::scoped_lock lock(weak_loop_mutex_);
    if (weak_loop_) {
      weak_loop_->PostTask(task);
    }
  }
}

bool ConcurrentMessageLoop::RunsTasksOnCurrentThread() {
  std::scoped_lock lock(tasks_mutex_);
  for (const auto& worker_thread_id : worker_thread_ids_) {
    if (worker_thread_id == std::this_thread::get_id()) {
      return true;
    }
  }
  return false;
}

}  // namespace fml

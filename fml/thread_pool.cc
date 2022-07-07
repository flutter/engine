// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/fml/thread_pool.h"

#include <functional>
#include <memory>
#include <mutex>
#include <utility>

namespace fml {

ThreadPool::ThreadPool(size_t count) {
  for (size_t i = 0; i < count; i++) {
    std::unique_ptr<Thread> thread = std::make_unique<Thread>();
    thread->GetTaskRunner()->PostTask([this] {
      while (true) {
        // Get a task from task_queue, and run in this thread.
        std::function<void()> task;
        {
          std::unique_lock<std::mutex> lock(this->queue_mutex_);
          this->condition_.wait(
              lock, [this] { return this->stop_ || !this->tasks_.empty(); });
          if (stop_ && tasks_.empty()) {
            return;
          }
          task = std::move(tasks_.front());
          tasks_.pop();
        }
        task();
      }
    });
    threads_.push_back(std::move(thread));
  }
}

ThreadPool::~ThreadPool() {
  {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    stop_ = true;
  }
  condition_.notify_all();
  for (auto& thread : threads_) {
    thread->Join();
  }
}

}  // namespace fml

// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FML_THREAD_POOL_H_
#define FLUTTER_FML_THREAD_POOL_H_

#include <condition_variable>
#include <cstddef>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <type_traits>
#include <vector>

#include "flutter/fml/thread.h"

namespace fml {

class ThreadPool {
 public:
  explicit ThreadPool(size_t);

  ~ThreadPool();

  template <class F, class... Args>
  auto enqueue(F&& f, Args&&... args)
      -> std::optional<std::future<typename std::invoke_result_t<F, Args...>>> {
    using return_type = typename std::invoke_result_t<F, Args...>;
    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));
    std::future<return_type> res = task->get_future();
    {
      std::unique_lock<std::mutex> lock(queue_mutex_);
      //
      if (stop_) {
        return std::nullopt;
      }
      tasks_.emplace([task] { (*task)(); });
    }
    condition_.notify_one();
    return res;
  }

 private:
  std::vector<std::unique_ptr<Thread>> threads_;

  std::queue<std::function<void()>> tasks_;

  std::mutex queue_mutex_;

  std::condition_variable condition_;

  bool stop_ = false;
};

}  // namespace fml
#endif /* FLUTTER_FML_THREAD_POOL_H_ */

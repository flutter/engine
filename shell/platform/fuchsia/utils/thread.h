// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_FUCHSIA_UTILS_THREAD_H_
#define FLUTTER_SHELL_PLATFORM_FUCHSIA_UTILS_THREAD_H_

#include <lib/async-loop/cpp/loop.h>
#include <lib/async/dispatcher.h>

#include <functional>
#include <string>
#include <thread>

namespace fx {

class Thread {
 public:
  static void SetProcessName(std::string process_name);
  static void SetCurrentThreadName(std::string thread_name);

  Thread() {}  // Represents an invalid Thread.
  explicit Thread(std::string name);
  Thread(const Thread&) = delete;
  Thread(Thread&& other) = default;
  ~Thread();

  Thread& operator=(const Thread&) = delete;
  Thread& operator=(Thread&& other) = default;

  async_dispatcher_t* dispatcher() const { return loop_->dispatcher(); }

  void TaskBarrier(std::function<void()> task) const;
  void Join();

 private:
  async::Loop* loop_ = nullptr;
  std::thread thread_;

  bool joined_ = false;
};

}  // namespace fx

#endif  // FLUTTER_SHELL_PLATFORM_FUCHSIA_UTILS_THREAD_H_

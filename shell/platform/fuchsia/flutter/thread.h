// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_FUCHSIA_FLUTTER_THREAD_H_
#define FLUTTER_SHELL_PLATFORM_FUCHSIA_FLUTTER_THREAD_H_

#include <lib/async-loop/cpp/loop.h>
#include <lib/async/dispatcher.h>

#include <memory>
#include <thread>

namespace flutter_runner {

class Thread {
 public:
  Thread();
  ~Thread();
  Thread(const Thread&) = delete;
  Thread& operator=(const Thread&) = delete;

  async_dispatcher_t* dispatcher() const;

  void Quit();

  void Join();

 private:
  async::Loop loop_;
  std::thread thread_;
  bool joined_ = false;
};

}  // namespace flutter_runner

#endif  // FLUTTER_SHELL_PLATFORM_FUCHSIA_FLUTTER_THREAD_H_

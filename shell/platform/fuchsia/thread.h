// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TOPAZ_RUNTIME_FLUTTER_RUNNER_THREAD_H_
#define TOPAZ_RUNTIME_FLUTTER_RUNNER_THREAD_H_

#include <pthread.h>

#include <functional>

#include <lib/async-loop/cpp/loop.h>

#include "flutter/fml/macros.h"

namespace flutter_runner {

class Thread {
 public:
  Thread();

  ~Thread();

  void Quit();

  bool Join();

  bool IsValid() const;

  async_dispatcher_t* dispatcher() const;

 private:
  bool valid_;
  pthread_t thread_;
  std::unique_ptr<async::Loop> loop_;

  void Main();

  FML_DISALLOW_COPY_AND_ASSIGN(Thread);
};

}  // namespace flutter_runner

 #endif  // TOPAZ_RUNTIME_FLUTTER_RUNNER_THREAD_H_

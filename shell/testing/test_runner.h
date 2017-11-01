// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHELL_TESTING_TEST_RUNNER_H_
#define SHELL_TESTING_TEST_RUNNER_H_

#include <memory>
#include <string>

#include "flutter/shell/common/shell.h"
#include "flutter/shell/common/thread_host.h"
#include "lib/fxl/macros.h"
#include "lib/fxl/memory/weak_ptr.h"

namespace shell {

class TestRunner {
 public:
  static TestRunner& Shared();

  void Run(RunConfiguration config);

 private:
  ThreadHost thread_host_;
  std::unique_ptr<Shell> shell_;

  TestRunner();

  ~TestRunner();

  FXL_DISALLOW_COPY_AND_ASSIGN(TestRunner);
};

}  // namespace shell

#endif  // SHELL_TESTING_TEST_RUNNER_H_

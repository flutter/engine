// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_FUCHSIA_UTILS_TEMPFS_H_
#define FLUTTER_SHELL_PLATFORM_FUCHSIA_UTILS_TEMPFS_H_

#include <lib/async-loop/cpp/loop.h>
#include <lib/fdio/namespace.h>

#include <memory>

namespace fx {

// Sets up /tmp for the dart_runner and flutter_runner.
class RunnerTemp {
 public:
  // Take the memfs mapped into the process-wide namespace for /tmp, and map it
  // to /tmp in the given namespace.
  static void SetupComponent(fdio_ns_t* ns);

  // Sets up a memfs bound to /tmp in the process-wide namespace that has the
  // lifetime of this instance.
  RunnerTemp();
  RunnerTemp(const RunnerTemp&) = delete;
  RunnerTemp(RunnerTemp&&) = delete;
  ~RunnerTemp();

  RunnerTemp& operator=(const RunnerTemp&) = delete;
  RunnerTemp& operator=(RunnerTemp&&) = delete;

 private:
  void Start();

  std::unique_ptr<async::Loop> loop_;
};

}  // namespace fx

#endif  // FLUTTER_SHELL_PLATFORM_FUCHSIA_UTILS_TEMPFS_H_

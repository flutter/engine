// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_CLIENT_WRAPPER_INCLUDE_FLUTTER_TASK_RUNNER_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_CLIENT_WRAPPER_INCLUDE_FLUTTER_TASK_RUNNER_H_

#include <flutter_windows.h>

#include <functional>

namespace flutter {

// An interface for scheduling tasks on the associated thread.
class FlutterTaskRunner {
 public:
  typedef std::function<void()> VoidCallback;
  explicit FlutterTaskRunner(FlutterDesktopTaskRunnerRef task_runner)
      : task_runner_(task_runner) {}

  // Schedules the task for execution by the associated thread.
  void PostTask(VoidCallback callback) {
    if (RunsTasksOnCurrentThread()) {
      callback();
      return;
    }

    struct Captures {
      VoidCallback callback;
    };
    auto captures = new Captures();
    captures->callback = callback;
    FlutterDesktopTaskRunnerPostTask(
        task_runner_,
        [](void* opaque) {
          auto captures = reinterpret_cast<Captures*>(opaque);
          captures->callback();
          delete captures;
        },
        captures);
  }

  // Returns `true` if the associated thread is the current thread.
  bool RunsTasksOnCurrentThread() {
    return FlutterDesktopTaskRunnerRunsTasksOnCurrentThread(task_runner_);
  }

 private:
  // Handle for interacting with the C API's task runner.
  FlutterDesktopTaskRunnerRef task_runner_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_CLIENT_WRAPPER_INCLUDE_FLUTTER_TASK_RUNNER_H_

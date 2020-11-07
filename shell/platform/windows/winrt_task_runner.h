// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_WINRT_TASK_RUNNER_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_WINRT_TASK_RUNNER_H_

#include <windows.h>

#include <winrt/Windows.UI.Core.h>

#include <chrono>
#include <functional>
#include <thread>

#include "flutter/shell/platform/embedder/embedder.h"
#include "flutter/shell/platform/windows/task_runner.h"

namespace flutter {

// A custom task runner that uses a CoreDispatcher to schedule
// flutter tasks.
class WinrtTaskRunner : public TaskRunner {
 public:
  using TaskExpiredCallback = std::function<void(const FlutterTask*)>;
  WinrtTaskRunner(DWORD main_thread_id,
                  const TaskExpiredCallback& on_task_expired);

  ~WinrtTaskRunner();

  // |RunsTasksOnCurrentThread|
  bool RunsTasksOnCurrentThread() const override;

  // |PostTask|
  void PostTask(FlutterTask flutter_task,
                uint64_t flutter_target_time_nanos) override;

 private:
  using TaskTimePoint = std::chrono::steady_clock::time_point;
  DWORD main_thread_id_;
  TaskExpiredCallback on_task_expired_;

  WinrtTaskRunner(const WinrtTaskRunner&) = delete;

  WinrtTaskRunner& operator=(const WinrtTaskRunner&) = delete;

  winrt::Windows::UI::Core::CoreDispatcher dispatcher_{nullptr};

  static TaskTimePoint TimePointFromFlutterTime(
      uint64_t flutter_target_time_nanos);
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_WINRT_TASK_RUNNER_H_

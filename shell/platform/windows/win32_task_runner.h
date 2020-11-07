// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_WIN32_TASK_RUNNER_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_WIN32_TASK_RUNNER_H_

#include <windows.h>

#include <chrono>
#include <deque>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

#include "flutter/shell/platform/embedder/embedder.h"
#include "flutter/shell/platform/windows/task_runner.h"

namespace flutter {

// A custom task runner that integrates with user32 GetMessage semantics so that
// host app can own its own message loop and flutter still gets to process
// tasks on a timely basis.
class Win32TaskRunner : public TaskRunner {
 public:
  using TaskExpiredCallback = std::function<void(const FlutterTask*)>;
  Win32TaskRunner(DWORD main_thread_id,
                  const TaskExpiredCallback& on_task_expired);

  virtual ~Win32TaskRunner();

  // |RunsTasksOnCurrentThread|
  bool RunsTasksOnCurrentThread() const override;

  std::chrono::nanoseconds ProcessTasks();

  // |PostTask|
  void PostTask(FlutterTask flutter_task,
                uint64_t flutter_target_time_nanos) override;

 private:
  using TaskTimePoint = std::chrono::steady_clock::time_point;
  struct Task {
    uint64_t order;
    TaskTimePoint fire_time;
    FlutterTask task;

    struct Comparer {
      bool operator()(const Task& a, const Task& b) {
        if (a.fire_time == b.fire_time) {
          return a.order > b.order;
        }
        return a.fire_time > b.fire_time;
      }
    };
  };
  DWORD main_thread_id_;
  TaskExpiredCallback on_task_expired_;
  std::mutex task_queue_mutex_;
  std::priority_queue<Task, std::deque<Task>, Task::Comparer> task_queue_;

  Win32TaskRunner(const Win32TaskRunner&) = delete;

  Win32TaskRunner& operator=(const Win32TaskRunner&) = delete;

  static TaskTimePoint TimePointFromFlutterTime(
      uint64_t flutter_target_time_nanos);
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_WIN32_TASK_RUNNER_H_

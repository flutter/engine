// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_TASK_RUNNER_WIN32_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_TASK_RUNNER_WIN32_H_

#include <windows.h>

#include "flutter/shell/platform/embedder/embedder.h"
#include "flutter/shell/platform/windows/task_runner.h"
#include "flutter/shell/platform/windows/task_runner_win32_window.h"

namespace flutter {

// A custom timer that integrates with user32 GetMessage semantics so that
// host app can own its own message loop and flutter still gets to process
// tasks on a timely basis.
class TaskRunnerTimerWin32 : public TaskRunnerTimer,
                             public TaskRunnerWin32Window::Delegate {
 public:
  TaskRunnerTimerWin32(TaskRunnerTimer::Delegate* delegate);
  virtual ~TaskRunnerTimerWin32();

  // |TaskRunnerTimer|
  void WakeUp() override;

  // |TaskRunnerTimer|
  bool RunsOnCurrentThread() const override;

  // |TaskRunnerTimer|
  std::chrono::nanoseconds ProcessTasks() override;

 private:
  TaskRunnerTimer::Delegate* delegate_;
  DWORD main_thread_id_;
  std::shared_ptr<TaskRunnerWin32Window> task_runner_window_;

  TaskRunnerTimerWin32(const TaskRunnerTimerWin32&) = delete;
  TaskRunnerTimerWin32& operator=(const TaskRunnerTimerWin32&) = delete;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_TASK_RUNNER_WIN32_H_

// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/windows/task_runner_win32.h"

namespace flutter {

// static
std::unique_ptr<TaskRunnerTimer> TaskRunnerTimer::Create(
    TaskRunnerTimer::Delegate* delegate) {
  return std::make_unique<TaskRunnerTimerWin32>(delegate);
}

TaskRunnerTimerWin32::TaskRunnerTimerWin32(TaskRunnerTimer::Delegate* delegate)
    : delegate_(delegate) {
  main_thread_id_ = GetCurrentThreadId();
  task_runner_window_ = TaskRunnerWin32Window::GetSharedInstance();
  task_runner_window_->AddDelegate(this);
}

TaskRunnerTimerWin32::~TaskRunnerTimerWin32() {
  task_runner_window_->RemoveDelegate(this);
}

void TaskRunnerTimerWin32::WakeUp() {
  task_runner_window_->WakeUp();
};

bool TaskRunnerTimerWin32::RunsOnCurrentThread() const {
  return GetCurrentThreadId() == main_thread_id_;
};

std::chrono::nanoseconds TaskRunnerTimerWin32::ProcessTasks() {
  return delegate_->ProcessTasks();
}

}  // namespace flutter

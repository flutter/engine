// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/windows/task_runner_winuwp.h"

namespace flutter {

// static
std::unique_ptr<TaskRunnerTimer> TaskRunnerTimer::Create(
    TaskRunnerTimer::Delegate* delegate) {
  return std::make_unique<TaskRunnerTimerWinUwp>(delegate);
}

TaskRunnerTimerWinUwp::TaskRunnerTimerWinUwp(
    TaskRunnerTimer::Delegate* delegate)
    : delegate_(delegate) {
  dispatcher_queue_ =
      winrt::Windows::System::DispatcherQueue::GetForCurrentThread();
  dispatcher_queue_timer_ = dispatcher_queue_.CreateTimer();
  dispatcher_queue_timer_.Tick({this, &TaskRunnerTimerWinUwp::OnTick});
}

TaskRunnerTimerWinUwp::~TaskRunnerTimerWinUwp() = default;

void TaskRunnerTimerWinUwp::WakeUp() {
  dispatcher_queue_.TryEnqueue([this]() { ProcessTasks(); });
};

bool TaskRunnerTimerWinUwp::RunsOnCurrentThread() const {
  return dispatcher_queue_.HasThreadAccess();
};

void TaskRunnerTimerWinUwp::OnTick(
    winrt::Windows::System::DispatcherQueueTimer const&,
    winrt::Windows::Foundation::IInspectable const&) {
  ProcessTasks();
}

void TaskRunnerTimerWinUwp::ProcessTasks() {
  auto next = delegate_->ProcessTasks();

  if (next == std::chrono::nanoseconds::max()) {
    dispatcher_queue_timer_.Stop();
  } else {
    dispatcher_queue_timer_.Interval(
        std::chrono::duration_cast<winrt::Windows::Foundation::TimeSpan>(next));
    dispatcher_queue_timer_.Start();
  }
}

}  // namespace flutter

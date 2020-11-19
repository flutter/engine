// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/windows/task_runner_winuwp.h"

#include <atomic>
#include <utility>

namespace flutter {

TaskRunnerWinUwp::TaskRunnerWinUwp(DWORD main_thread_id,
                                   const TaskExpiredCallback& on_task_expired)
    : main_thread_id_(main_thread_id),
      on_task_expired_(std::move(on_task_expired)) {
  dispatcher_ =
      winrt::Windows::UI::Core::CoreWindow::GetForCurrentThread().Dispatcher();
}

TaskRunnerWinUwp::~TaskRunnerWinUwp() = default;

bool TaskRunnerWinUwp::RunsTasksOnCurrentThread() const {
  return GetCurrentThreadId() == main_thread_id_;
}

TaskRunnerWinUwp::TaskTimePoint TaskRunnerWinUwp::TimePointFromFlutterTime(
    uint64_t flutter_target_time_nanos) {
  const auto now = TaskTimePoint::clock::now();
  const auto flutter_duration =
      flutter_target_time_nanos - FlutterEngineGetCurrentTime();
  return now + std::chrono::nanoseconds(flutter_duration);
}

void TaskRunnerWinUwp::PostTask(FlutterTask flutter_task,
                                uint64_t flutter_target_time_nanos) {
  // TODO: consider waiting on threadpool thread until target time

  dispatcher_.RunAsync(
      winrt::Windows::UI::Core::CoreDispatcherPriority::Normal,
      [this, flutter_task]() { on_task_expired_(&flutter_task); });
}

}  // namespace flutter

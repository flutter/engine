// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/windows/winrt_task_runner.h"

#include <atomic>
#include <utility>

namespace flutter {

WinrtTaskRunner::WinrtTaskRunner(DWORD main_thread_id,
                                 const TaskExpiredCallback& on_task_expired)
    : main_thread_id_(main_thread_id),
      on_task_expired_(std::move(on_task_expired)) {
  dispatcher_ =
      winrt::Windows::UI::Core::CoreWindow::GetForCurrentThread().Dispatcher();
}

WinrtTaskRunner::~WinrtTaskRunner() = default;

bool WinrtTaskRunner::RunsTasksOnCurrentThread() const {
  return GetCurrentThreadId() == main_thread_id_;
}

WinrtTaskRunner::TaskTimePoint WinrtTaskRunner::TimePointFromFlutterTime(
    uint64_t flutter_target_time_nanos) {
  const auto now = TaskTimePoint::clock::now();
  const auto flutter_duration =
      flutter_target_time_nanos - FlutterEngineGetCurrentTime();
  return now + std::chrono::nanoseconds(flutter_duration);
}

void WinrtTaskRunner::PostTask(FlutterTask flutter_task,
                               uint64_t flutter_target_time_nanos) {
  // TODO: consider waiting on threadpool thread until target time

  dispatcher_.RunAsync(
      winrt::Windows::UI::Core::CoreDispatcherPriority::Normal,
      [this, flutter_task]() { on_task_expired_(&flutter_task); });
}

}  // namespace flutter

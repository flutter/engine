// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/windows/task_runner_winuwp.h"

#include <atomic>
#include <utility>

namespace flutter {

// static
std::unique_ptr<TaskRunner> TaskRunner::Create(
    DWORD main_thread_id,
    CurrentTimeProc get_current_time,
    const TaskExpiredCallback& on_task_expired) {
  return std::make_unique<TaskRunnerWinUwp>(main_thread_id, get_current_time,
                                            on_task_expired);
}

TaskRunnerWinUwp::TaskRunnerWinUwp(DWORD main_thread_id,
                                   CurrentTimeProc get_current_time,
                                   const TaskExpiredCallback& on_task_expired)
    : main_thread_id_(main_thread_id),
      get_current_time_(get_current_time),
      on_task_expired_(std::move(on_task_expired)) {
  dispatcher_ =
      winrt::Windows::UI::Core::CoreWindow::GetForCurrentThread().Dispatcher();
}

TaskRunnerWinUwp::~TaskRunnerWinUwp() {
  std::lock_guard<std::mutex> lock(timer_queued_mutex_);
  for (const auto& timer : timer_queued_) {
    timer.Cancel();
  }
};

bool TaskRunnerWinUwp::RunsTasksOnCurrentThread() const {
  return GetCurrentThreadId() == main_thread_id_;
}

void TaskRunnerWinUwp::PostFlutterTask(FlutterTask flutter_task,
                                       uint64_t flutter_target_time_nanos) {
  const auto delay =
      std::chrono::duration_cast<winrt::Windows::Foundation::TimeSpan>(
          std::chrono::nanoseconds(flutter_target_time_nanos -
                                   get_current_time_()));

  auto timer = winrt::Windows::System::Threading::ThreadPoolTimer::CreateTimer(
      winrt::Windows::System::Threading::TimerElapsedHandler(
          [this, flutter_task](auto& timer) {
            {
              std::lock_guard<std::mutex> lock(timer_queued_mutex_);
              auto i =
                  std::find(timer_queued_.begin(), timer_queued_.end(), timer);
              if (i != timer_queued_.end()) {
                timer_queued_.erase(i);
              }
            }

            PostTask(
                [this, flutter_task]() { on_task_expired_(&flutter_task); });
          }),
      delay);

  {
    std::lock_guard<std::mutex> lock(timer_queued_mutex_);
    timer_queued_.push_back(timer);
  }
}

void TaskRunnerWinUwp::PostTask(TaskClosure task) {
  dispatcher_.RunAsync(winrt::Windows::UI::Core::CoreDispatcherPriority::Normal,
                       [task]() { task(); });
}

}  // namespace flutter

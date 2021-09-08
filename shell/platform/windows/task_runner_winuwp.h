// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_WINRT_TASK_RUNNER_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_WINRT_TASK_RUNNER_H_

#include <third_party/cppwinrt/generated/winrt/Windows.Foundation.h>
#include <third_party/cppwinrt/generated/winrt/Windows.System.h>

#include "flutter/shell/platform/embedder/embedder.h"
#include "flutter/shell/platform/windows/task_runner.h"

namespace flutter {

// A custom timer that uses a DispatcherQueue.
class TaskRunnerTimerWinUwp : public TaskRunnerTimer {
 public:
  TaskRunnerTimerWinUwp(TaskRunnerTimer::Delegate* delegate);
  virtual ~TaskRunnerTimerWinUwp();

  // |TaskRunnerTimer|
  void WakeUp() override;

  // |TaskRunnerTimer|
  bool RunsOnCurrentThread() const override;

 private:
  void OnTick(winrt::Windows::System::DispatcherQueueTimer const&,
              winrt::Windows::Foundation::IInspectable const&);

  void ProcessTasks();

  TaskRunnerTimer::Delegate* delegate_;

  winrt::Windows::System::DispatcherQueue dispatcher_queue_{nullptr};
  winrt::Windows::System::DispatcherQueueTimer dispatcher_queue_timer_{nullptr};

  TaskRunnerTimerWinUwp(const TaskRunnerTimerWinUwp&) = delete;
  TaskRunnerTimerWinUwp& operator=(const TaskRunnerTimerWinUwp&) = delete;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_WINRT_TASK_RUNNER_H_

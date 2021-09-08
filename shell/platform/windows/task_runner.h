// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_TASK_RUNNER_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_TASK_RUNNER_H_

#include <chrono>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <variant>

#include "flutter/shell/platform/embedder/embedder.h"

namespace flutter {

typedef uint64_t (*CurrentTimeProc)();

// Abstract custom timer.
class TaskRunnerTimer {
 public:
  class Delegate {
   public:
    virtual std::chrono::nanoseconds ProcessTasks() = 0;
  };

  virtual void WakeUp() = 0;

  virtual bool RunsOnCurrentThread() const = 0;

  virtual ~TaskRunnerTimer() = default;

  // Creates a new timer with the current thread.
  static std::unique_ptr<TaskRunnerTimer> Create(Delegate* delegate);
};

class TaskRunner : public TaskRunnerTimer::Delegate {
 public:
  using TaskTimePoint = std::chrono::steady_clock::time_point;
  using TaskExpiredCallback = std::function<void(const FlutterTask*)>;
  using TaskClosure = std::function<void()>;

  // Creates a new task runner with the current thread, current time
  // provider, and callback for tasks that are ready to be run.
  TaskRunner(CurrentTimeProc get_current_time,
             const TaskExpiredCallback& on_task_expired);

  // Returns if the current thread is the UI thread.
  bool RunsTasksOnCurrentThread() const;

  // Post a Flutter engine task to the event loop for delayed execution.
  void PostFlutterTask(FlutterTask flutter_task,
                       uint64_t flutter_target_time_nanos);

  // Post a task to the event loop.
  void PostTask(TaskClosure task);

  // |TaskRunnerTimer::Delegate|
  std::chrono::nanoseconds ProcessTasks() override;

  // Post a task to the event loop or run it immediately if this is being called
  // from the main thread.
  void RunNowOrPostTask(TaskClosure task) {
    if (RunsTasksOnCurrentThread()) {
      task();
    } else {
      PostTask(std::move(task));
    }
  }

 private:
  typedef std::variant<FlutterTask, TaskClosure> TaskVariant;

  struct Task {
    uint64_t order;
    TaskTimePoint fire_time;
    TaskVariant variant;

    struct Comparer {
      bool operator()(const Task& a, const Task& b) {
        if (a.fire_time == b.fire_time) {
          return a.order > b.order;
        }
        return a.fire_time > b.fire_time;
      }
    };
  };

  // Enqueues the given task.
  void EnqueueTask(Task task);

  // Returns a TaskTimePoint computed from the given target time from Flutter.
  TaskTimePoint TimePointFromFlutterTime(
      uint64_t flutter_target_time_nanos) const;

  CurrentTimeProc get_current_time_;
  TaskExpiredCallback on_task_expired_;
  std::mutex task_queue_mutex_;
  std::priority_queue<Task, std::deque<Task>, Task::Comparer> task_queue_;
  std::unique_ptr<TaskRunnerTimer> timer_;

  TaskRunner(const TaskRunner&) = delete;
  TaskRunner& operator=(const TaskRunner&) = delete;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_TASK_RUNNER_H_

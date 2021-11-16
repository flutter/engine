// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_COMMON_VSYNC_WAITER_H_
#define FLUTTER_SHELL_COMMON_VSYNC_WAITER_H_

#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>

#include "flutter/common/task_runners.h"
#include "flutter/flow/frame_timings.h"
#include "flutter/fml/time/time_point.h"

namespace flutter {

// Represents a dynamic frame rate range.
struct FrameRateRange {
 public:
  // Constructor with default values:
  // min = kFrameRateMin,
  // preferred = kFrameRateHigh,
  // max = kFrameRateHigh
  FrameRateRange();
  // Constructor with custom values.
  FrameRateRange(const int64_t min, const int64_t preferred, const int64_t max);

  // Get the min frame rate.
  int64_t GetMin() const;

  // Get the preferred frame rate.
  int64_t GetPreferred() const;

  // Get the max frame rate.
  int64_t GetMax() const;

  friend bool operator==(const FrameRateRange& lhs, const FrameRateRange& rhs) {
    return lhs.min_ == rhs.min_ && lhs.preferred_ == rhs.preferred_ &&
           lhs.max_ == rhs.max_;
  }

  friend bool operator!=(const FrameRateRange& lhs, const FrameRateRange& rhs) {
    return lhs.min_ != rhs.min_ || lhs.preferred_ != rhs.preferred_ ||
           lhs.max_ != rhs.max_;
  }

 private:
  int64_t min_;
  int64_t preferred_;
  int64_t max_;
};

static const size_t kDefaultFRRProviderSize = 10;

// This object can provide the preferred |FrameRateRange| to use
// based on the performance.
class DynamicFrameRateRangeProvider {
 public:
  // Constuctor.
  // The `size` is used to determine how many frame durations are stored.
  DynamicFrameRateRangeProvider(size_t size = kDefaultFRRProviderSize);

  // Record a frame_duration for the current frame.
  // The `frame_duration` must be the current frame's duration.
  void Record(int64_t frame_duration);

  // Provides a |FrameRateRange| based on the recorded frame_durations.
  FrameRateRange Provide();

  // Lose all memory about the recent frame performances, resets to inital
  // state.
  void Reset();

 private:
  std::vector<int64_t> frame_durations_;
  size_t size_;
};

/// Abstract Base Class that represents a platform specific mechanism for
/// getting callbacks when a vsync event happens.
class VsyncWaiter : public std::enable_shared_from_this<VsyncWaiter> {
 public:
  using Callback = std::function<void(std::unique_ptr<FrameTimingsRecorder>)>;

  virtual ~VsyncWaiter();

  void AsyncWaitForVsync(const Callback& callback);

  /// Add a secondary callback for key |id| for the next vsync.
  ///
  /// See also |PointerDataDispatcher::ScheduleSecondaryVsyncCallback| and
  /// |Animator::ScheduleMaybeClearTraceFlowIds|.
  void ScheduleSecondaryCallback(uintptr_t id, const fml::closure& callback);

  // Tell vsync waiter to update the frame rate range.
  virtual void UpdateFrameRateRange(const FrameRateRange& frame_rate_range){};

 protected:
  // On some backends, the |FireCallback| needs to be made from a static C
  // method.
  friend class VsyncWaiterAndroid;
  friend class VsyncWaiterEmbedder;

  const TaskRunners task_runners_;

  explicit VsyncWaiter(TaskRunners task_runners);

  // There are two distinct situations where VsyncWaiter wishes to awaken at
  // the next vsync. Although the functionality can be the same, the intent is
  // different, therefore it makes sense to have a method for each intent.

  // The intent of AwaitVSync() is that the Animator wishes to produce a frame.
  // The underlying implementation can choose to be aware of this intent when
  // it comes to implementing backpressure and other scheduling invariants.
  //
  // Implementations are meant to override this method and arm their vsync
  // latches when in response to this invocation. On vsync, they are meant to
  // invoke the |FireCallback| method once (and only once) with the appropriate
  // arguments. This method should not block the current thread.
  virtual void AwaitVSync() = 0;

  // The intent of AwaitVSyncForSecondaryCallback() is simply to wake up at the
  // next vsync.
  //
  // Because there is no association with frame scheduling, underlying
  // implementations do not need to worry about maintaining invariants or
  // backpressure. The default implementation is to simply follow the same logic
  // as AwaitVSync().
  virtual void AwaitVSyncForSecondaryCallback() { AwaitVSync(); }

  // Schedules the callback on the UI task runner. Needs to be invoked as close
  // to the `frame_start_time` as possible.
  void FireCallback(fml::TimePoint frame_start_time,
                    fml::TimePoint frame_target_time,
                    bool pause_secondary_tasks = true);

 private:
  std::mutex callback_mutex_;
  Callback callback_;
  std::unordered_map<uintptr_t, fml::closure> secondary_callbacks_;

  void PauseDartMicroTasks();
  static void ResumeDartMicroTasks(fml::TaskQueueId ui_task_queue_id);

  FML_DISALLOW_COPY_AND_ASSIGN(VsyncWaiter);
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_COMMON_VSYNC_WAITER_H_

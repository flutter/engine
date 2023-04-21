// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_VIEWPORT_METRICS_UPDATER_H
#define FLUTTER_VIEWPORT_METRICS_UPDATER_H

#include <memory>
#include "flutter/fml/closure.h"
#include "flutter/fml/macros.h"
#include "flutter/fml/memory/weak_ptr.h"
#include "flutter/lib/ui/window/viewport_metrics.h"
#include "flutter/shell/common/vsync_waiter.h"

namespace flutter {

class ViewportMetricsUpdater;

class ViewportMetricsUpdater {
 public:
  class Delegate {
   public:
    /// Actually update the metrics data using Engine's `runtime_controller_`.
    virtual void DoUpdateViewportMetrics(const ViewportMetrics& metrics) = 0;

    /// Get current process stage of current vsync waiter. And updater will use
    /// different strategies to update the viewport metrics data to receiver.
    virtual VsyncWaiterProcessStage GetVsyncWaiterProcessStage() = 0;

    /// Get current vsync waiter's frame target time.
    virtual fml::TimePoint GetVsyncWaiterFrameTargetTime() = 0;

    /// Post a task to UI TaskRunner.
    virtual void PostTaskOnUITaskRunner(const fml::closure& callback) = 0;

    /// Schedule secondary vsync callback to execute after the main vsync
    /// process callback.
    virtual void ScheduleSecondaryVsyncCallback(
        uintptr_t id,
        const fml::closure& callback) = 0;
  };

  explicit ViewportMetricsUpdater(Delegate& delegate);

  ~ViewportMetricsUpdater();

  void UpdateViewportMetrics(const ViewportMetrics& metrics);

 private:
  Delegate& delegate_;

  bool has_set_valid_viewport_metrics_ = false;

  // WeakPtrFactory must be the last member.
  fml::WeakPtrFactory<ViewportMetricsUpdater> weak_factory_;
  FML_DISALLOW_COPY_AND_ASSIGN(ViewportMetricsUpdater);
};

}  // namespace flutter

#endif  // FLUTTER_VIEWPORT_METRICS_UPDATER_H

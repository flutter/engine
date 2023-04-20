// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "viewport_metrics_updater.h"
#include "flutter/fml/trace_event.h"
#include "flutter/shell/common/vsync_waiter.h"

namespace flutter {

ViewportMetricsUpdater::ViewportMetricsUpdater(Delegate& delegate)
    : delegate_(delegate), weak_factory_(this) {}
ViewportMetricsUpdater::~ViewportMetricsUpdater() = default;

void ViewportMetricsUpdater::UpdateViewportMetrics(
    const ViewportMetrics& metrics) {
  TRACE_EVENT0("flutter", "ViewportMetricsUpdater::UpdateViewportMetrics");
  if (!has_set_valid_viewport_metrics_) {
    if (metrics.physical_width > 0 && metrics.physical_height > 0) {
      has_set_valid_viewport_metrics_ = true;
      delegate_.DoUpdateViewportMetrics(metrics);
    }
    return;
  }

  VsyncWaiterProcessStage stage = delegate_.GetVsyncWaiterProcessStage();
  switch (stage) {
    case VsyncWaiterProcessStage::kAwaiting: {
      fml::TimePoint frame_target_time =
          delegate_.GetVsyncWaiterFrameTargetTime();
      if (frame_target_time > fml::TimePoint::Now()) {
        delegate_.DoUpdateViewportMetrics(metrics);
      } else {
        TRACE_EVENT0("flutter",
                     "ViewportMetricsUpdaterScheduleSecondaryVsyncCallback");
        delegate_.ScheduleSecondaryVsyncCallback(
            reinterpret_cast<uintptr_t>(this),
            [updater = weak_factory_.GetWeakPtr(), metrics] {
              if (updater) {
                updater->delegate_.DoUpdateViewportMetrics(metrics);
              }
            });
      }
      break;
    }
    case VsyncWaiterProcessStage::kProcessing: {
      TRACE_EVENT0("flutter",
                   "ViewportMetricsUpdaterRegisterTaskOnUITaskRunner");
      delegate_.PostTaskOnUITaskRunner(
          [updater = weak_factory_.GetWeakPtr(), metrics]() {
            if (updater) {
              TRACE_EVENT0("flutter", "UpdateViewportMetricsUsingPostUITask");
              updater->delegate_.DoUpdateViewportMetrics(metrics);
            }
          });
      break;
    }
    case VsyncWaiterProcessStage::kProcessingComplete: {
      delegate_.DoUpdateViewportMetrics(metrics);
      break;
    }
  }
}

}  // namespace flutter

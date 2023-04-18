// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "viewport_metrics_updater.h"
#include "flutter/fml/trace_event.h"
#include "shell/common/vsync_waiter.h"

namespace flutter {

ViewportMetricsUpdater::ViewportMetricsUpdater(Delegate& delegate)
    : delegate_(delegate), weak_factory_(this) {}
ViewportMetricsUpdater::~ViewportMetricsUpdater() = default;

void ViewportMetricsUpdater::UpdateViewportMetrics(
    const ViewportMetrics& metrics) {
  TRACE_EVENT0("flutter", "ViewportMetricsUpdater::UpdateViewportMetrics");
  const VsyncWaiterProcessStage& stage = delegate_.GetVsyncWaiterProcessStage();
  switch (stage) {
    case VsyncWaiterProcessStage::kAwaiting: {
      TRACE_EVENT0("flutter",
                   "ViewportMetricsUpdaterScheduleSecondaryVsyncCallback");
      delegate_.ScheduleSecondaryVsyncCallback(
          reinterpret_cast<uintptr_t>(this),
          [updater = weak_factory_.GetWeakPtr(), metrics] {
            updater->delegate_.DoUpdateViewportMetrics(metrics);
          });
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

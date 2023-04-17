// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "viewport_metrics_updater.h"
#include "flutter/fml/trace_event.h"

namespace flutter {

ViewportMetricsUpdater::ViewportMetricsUpdater(Delegate& delegate)
    : delegate_(delegate), weak_factory_(this) {}
ViewportMetricsUpdater::~ViewportMetricsUpdater() = default;

void ViewportMetricsUpdater::UpdateViewportMetrics(
    const ViewportMetrics& metrics) {
  if (delegate_.IsVsyncWaiterMajorCallbackComplete()) {
    delegate_.DoUpdateViewportMetrics(metrics);
  } else {
    TRACE_EVENT0("flutter", "ViewportMetricsUpdaterRegisterTaskOnUITaskRunner");
    delegate_.PostTaskOnUITaskRunner(
        [updater = weak_factory_.GetWeakPtr(), metrics]() {
          if (updater) {
            TRACE_EVENT0("flutter", "UpdateViewportMetricsUsingPostUITask");
            updater->delegate_.DoUpdateViewportMetrics(metrics);
          }
        });
  }
}

}  // namespace flutter

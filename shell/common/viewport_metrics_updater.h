// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_VIEWPORT_METRICS_UPDATER_H
#define FLUTTER_VIEWPORT_METRICS_UPDATER_H

#include <memory>
#include "flutter/lib/ui/window/viewport_metrics.h"
#include "fml/closure.h"
#include "fml/macros.h"
#include "fml/memory/weak_ptr.h"

namespace flutter {

class ViewportMetricsUpdater;

class ViewportMetricsUpdater {
 public:
  class Delegate {
   public:
    /// Actually update the metrics data using Engine's `runtime_controller_`.
    virtual void DoUpdateViewportMetrics(const ViewportMetrics& metrics) = 0;

    /// Return the value that indicates whether major process callback added by
    /// vsync is complete to process.
    virtual bool IsVsyncWaiterMajorCallbackComplete() = 0;

    /// Post a task to UI TaskRunner.
    virtual void PostTaskOnUITaskRunner(const fml::closure& callback) = 0;
  };

  explicit ViewportMetricsUpdater(Delegate& delegate);

  ~ViewportMetricsUpdater();

  void UpdateViewportMetrics(const ViewportMetrics& metrics);

 private:
  Delegate& delegate_;

  // WeakPtrFactory must be the last member.
  fml::WeakPtrFactory<ViewportMetricsUpdater> weak_factory_;
  FML_DISALLOW_COPY_AND_ASSIGN(ViewportMetricsUpdater);
};

}  // namespace flutter

#endif  // FLUTTER_VIEWPORT_METRICS_UPDATER_H

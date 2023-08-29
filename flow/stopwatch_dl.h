// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_STOPWATCH_DL_H_
#define FLUTTER_FLOW_STOPWATCH_DL_H_

#include "flow/stopwatch.h"

namespace flutter {

//------------------------------------------------------------------------------
/// A stopwatch visualizer that uses DisplayList (|DlCanvas|) to draw.
///
/// @note This is the newer non-backend specific version, that works in both
///       Skia and Impeller. The older Skia-specific version is
///       |SkStopwatchVisualizer|, which still should be used for Skia-specific
///       optimizations.
class DlStopwatchVisualizer : public StopwatchVisualizer {
 public:
  explicit DlStopwatchVisualizer(const Stopwatch& stopwatch)
      : StopwatchVisualizer(stopwatch) {}

  void Visualize(DlCanvas* canvas, const SkRect& rect) const override;
};

}  // namespace flutter

#endif  // FLUTTER_FLOW_STOPWATCH_DL_H_

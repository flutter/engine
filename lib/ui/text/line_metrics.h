// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_TEXT_LINE_METRICS_H_
#define FLUTTER_LIB_UI_TEXT_LINE_METRICS_H_

#include "third_party/dart/runtime/include/dart_api.h"
#include "third_party/tonic/converter/dart_converter.h"

namespace flutter {

struct LineMetrics {
  bool* hard_break;

  // The final computed ascent and descent for the line. This can be impacted by
  // the strut, height, scaling, as well as outlying runs that are very tall.
  //
  // The top edge is `baseline - ascent` and the bottom edge is `baseline +
  // descent`. Ascent and descent are provided as positive numbers. Raw numbers
  // for specific runs of text can be obtained in run_metrics_map. These values
  // are the cumulative metrics for the entire line.
  double* ascent;
  double* descent;
  double* unscaled_ascent;
  // Height of the line.
  double* height;
  // Width of the line.
  double* width;
  // The left edge of the line. The right edge can be obtained with `left +
  // width`
  double* left;
  // The y position of the baseline for this line from the top of the paragraph.
  double* baseline;
  // Zero indexed line number.
  size_t* line_number;

  // std::map<size_t, txt::RunMetrics> run_metrics;

  LineMetrics();

  LineMetrics(bool* hard_break,
              double* ascent,
              double* descent,
              double* unscaled_ascent,
              double* height,
              double* width,
              double* left,
              double* baseline,
              size_t* line_number)
      : hard_break(hard_break),
        ascent(ascent),
        descent(descent),
        unscaled_ascent(unscaled_ascent),
        height(height),
        width(width),
        left(left),
        baseline(baseline),
        line_number(line_number) {}
};

}  // namespace flutter

namespace tonic {
template <>
struct DartConverter<flutter::LineMetrics> {
  static Dart_Handle ToDart(const flutter::LineMetrics& val);
};

template <>
struct DartListFactory<flutter::LineMetrics> {
  static Dart_Handle NewList(intptr_t length);
};

}  // namespace tonic

#endif  // FLUTTER_LIB_UI_TEXT_LINE_METRICS_H_
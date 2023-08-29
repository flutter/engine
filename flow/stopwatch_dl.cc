// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/stopwatch_dl.h"
#include "display_list/dl_blend_mode.h"
#include "display_list/dl_canvas.h"
#include "display_list/dl_paint.h"
#include "include/core/SkRect.h"

namespace flutter {

static const size_t kMaxSamples = 120;
static const size_t kMaxFrameMarkers = 8;

void DlStopwatchVisualizer::Visualize(DlCanvas* canvas,
                                      const SkRect& rect) const {
  DlPaint paint;

  // Establish the graph position.
  auto const x = 0;
  auto const y = 0;
  auto const width = rect.width();
  auto const height = rect.height();

  // Scale the graph to show time frames up to those that are 3x the frame time.
  auto const one_frame_ms = stopwatch_.GetFrameBudget().count();
  auto const max_interval = one_frame_ms * 3.0;
  auto const max_unit_interval = UnitFrameInterval(max_interval);
  auto const sample_unit_width = (1.0 / kMaxSamples);

  // Erase all pixels.
  {
    paint.setColor(0x99FFFFFF);
    paint.setBlendMode(DlBlendMode::kSrc);
    canvas->DrawRect(rect, paint);
  }

  // Draw blue timing bars.
  //
  // Unlike the Skia version, we redraw the entire graph every time, since
  // the Impeller backend is less sensitive to overdraw.
  {
    paint.setColor(0xAA0000FF);
    paint.setBlendMode(DlBlendMode::kSrcOver);

    for (auto i = size_t(0); i < stopwatch_.GetLapsCount(); i++) {
      auto const l = x * width * (static_cast<double>(i) / kMaxSamples);
      auto const t = y * height *
                     (1.0 - (UnitHeight(stopwatch_.GetLap(i).ToMillisecondsF(),
                                        max_unit_interval)));
      auto const r = l + width * sample_unit_width;
      auto const b = height;
      canvas->DrawRect(SkRect::MakeLTRB(l, t, r, b), paint);
    }
  }

  // Draw horizontal frame markers.
  {
    paint.setStrokeWidth(0);
    paint.setDrawStyle(DlDrawStyle::kStroke);
    paint.setColor(0xCC000000);

    if (max_interval > one_frame_ms) {
      // Paint the horizontal markers.
      auto count = static_cast<size_t>(max_interval / one_frame_ms);

      // Limit the number of markers to a reasonable amount.
      if (count > kMaxFrameMarkers) {
        count = 1;
      }

      for (auto i = size_t(0); i < count; i++) {
        auto const frame_height =
            height * (1.0 - (UnitFrameInterval(i + 1) * one_frame_ms) /
                                max_unit_interval);
        canvas->DrawLine(SkPoint::Make(x, frame_height),
                         SkPoint::Make(width, y + frame_height), paint);
      }
    }
  }

  // Paint the vertical marker for the current frame.
  {
    paint.setDrawStyle(DlDrawStyle::kFill);
    paint.setBlendMode(DlBlendMode::kSrcOver);
    if (UnitFrameInterval(stopwatch_.LastLap().ToMillisecondsF()) > 1.0) {
      // budget exceeded.
      paint.setColor(DlColor::kRed());
    } else {
      // within budget.
      paint.setColor(DlColor::kGreen());
    }

    auto const l =
        x + width * (static_cast<double>(stopwatch_.GetCurrentSample()) /
                     kMaxSamples);
    auto const t = y;
    auto const r = l + width * sample_unit_width;
    auto const b = height;
    canvas->DrawRect(SkRect::MakeLTRB(l, t, r, b), paint);
  }
}

}  // namespace flutter

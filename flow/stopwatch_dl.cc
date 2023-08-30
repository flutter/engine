// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/stopwatch_dl.h"
#include <memory>
#include "display_list/dl_blend_mode.h"
#include "display_list/dl_canvas.h"
#include "display_list/dl_paint.h"
#include "display_list/dl_vertices.h"
#include "include/core/SkRect.h"

namespace flutter {

/// Returns 6 vertices representing a rectangle.
///
/// Rather than using a path, which we'll end up tessellating per frame, we
/// create a vertices object and add the rectangles (2x triangles) to it.
///
/// The goal is minimally invasive rendering for the performance monitor.
std::shared_ptr<DlVertices> FromRectLTRB(const SkScalar left,
                                         const SkScalar top,
                                         const SkScalar right,
                                         const SkScalar bottom) {
  // FIXME: Convert this into a helper class with AddRect and AddLine.
  // FIXME: Move the helper class into stopwatch_dl_vertices_helper and test it.
  auto const top_left = SkPoint::Make(left, top);
  auto const top_right = SkPoint::Make(right, top);
  auto const bottom_right = SkPoint::Make(right, bottom);
  auto const bottom_left = SkPoint::Make(left, bottom);
  const SkPoint vertices[6] = {
      top_left,      // tl tr
      top_right,     //    br
      bottom_right,  //
      bottom_right,  // tl
      bottom_left,   // bl br
      top_left       //
  };
  return DlVertices::Make(DlVertexMode::kTriangles, 6, vertices, nullptr,
                          nullptr);
}

static const size_t kMaxSamples = 120;
static const size_t kMaxFrameMarkers = 8;

// FIXME: Clean up this method in general, including all the functions and
// as-needed split into multiple methods and give better names and comments
// to everything.
void DlStopwatchVisualizer::Visualize(DlCanvas* canvas,
                                      const SkRect& rect) const {
  DlPaint paint;

  // Establish the graph position.
  auto const x = rect.x();
  auto const y = rect.y();
  auto const width = rect.width();
  auto const height = rect.height();
  auto const bottom = rect.bottom();

  // Scale the graph to show time frames up to those that are 3x the frame time.
  auto const one_frame_ms = stopwatch_.GetFrameBudget().count();
  auto const max_interval = one_frame_ms * 3.0;
  auto const max_unit_interval = UnitFrameInterval(max_interval);
  auto const sample_unit_width = (1.0 / kMaxSamples);

  // Determine how many lines to draw.
  auto horizontal_markers = static_cast<size_t>(max_interval / one_frame_ms);

  // Limit the number of markers to a reasonable amount.
  if (horizontal_markers > kMaxFrameMarkers) {
    horizontal_markers = 1;
  }

  // Erase all pixels.
  {
    paint.setColor(0x99FFFFFF);
    canvas->DrawRect(rect, paint);
  }

  // Prepare a path for the data; we start at the height of the last point so
  // it looks like we wrap around.
  {
    for (auto i = size_t(0); i < stopwatch_.GetLapsCount(); i++) {
      auto const sample_unit_height =
          (1.0 - UnitHeight(stopwatch_.GetLap(i).ToMillisecondsF(),
                            max_unit_interval));

      auto const bar_width = width * sample_unit_width;
      auto const bar_height = height * sample_unit_height;
      auto const bar_left = x + width * sample_unit_width * i;

      paint.setColor(0xAA0000FF);
      // FIXME: We should be collecting all of our vertices into a single array
      // and then drawing them all at once, rather than drawing each one as we
      // go. Ideally use DlColor as well and do literally 1 DrawVertices call
      // at the end of this function.
      canvas->DrawVertices(FromRectLTRB(/*left=*/bar_left,
                                        /*top=*/y + bar_height,
                                        /*right=*/bar_left + bar_width,
                                        /*bottom=*/y + height),
                           DlBlendMode::kSrc, paint);
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

        // Draw a skinny rectangle (i.e. line).
        canvas->DrawVertices(FromRectLTRB(/*left=*/x,
                                          /*top=*/y + frame_height,
                                          /*right=*/width,
                                          /*bottom=*/y + frame_height + 1),
                             DlBlendMode::kSrc, paint);
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
    auto const b = rect.bottom();
    canvas->DrawVertices(FromRectLTRB(l, t, r, b), DlBlendMode::kSrc, paint);
  }
}

}  // namespace flutter

// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/performance_overlay_layer.h"

#include <iomanip>
#include <string>

#include "third_party/skia/include/core/SkFont.h"
#include "third_party/skia/include/core/SkTextBlob.h"

namespace flutter {
namespace {

void VisualizeStopWatch(SkCanvas* canvas,
                        const Stopwatch& stopwatch,
                        SkScalar x,
                        SkScalar y,
                        SkScalar width,
                        SkScalar height,
                        bool show_graph,
                        bool show_labels,
                        const std::string& label_prefix,
                        const std::string& font_path) {
  const int margin_top = 3;
  const int margin_left = 1;

  if (show_graph) {
    SkRect visualization_rect = SkRect::MakeXYWH(x, y, width, height);
    stopwatch.Visualize(canvas, visualization_rect);
  }

  if (show_labels) {
    SkFont font;
    if (!font_path.empty()) {
      font = SkFont(SkTypeface::MakeFromFile(font_path.c_str()));
    }
    font.setSize(PerformanceOverlayLayer::kFontSize);

    auto label = PerformanceOverlayLayer::MakeLabelText(label_prefix, font);
    const SkRect& label_bounds = label.bounds;
    auto statistics =
        PerformanceOverlayLayer::MakeStatisticsText(stopwatch, font);
    const SkRect& statistics_bounds = statistics.bounds;

    SkPaint paint;
    paint.setColor(SK_ColorRED);

    // Draw the label on the first line.
    canvas->drawTextBlob(label.text_blob, x + margin_left - label_bounds.left(),
                         y + margin_top - label_bounds.top(), paint);
    // Draw the statistics text on the second line.
    canvas->drawTextBlob(
        statistics.text_blob, x + margin_left - statistics_bounds.left(),
        y + margin_top * 2 - statistics_bounds.top() + label_bounds.height(),
        paint);
  }
}

}  // namespace

PerformanceOverlayLayer::TextBlobAndBounds
PerformanceOverlayLayer::MakeLabelText(const std::string& label_prefix,
                                       const SkFont& font) {
  std::stringstream stream;
  stream.setf(std::ios::fixed | std::ios::showpoint);
  stream << std::setprecision(1);
  stream << label_prefix << " thread";
  auto text = stream.str();
  SkRect measured_bounds{};
  // The bounds measured by font is preciser than the SkTextBlob's bounds.
  font.measureText(text.c_str(), text.size(), SkTextEncoding::kUTF8,
                   &measured_bounds);
  return TextBlobAndBounds{
      .text_blob = SkTextBlob::MakeFromText(text.c_str(), text.size(), font,
                                            SkTextEncoding::kUTF8),
      .bounds = measured_bounds};
}

PerformanceOverlayLayer::TextBlobAndBounds
PerformanceOverlayLayer::MakeStatisticsText(const Stopwatch& stopwatch,
                                            const SkFont& font) {
  double max_ms_per_frame = stopwatch.MaxDelta().ToMillisecondsF();
  double average_ms_per_frame = stopwatch.AverageDelta().ToMillisecondsF();
  const Stopwatch::FpsInfo& fps_info = stopwatch.AverageFpsInfo();
  std::stringstream stream;
  stream.setf(std::ios::fixed | std::ios::showpoint);
  stream << std::setprecision(1);
  stream << fps_info.average_fps << " FPS (jank " << fps_info.janky_frame_count
         << " in " << fps_info.frame_count << " frames, "
         << fps_info.total_time_ms << " ms), max=" << max_ms_per_frame
         << " ms, avg=" << average_ms_per_frame << " ms";
  auto text = stream.str();
  SkRect measured_bounds{};
  // The bounds measured by font is preciser than the SkTextBlob's bounds.
  font.measureText(text.c_str(), text.size(), SkTextEncoding::kUTF8,
                   &measured_bounds);
  return TextBlobAndBounds{
      .text_blob = SkTextBlob::MakeFromText(text.c_str(), text.size(), font,
                                            SkTextEncoding::kUTF8),
      .bounds = measured_bounds};
}

PerformanceOverlayLayer::PerformanceOverlayLayer(uint64_t options,
                                                 const char* font_path)
    : options_(options) {
  if (font_path != nullptr) {
    font_path_ = font_path;
  }
}

void PerformanceOverlayLayer::Diff(DiffContext* context,
                                   const Layer* old_layer) {
  DiffContext::AutoSubtreeRestore subtree(context);
  if (!context->IsSubtreeDirty()) {
    FML_DCHECK(old_layer);
    auto prev = old_layer->as_performance_overlay_layer();
    context->MarkSubtreeDirty(context->GetOldLayerPaintRegion(prev));
  }
  context->AddLayerBounds(paint_bounds());
  context->SetLayerPaintRegion(this, context->CurrentSubtreeRegion());
}

void PerformanceOverlayLayer::Paint(PaintContext& context) const {
  const int padding = 8;

  if (!options_) {
    return;
  }

  TRACE_EVENT0("flutter", "PerformanceOverlayLayer::Paint");
  SkScalar x = paint_bounds().x() + padding;
  SkScalar y = paint_bounds().y() + padding;
  SkScalar width = paint_bounds().width() - (padding * 2);
  SkScalar height = paint_bounds().height() / 2;
  SkAutoCanvasRestore save(context.leaf_nodes_canvas, true);

  VisualizeStopWatch(
      context.leaf_nodes_canvas, context.raster_time, x, y, width,
      height - padding, options_ & kVisualizeRasterizerStatistics,
      options_ & kDisplayRasterizerStatistics, "Raster", font_path_);

  VisualizeStopWatch(context.leaf_nodes_canvas, context.ui_time, x, y + height,
                     width, height - padding,
                     options_ & kVisualizeEngineStatistics,
                     options_ & kDisplayEngineStatistics, "UI", font_path_);
}

}  // namespace flutter

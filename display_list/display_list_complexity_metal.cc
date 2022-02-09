// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/display_list_complexity_metal.h"

// The numbers and weightings used in this file stem from taking the
// data from the DisplayListBenchmarks suite run on an iPhone 12 and
// applying very rough analysis on them to identify the approximate
// trends.
//
// Constants of proportionality and trends were chosen to better match
// larger numbers rather than smaller ones. This may turn out to be the
// wrong decision, but the rationale here is that with the smaller numbers,
// we have:
//
// a) More noise
// b) Less absolute difference. If the constant we've chosen is out by 50%
//    on a measurement that is 0.001ms, that's less of an issue than if
//    the measurement is 15ms.
//
// The error bars in measurement are likely quite large and will
// vary from device to device, so the goal here is not to provide a
// perfectly accurate representation of a given DisplayList's
// complexity but rather a very rough estimate that improves upon our
// previous cache admission policy (op_count > 5).
//
// There will likely be future work that will improve upon the figures
// in here. Notably, we do not take matrices or clips into account yet.

namespace flutter {

MetalHelper::MetalHelper() {
  paint_stack_.push(SkPaint());
}

void MetalHelper::setAntiAlias(bool aa) {
  paint_stack_.top().setAntiAlias(aa);
}

void MetalHelper::setStyle(SkPaint::Style style) {
  paint_stack_.top().setStyle(style);
}

void MetalHelper::setStrokeWidth(SkScalar width) {
  paint_stack_.top().setStrokeWidth(width);
}

void MetalHelper::save() {
  save_types_.push(kSave);
}

void MetalHelper::restore() {
  if (save_types_.top() == kSaveLayer) {
    paint_stack_.pop();
  }
  save_types_.pop();
}

void MetalHelper::saveLayer(const SkRect* bounds,
                            const SaveLayerOptions options) {
  // Ignore options here as we only look at style/AA and saveLayer ignores
  // both of those
  save_types_.push(kSaveLayer);
  paint_stack_.push(SkPaint());
}

void MetalHelper::drawColor(SkColor color, SkBlendMode mode) {
  // Placeholder value here. This is a relatively cheap operation.
  complexity_score_ += 50;
}

void MetalHelper::drawPaint() {}

// Normalise DrawLine to have a score of 100 when drawn with approximate line
// length of 100, AA disabled and a hairline stroke
// This represents a wall clock time of around 0.0005 milliseconds
void MetalHelper::drawLine(const SkPoint& p0, const SkPoint& p1) {
  // The performance penalties seem fairly consistent percentage-wise
  float nonHairlinePenalty = 1.2f;
  float aaPenalty = 1.3f;
  SkScalar distance = SkPoint::Distance(p0, p1);

  // Cost scales linearly with length approximately through the origin
  // So a line length of 500 would roughly have 5x the complexity cost
  // of a line length of 100
  unsigned int complexity = distance;
  if (isAntiAlias()) {
    complexity *= aaPenalty;
  }
  if (!isHairline()) {
    complexity *= nonHairlinePenalty;
  }

  complexity_score_ += complexity;
}

void MetalHelper::drawRect(const SkRect& rect) {
  // Hairline stroke vs non hairline has no real penalty here
  // There is a penalty for AA being *disabled* of around 50% with stroked styles
  // At very small sizes (rect area < 4000px^2) stroke vs filled is approximately
  // the same time cost
  float nonAAPenalty = 1.0f;
  if (style() == SkPaint::Style::kStroke_Style) {
    nonAAPenalty = 1.5f;
  }

  // Very roughly there's about a 7x penalty for filled vs stroked.
  float fillPenalty = 7.0f;

  // Cost scales approximately linearly with pixel area
  // For no attributes intersection is when area ~= 60000px^2
  // A coefficient of 2.5 gives an approximate mapping here to DrawLine.
  unsigned int area = rect.width() * rect.height();
  unsigned int complexity = (area / (60000 * 2.5)) * 100;

  if (!isAntiAlias()) {
    complexity *= nonAAPenalty;
  }

  if (style() == SkPaint::Style::kFill_Style) {
    complexity *= fillPenalty;
  }

  complexity_score_ += complexity;
}

void MetalHelper::drawOval(const SkRect& bounds) {
  // DrawOval scales very roughly linearly with the bounding box width/height
  // (not area)
  //
  // Take the average of the width and height
  float length = (bounds.width() + bounds.height()) / 2.0;

  // No penalty for hairline vs non-hairline stroke
  // Very small penalty for AA vs non-AA in filled style
  float aaPenalty = 1.0f;

  if (style() == SkPaint::Style::kStroke_Style) {
    // AA penalty for stroked styles is proportional to the oval length
    // with a constant of around 1/100.
    aaPenalty = length / 100f;
  }

  // Filled ovals have a penalty that is proportional to the oval
  // length.
  //
  // length/200 seems to give a reasonable approximation of the penalty
  // for a filled rect vs. a stroked rect.
  float fillPenalty = std::min(1.0f, length / 200f);

  // A length of 35 ~= baseline timing of 0.0005ms
  unsigned int complexity = (length / 35) * 100;

  if (isAntiAlias()) {
    complexity *= aaPenalty;
  }
  if (style() == SkPaint::Style::kFill_Style) {
    complexity *= fillPenalty;
  }

  complexity_score_ += complexity;
}

void MetalHelper::drawCircle(const SkPoint& center, SkScalar radius) {
  // No penalty for hairline vs non-hairline stroke
  // No penalty for AA for non-AA in filled style
  float aaPenalty = 1.0f;

  if (style() == SkPaint::Style::kStroke_Style) {
    // For stroked styles the data is a little noisy with no clear
    // trend, but the average is around 40%.
    aaPenalty = 1.4f;
  }

  // Filled circles have a penalty that is proportional to the radius.
  //
  // r/80 seems to give a reasonable approximation of the penalty
  // for a filled circle vs. a stroked circle.
  float fillPenalty = std::min(1.0f, length / 80f);

  // A radius of 250 ~= 10x baseline timing of 0.0005ms (0.005ms)
  unsigned int complexity = (radius / 250) * 1000 * aaPenalty;

  if (isAntiAlias()) {
    complexity *= aaPenalty;
  }
  if (style() == SkPaint::Style::kFill_Style) {
    complexity *= fillPenalty;
  }

  complexity_score_ += complexity;
}

void MetalHelper::drawRRect(const SkRRect& rrect) {}
void MetalHelper::drawDRRect(const SkRRect& outer, const SkRRect& inner) {}
void MetalHelper::drawPath(const SkPath& path) {}
void MetalHelper::drawArc(const SkRect& oval_bounds,
            SkScalar start_degrees,
            SkScalar sweep_degrees,
            bool use_center) {}
void MetalHelper::drawPoints(SkCanvas::PointMode mode,
                uint32_t count,
                const SkPoint points[]) {}
void MetalHelper::drawVertices(const sk_sp<SkVertices> vertices,
                  SkBlendMode mode) {}
void MetalHelper::drawImage(const sk_sp<SkImage> image,
              const SkPoint point,
              const SkSamplingOptions& sampling,
              bool render_with_attributes) {}
void MetalHelper::drawImageRect(const sk_sp<SkImage> image,
                  const SkRect& src,
                  const SkRect& dst,
                  const SkSamplingOptions& sampling,
                  bool render_with_attributes,
                  SkCanvas::SrcRectConstraint constraint) {}
void MetalHelper::drawImageNine(const sk_sp<SkImage> image,
                  const SkIRect& center,
                  const SkRect& dst,
                  SkFilterMode filter,
                  bool render_with_attributes) {}
void MetalHelper::drawImageLattice(const sk_sp<SkImage> image,
                      const SkCanvas::Lattice& lattice,
                      const SkRect& dst,
                      SkFilterMode filter,
                      bool render_with_attributes) {}
void MetalHelper::drawAtlas(const sk_sp<SkImage> atlas,
              const SkRSXform xform[],
              const SkRect tex[],
              const SkColor colors[],
              int count,
              SkBlendMode mode,
              const SkSamplingOptions& sampling,
              const SkRect* cull_rect,
              bool render_with_attributes) {}
void MetalHelper::drawPicture(const sk_sp<SkPicture> picture,
                const SkMatrix* matrix,
                bool render_with_attributes) {}
void MetalHelper::drawDisplayList(const sk_sp<DisplayList> display_list) {}
void MetalHelper::drawTextBlob(const sk_sp<SkTextBlob> blob,
                  SkScalar x,
                  SkScalar y) {}
void MetalHelper::drawShadow(const SkPath& path,
                const SkColor color,
                const SkScalar elevation,
                bool transparent_occluder,
                SkScalar dpr) {}

} // namespace flutter

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
// c) Smaller numbers affect the caching decision negligibly; the caching
//    decision is likely to be driven by slower ops rather than faster ones.
//
// In some cases, a cost penalty is used to figure out the cost of an
// attribute such as anti-aliasing or fill style. In some of these, the
// penalty is proportional to some other value such as the radius of
// a circle. In these cases, we ensure that the penalty is never smaller
// than 1.0f.
//
// The error bars in measurement are likely quite large and will
// vary from device to device, so the goal here is not to provide a
// perfectly accurate representation of a given DisplayList's
// complexity but rather a very rough estimate that improves upon our
// previous cache admission policy (op_count > 5).
//
// There will likely be future work that will improve upon the figures
// in here. Notably, we do not take matrices or clips into account yet.
//
// The scoring is based around a baseline score of 100 being roughly
// equivalent to 0.0005ms of time. With a 32-bit unsigned integer, this
// would set the maximum time estimate associated with the complexity score
// at about 21 seconds, which far exceeds what we would ever expect a
// DisplayList to take rasterising.

namespace flutter {

DisplayListMetalComplexityCalculator*
    DisplayListMetalComplexityCalculator::instance_ = nullptr;

DisplayListComplexityCalculator*
DisplayListMetalComplexityCalculator::GetInstance() {
  if (instance_ == nullptr) {
    instance_ = new DisplayListMetalComplexityCalculator();
  }
  return instance_;
}

DisplayListMetalComplexityCalculator::MetalHelper::MetalHelper()
    : is_complex_(false) {}

unsigned int
DisplayListMetalComplexityCalculator::MetalHelper::ComplexityScore() {
  // We overflowed, return int_max
  if (is_complex_) {
    return std::numeric_limits<unsigned int>::max();
  }

  // Calculate the impact of saveLayer
  unsigned int save_layer_complexity;
  if (save_layer_count_ > 200) {
    // m = 1/5
    // c = 1
    save_layer_complexity = (save_layer_count_ + 5) * 40000;
  } else {
    // m = 1/2
    // c = 1
    save_layer_complexity = (save_layer_count_ + 2) * 100000;
  }

  return complexity_score_ + save_layer_complexity;
}

void DisplayListMetalComplexityCalculator::MetalHelper::AccumulateComplexity(
    unsigned int complexity) {
  // Check to see if we will overflow by accumulating this complexity score
  if (std::numeric_limits<unsigned int>::max() - complexity < complexity) {
    is_complex_ = true;
    return;
  }

  complexity_score_ += complexity;
}

void DisplayListMetalComplexityCalculator::MetalHelper::setAntiAlias(bool aa) {
  current_paint_.setAntiAlias(aa);
}

void DisplayListMetalComplexityCalculator::MetalHelper::setStyle(
    SkPaint::Style style) {
  current_paint_.setStyle(style);
}

void DisplayListMetalComplexityCalculator::MetalHelper::setStrokeWidth(
    SkScalar width) {
  current_paint_.setStrokeWidth(width);
}

void DisplayListMetalComplexityCalculator::MetalHelper::saveLayer(
    const SkRect* bounds,
    const SaveLayerOptions options) {
  if (is_complex_) {
    return;
  }
  // saveLayer seems to have two trends; if the count is < 200,
  // then the individual cost of a saveLayer is higher than if
  // the count is > 200.
  save_layer_count_++;
}

void DisplayListMetalComplexityCalculator::MetalHelper::drawColor(
    SkColor color,
    SkBlendMode mode) {
  if (is_complex_) {
    return;
  }
  // Placeholder value here. This is a relatively cheap operation.
  AccumulateComplexity(50);
}

void DisplayListMetalComplexityCalculator::MetalHelper::drawPaint() {
  if (is_complex_) {
    return;
  }
  // Placeholder value here. This can be cheap (e.g. effectively a drawColor),
  // or expensive (e.g. a bitmap shader with an image filter)
  AccumulateComplexity(50);
}

// Normalise DrawLine to have a score of 100 when drawn with approximate line
// length of 100, AA disabled and a hairline stroke
// This represents a wall clock time of around 0.0005 milliseconds
void DisplayListMetalComplexityCalculator::MetalHelper::drawLine(
    const SkPoint& p0,
    const SkPoint& p1) {
  if (is_complex_) {
    return;
  }
  // The performance penalties seem fairly consistent percentage-wise
  float non_hairline_penalty = 1.0f;
  float aa_penalty = 1.0f;

  if (!IsHairline()) {
    non_hairline_penalty = 1.2f;
  }
  if (IsAntiAliased()) {
    aa_penalty = 1.3f;
  }

  SkScalar distance = SkPoint::Distance(p0, p1);

  // Cost scales linearly with length approximately through the origin
  // So a line length of 500 would roughly have 5x the complexity cost
  // of a line length of 100
  unsigned int complexity = distance * aa_penalty * non_hairline_penalty;

  AccumulateComplexity(complexity);
}

void DisplayListMetalComplexityCalculator::MetalHelper::drawRect(
    const SkRect& rect) {
  if (is_complex_) {
    return;
  }
  // Hairline stroke vs non hairline has no real penalty here
  // There is a penalty for AA being *disabled* of around 50% with stroked
  // styles At very small sizes (rect area < 4000px^2) stroke vs filled is
  // approximately the same time cost
  float nonAA_penalty = 1.0f;
  float fill_penalty = 1.0f;

  if (!IsAntiAliased() && Style() == SkPaint::Style::kStroke_Style) {
    nonAA_penalty = 1.5f;
  }

  // Very roughly there's about a 7x penalty for filled vs stroked.
  if (Style() == SkPaint::Style::kFill_Style) {
    fill_penalty = 7.0f;
  }

  // Cost scales approximately linearly with pixel area
  // For no attributes intersection is when area ~= 60000px^2
  // A coefficient of 2.5 gives an approximate mapping here to DrawLine.
  unsigned int area = rect.width() * rect.height();
  unsigned int complexity =
      (area / (60000 * 2.5)) * 100 * nonAA_penalty * fill_penalty;

  AccumulateComplexity(complexity);
}

void DisplayListMetalComplexityCalculator::MetalHelper::drawOval(
    const SkRect& bounds) {
  if (is_complex_) {
    return;
  }
  // DrawOval scales very roughly linearly with the bounding box width/height
  // (not area)
  //
  // Take the average of the width and height
  unsigned int length = (bounds.width() + bounds.height()) / 2;

  // No penalty for hairline vs non-hairline stroke
  // Very small penalty for AA vs non-AA in filled style
  float aa_penalty = 1.0f;
  float fill_penalty = 1.0f;

  if (Style() == SkPaint::Style::kStroke_Style) {
    // AA penalty for stroked styles is proportional to the oval length
    // with a constant of around 1/100.
    aa_penalty = length / 100.0f;
  }

  // Filled ovals have a penalty that is proportional to the oval
  // length.
  //
  // length/200 seems to give a reasonable approximation of the penalty
  // for a filled rect vs. a stroked rect.
  if (Style() == SkPaint::Style::kFill_Style) {
    fill_penalty = std::max(1.0f, length / 200.0f);
  }

  // A length of 35 ~= baseline timing of 0.0005ms
  unsigned int complexity = (length / 35) * 100 * aa_penalty * fill_penalty;

  AccumulateComplexity(complexity);
}

void DisplayListMetalComplexityCalculator::MetalHelper::drawCircle(
    const SkPoint& center,
    SkScalar radius) {
  if (is_complex_) {
    return;
  }
  // No penalty for hairline vs non-hairline stroke
  // No penalty for AA for non-AA in filled style
  float aa_penalty = 1.0f;
  float fill_penalty = 1.0f;

  if (Style() == SkPaint::Style::kStroke_Style) {
    // For stroked styles the data is a little noisy with no clear
    // trend, but the average is around 40%.
    aa_penalty = 1.4f;
  }

  // Filled circles have a penalty that is proportional to the radius.
  //
  // r/80 seems to give a reasonable approximation of the penalty
  // for a filled circle vs. a stroked circle.
  if (Style() == SkPaint::Style::kFill_Style) {
    fill_penalty = std::max(1.0f, radius / 80.0f);
  }

  // A radius of 250 ~= 10x baseline timing of 0.0005ms (0.005ms)
  unsigned int complexity = (radius / 250) * 1000 * aa_penalty * fill_penalty;

  AccumulateComplexity(complexity);
}

void DisplayListMetalComplexityCalculator::MetalHelper::drawRRect(
    const SkRRect& rrect) {
  if (is_complex_) {
    return;
  }
  // RRects scale linearly with the area of the bounding rect
  unsigned int area = rrect.width() * rrect.height();

  // Drawing RRects is split into two performance tiers; an expensive
  // one and a less expensive one. Both scale linearly with area.
  //
  // Expensive: All filled style, symmetric w/AA
  bool expensive =
      (Style() == SkPaint::Style::kFill_Style) ||
      ((rrect.getType() == SkRRect::Type::kSimple_Type) && IsAntiAliased());

  unsigned int complexity;

  // These values were worked out by creating a straight line graph (y=mx+c)
  // approximately matching the measured data, normalising the data so that
  // 0.0005ms resulted in a score of 100 then simplifying down the formula.
  if (expensive) {
    // m = 1/25000
    // c = 2
    // An area of 7000px^2 ~= baseline timing of 0.0005ms
    complexity = (area + 10500) / 175;
  } else {
    // m = 1/7000
    // c = 1.5
    // An area of 16000px^2 ~= baseline timing of 0.0005ms
    complexity = (area + 50000) / 625;
  }

  AccumulateComplexity(complexity);
}

void DisplayListMetalComplexityCalculator::MetalHelper::drawDRRect(
    const SkRRect& outer,
    const SkRRect& inner) {
  if (is_complex_) {
    return;
  }
  // There are roughly three classes here:
  // a) Filled style with AA enabled
  // b) Filled style with AA disabled
  // c) Everything else
  //
  // a) scales linearly with the area, b) and c) scale linearly with
  // a single dimension (length). In both cases, the dimensions refer to
  // the outer RRect.
  unsigned int area = outer.width() * outer.height();
  unsigned int length = (outer.width() + outer.height()) / 2;

  unsigned int complexity;

  // These values were worked out by creating a straight line graph (y=mx+c)
  // approximately matching the measured data, normalising the data so that
  // 0.0005ms resulted in a score of 100 then simplifying down the formula.
  if (Style() == SkPaint::Style::kFill_Style) {
    if (IsAntiAliased()) {
      // m = 1/3500
      // c = 1.5
      complexity = (area + 5250) / 35;
    } else {
      // m = 1/30
      // c = 1
      complexity = (300 + (10 * length)) / 3;
    }
  } else {
    // m = 1/60
    // c = 1.75
    complexity = ((10 * length) + 1050) / 6;
  }

  AccumulateComplexity(complexity);
}

void DisplayListMetalComplexityCalculator::MetalHelper::drawPath(
    const SkPath& path) {
  if (is_complex_) {
    return;
  }
  // There is negligible effect on the performance for hairline vs. non-hairline
  // stroke widths.
  //
  // The data for filled styles is currently suspicious, so for now we are going
  // to assign scores based on stroked styles.

  unsigned int line_verb_cost, quad_verb_cost, conic_verb_cost, cubic_verb_cost;

  if (IsAntiAliased()) {
    line_verb_cost = 75;
    quad_verb_cost = 100;
    conic_verb_cost = 160;
    cubic_verb_cost = 210;
  } else {
    line_verb_cost = 67;
    quad_verb_cost = 80;
    conic_verb_cost = 140;
    cubic_verb_cost = 210;
  }

  // There seems to be a fixed cost of around 1ms for calling drawPath
  unsigned int complexity = 200000;

  int verb_count = path.countVerbs();
  uint8_t verbs[verb_count];
  path.getVerbs(verbs, verb_count);

  for (int i = 0; i < verb_count; i++) {
    switch (verbs[i]) {
      case SkPath::Verb::kLine_Verb:
        complexity += line_verb_cost;
        break;
      case SkPath::Verb::kQuad_Verb:
        complexity += quad_verb_cost;
        break;
      case SkPath::Verb::kConic_Verb:
        complexity += conic_verb_cost;
        break;
      case SkPath::Verb::kCubic_Verb:
        complexity += cubic_verb_cost;
        break;
    }
  }

  AccumulateComplexity(complexity);
}

void DisplayListMetalComplexityCalculator::MetalHelper::drawArc(
    const SkRect& oval_bounds,
    SkScalar start_degrees,
    SkScalar sweep_degrees,
    bool use_center) {
  if (is_complex_) {
    return;
  }
  // Hairline vs non-hairline makes no difference to the performance
  // Stroked styles without AA scale linearly with the diameter
  // Stroked styles with AA scale linearly with the area except for small values
  // Filled styles scale lienarly with the area
  unsigned int diameter = (oval_bounds.width() + oval_bounds.height()) / 2;
  unsigned int area = oval_bounds.width() * oval_bounds.height();

  unsigned int complexity;

  // These values were worked out by creating a straight line graph (y=mx+c)
  // approximately matching the measured data, normalising the data so that
  // 0.0005ms resulted in a score of 100 then simplifying down the formula.
  if (Style() == SkPaint::Style::kStroke_Style) {
    if (IsAntiAliased()) {
      // m = 1/8500
      // c = 16
      complexity = (area + 136000) * 2 / 765;
    } else {
      // m = 1/60
      // c = 3
      complexity = (diameter + 180) * 10 / 27;
    }
  } else {
    if (IsAntiAliased()) {
      // m = 1/20000
      // c = 20
      complexity = (area + 400000) / 900;
    } else {
      // m = 1/2100
      // c = 8
      complexity = (area + 16800) * 2 / 189;
    }
  }

  AccumulateComplexity(complexity);
}

void DisplayListMetalComplexityCalculator::MetalHelper::drawPoints(
    SkCanvas::PointMode mode,
    uint32_t count,
    const SkPoint points[]) {
  if (is_complex_) {
    return;
  }
  unsigned int complexity;

  // If AA is off then they all behave similarly, and scale
  // linearly with the point count
  if (!IsAntiAliased()) {
    // m = 1/16000
    // c = 0.75
    complexity = (count + 12000) * 25 / 2;
  } else {
    if (mode == SkCanvas::kPolygon_PointMode) {
      // m = 1/1250
      // c = 1
      complexity = (count + 1250) * 160;
    } else {
      if (IsHairline() && mode == SkCanvas::kPoints_PointMode) {
        // This is a special case, it triggers an extremely fast path
        // m = 1/14500
        // c = 0
        complexity = count * 400 / 29;
      } else {
        // m = 1/2200
        // c = 0.75
        complexity = (count + 1650) * 1000 / 11;
      }
    }
  }
  AccumulateComplexity(complexity);
}

void DisplayListMetalComplexityCalculator::MetalHelper::drawVertices(
    const sk_sp<SkVertices> vertices,
    SkBlendMode mode) {
  // There is currently no way for us to get the VertexMode from the SkVertices
  // object, but for future reference:
  //
  // TriangleStrip is roughly 25% more expensive than TriangleFan
  // TriangleFan is roughly 5% more expensive than Triangles

  // There is currently no way for us to get the vertex count from an SkVertices
  // object, so we have to estimate it from the approximate size.
  //
  // Approximate size returns the sum of the sizes of the positions (SkPoint),
  // texs (SkPoint), colors (SkColor) and indices (uint16_t) arrays multiplied
  // by sizeof(type). As a very, very rough estimate, divide that by 20 to get
  // an idea of the vertex count.
  unsigned int approximate_vertex_count = vertices->approximateSize() / 20;

  // For the baseline, it's hard to identify the trend. It might be O(n^1/2)
  // For now, treat it as linear as an approximation.
  unsigned int complexity = (approximate_vertex_count + 4000) * 50;

  AccumulateComplexity(complexity);
}

void DisplayListMetalComplexityCalculator::MetalHelper::drawImage(
    const sk_sp<SkImage> image,
    const SkPoint point,
    const SkSamplingOptions& sampling,
    bool render_with_attributes) {
  if (is_complex_) {
    return;
  }
  // AA vs non-AA has a cost but it's dwarfed by the overall cost of the
  // drawImage call.
  //
  // The main difference is if the image is backed by a texture already or not
  // If we don't need to upload, then the cost scales linearly with the
  // area of the image. If it needs uploading, the cost scales linearly
  // with the square of the area (!!!).
  SkISize dimensions = image->dimensions();
  unsigned int area = dimensions.width() * dimensions.height();

  // m = 1/17000
  // c = 3
  unsigned int complexity = (area + 51000) * 4 / 170;

  if (!image->isTextureBacked()) {
    // We can't square the area here as we'll overflow, so let's approximate
    // by taking the calculated complexity score and applying a multiplier to it
    //
    // (complexity * area / 35000) + 3 gives a reasonable approximation.
    float multiplier = area / 35000.0f;
    complexity = complexity * multiplier + 3;
  }

  AccumulateComplexity(complexity);
}

void DisplayListMetalComplexityCalculator::MetalHelper::ImageRect(
    const SkISize& size,
    bool texture_backed,
    bool render_with_attributes,
    SkCanvas::SrcRectConstraint constraint) {
  if (is_complex_) {
    return;
  }
  // Two main groups here - texture-backed and non-texture-backed images
  // Within each group, they all perform within a few % of each other *except*
  // when we have a strict constraint and anti-aliasing enabled.
  unsigned int area = size.width() * size.height();

  // These values were worked out by creating a straight line graph (y=mx+c)
  // approximately matching the measured data, normalising the data so that
  // 0.0005ms resulted in a score of 100 then simplifying down the formula.
  unsigned int complexity;
  if (texture_backed) {
    // Baseline for texture-backed SkImages
    // m = 1/23000
    // c = 2.3
    complexity = (area + 52900) * 2 / 115;
    if (render_with_attributes &&
        constraint == SkCanvas::SrcRectConstraint::kStrict_SrcRectConstraint &&
        IsAntiAliased()) {
      // There's about a 30% performance penalty from the baseline
      complexity *= 1.3f;
    }
  } else {
    if (render_with_attributes &&
        constraint == SkCanvas::SrcRectConstraint::kStrict_SrcRectConstraint &&
        IsAntiAliased()) {
      // m = 1/12200
      // c = 2.75
      complexity = (area + 33550) * 2 / 61;
    } else {
      // m = 1/14500
      // c = 2.5
      complexity = (area + 36250) * 4 / 145;
    }
  }

  AccumulateComplexity(complexity);
}

void DisplayListMetalComplexityCalculator::MetalHelper::drawImageRect(
    const sk_sp<SkImage> image,
    const SkRect& src,
    const SkRect& dst,
    const SkSamplingOptions& sampling,
    bool render_with_attributes,
    SkCanvas::SrcRectConstraint constraint) {
  if (is_complex_) {
    return;
  }
  ImageRect(image->dimensions(), image->isTextureBacked(),
            render_with_attributes, constraint);
}

void DisplayListMetalComplexityCalculator::MetalHelper::drawImageNine(
    const sk_sp<SkImage> image,
    const SkIRect& center,
    const SkRect& dst,
    SkFilterMode filter,
    bool render_with_attributes) {
  if (is_complex_) {
    return;
  }
  // Whether uploading or not, the performance is comparable across all
  // variations
  SkISize dimensions = image->dimensions();
  unsigned int area = dimensions.width() * dimensions.height();

  // m = 1/8000
  // c = 3
  unsigned int complexity = (area + 24000) / 20;
  AccumulateComplexity(complexity);
}

void DisplayListMetalComplexityCalculator::MetalHelper::drawImageLattice(
    const sk_sp<SkImage> image,
    const SkCanvas::Lattice& lattice,
    const SkRect& dst,
    SkFilterMode filter,
    bool render_with_attributes) {
  if (is_complex_) {
    return;
  }
  // This is not currently called from Flutter code, and this API is likely
  // to be removed in the future. For now, just return what drawImageNine would
  ImageRect(image->dimensions(), image->isTextureBacked(),
            render_with_attributes,
            SkCanvas::SrcRectConstraint::kStrict_SrcRectConstraint);
}

void DisplayListMetalComplexityCalculator::MetalHelper::drawAtlas(
    const sk_sp<SkImage> atlas,
    const SkRSXform xform[],
    const SkRect tex[],
    const SkColor colors[],
    int count,
    SkBlendMode mode,
    const SkSamplingOptions& sampling,
    const SkRect* cull_rect,
    bool render_with_attributes) {
  if (is_complex_) {
    return;
  }
  // This API just does a series of drawImage calls from the atlas
  // This is equivalent to calling drawImageRect lots of times
  for (int i = 0; i < count; i++) {
    ImageRect(SkISize::Make(tex[i].width(), tex[i].height()), true,
              render_with_attributes,
              SkCanvas::SrcRectConstraint::kStrict_SrcRectConstraint);
  }
}

void DisplayListMetalComplexityCalculator::MetalHelper::drawDisplayList(
    const sk_sp<DisplayList> display_list) {
  if (is_complex_) {
    return;
  }
  MetalHelper helper;
  display_list->Dispatch(helper);
  AccumulateComplexity(helper.ComplexityScore());
}

void DisplayListMetalComplexityCalculator::MetalHelper::drawTextBlob(
    const sk_sp<SkTextBlob> blob,
    SkScalar x,
    SkScalar y) {
  if (is_complex_) {
    return;
  }
  // There are two classes here, hairline vs non-hairline.
  // Hairline scales loglinearly with the number of glyphs
  // Non-hairline scales linearly

  // Unfortunately there is currently no way for us to figure out the glyph
  // count from an SkTextBlob. We will need to figure out a better solution
  // here, but for now just use a placeholder value of 100 glyphs.
  unsigned int glyph_count = 100;

  // These values were worked out by creating a straight line graph (y=mx+c)
  // approximately matching the measured data, normalising the data so that
  // 0.0005ms resulted in a score of 100 then simplifying down the formula.
  unsigned int complexity;
  if (IsHairline()) {
    // m = 1/5000
    // c = 0.75
    complexity = 40 * (glyph_count + 3750);
  } else {
    // m = 1/3000
    // c = 1.75
    // x = glyph_count * log2(glyph_count)
    unsigned int x = glyph_count * log(glyph_count);
    complexity = (x + 5250) * 200 / 3;
  }

  AccumulateComplexity(complexity);
}

void DisplayListMetalComplexityCalculator::MetalHelper::drawShadow(
    const SkPath& path,
    const SkColor color,
    const SkScalar elevation,
    bool transparent_occluder,
    SkScalar dpr) {
  if (is_complex_) {
    return;
  }

  // Elevation has no significant effect on the timings. Whether the shadow
  // is cast by a transparent occluder or not has a small impact of around 5%.
  //
  // The path verbs do have an effect but only if the verb type is cubic; line,
  // quad and conic all perform similarly.
  float occluder_penalty = 1.0f;
  if (transparent_occluder) {
    occluder_penalty = 1.05f;
  }

  // The benchmark uses a test path of around 10 path elements. This is likely
  // to be similar to what we see in real world usage, but we should benchmark
  // different path lengths to see how much impact there is from varying the
  // path length.
  //
  // For now, we will assume that there is no fixed overhead and that the time
  // spent rendering the shadow for a path is split evenly amongst all the verbs
  // enumerated.
  unsigned int line_verb_cost = 20000;   // 0.1ms
  unsigned int quad_verb_cost = 20000;   // 0.1ms
  unsigned int conic_verb_cost = 20000;  // 0.1ms
  unsigned int cubic_verb_cost = 80000;  // 0.4ms

  unsigned int complexity = 0;

  int verb_count = path.countVerbs();
  uint8_t verbs[verb_count];
  path.getVerbs(verbs, verb_count);

  for (int i = 0; i < verb_count; i++) {
    switch (verbs[i]) {
      case SkPath::Verb::kLine_Verb:
        complexity += line_verb_cost;
        break;
      case SkPath::Verb::kQuad_Verb:
        complexity += quad_verb_cost;
        break;
      case SkPath::Verb::kConic_Verb:
        complexity += conic_verb_cost;
        break;
      case SkPath::Verb::kCubic_Verb:
        complexity += cubic_verb_cost;
        break;
    }
  }

  AccumulateComplexity(complexity * occluder_penalty);
}

}  // namespace flutter

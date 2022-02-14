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
//
// Finally, care has been taken to keep the computations as cheap as possible.
// We need to be able to calculate the complexity as quickly as possible
// so that we don't end up wasting too much time figuring out if something
// should be cached or not and eat into the time we could have just spent
// rasterising the DisplayList.
//
// In order to keep the computations cheap, the following tradeoffs were made:
//
// a) Limit all computations to simple division, multiplication, addition
//    and subtraction.
// b) Try and use integer arithmetic as much as possible.
// c) If a specific draw op is logarithmic in complexity, do a best fit
//    onto a linear equation within the range we expect to see the variables
//    fall within.

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

DisplayListMetalComplexityCalculator::MetalHelper::MetalHelper(
    unsigned int ceiling)
    : is_complex_(false), ceiling_(ceiling), save_layer_count_(0) {}

unsigned int
DisplayListMetalComplexityCalculator::MetalHelper::ComplexityScore() {
  // We hit our ceiling, so return that
  if (is_complex_) {
    return ceiling_;
  }

  // Calculate the impact of saveLayer.
  unsigned int save_layer_complexity;
  if (save_layer_count_ == 0) {
    save_layer_complexity = 0;
  } else {
    // saveLayer seems to have two trends; if the count is < 200,
    // then the individual cost of a saveLayer is higher than if
    // the count is > 200.
    if (save_layer_count_ > 200) {
      // m = 1/5
      // c = 1
      save_layer_complexity = (save_layer_count_ + 5) * 40000;
    } else {
      // m = 1/2
      // c = 1
      save_layer_complexity = (save_layer_count_ + 2) * 100000;
    }
  }

  return complexity_score_ + save_layer_complexity;
}

void DisplayListMetalComplexityCalculator::MetalHelper::AccumulateComplexity(
    unsigned int complexity) {
  // Check to see if we will overflow by accumulating this complexity score
  if (ceiling_ - complexity_score_ < complexity) {
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

void DisplayListMetalComplexityCalculator::MetalHelper::drawLine(
    const SkPoint& p0,
    const SkPoint& p1) {
  if (is_complex_) {
    return;
  }
  // The curve here may be log-linear, although it doesn't really match up that
  // well. To avoid costly computations, try and do a best fit of the data onto
  // a linear graph as a very rough first order approximation.

  float non_hairline_penalty = 1.0f;
  float aa_penalty = 1.0f;

  if (!IsHairline()) {
    non_hairline_penalty = 1.15f;
  }
  if (IsAntiAliased()) {
    aa_penalty = 1.4f;
  }

  // Use an approximation for the distance to avoid floating point or
  // sqrt() calls.
  SkScalar distance = abs(p0.x() - p1.x()) + abs(p0.y() - p1.y());

  // The baseline complexity is for a hairline stroke with no AA
  // m = 1/45
  // c = 5
  unsigned int complexity =
      ((distance + 225) * 4 / 9) * non_hairline_penalty * aa_penalty;

  AccumulateComplexity(complexity);
}

void DisplayListMetalComplexityCalculator::MetalHelper::drawRect(
    const SkRect& rect) {
  if (is_complex_) {
    return;
  }

  unsigned int complexity;

  // If stroked, cost scales linearly with the rectangle width/height.
  // If filled, it scales with the area.
  //
  // Hairline stroke vs non hairline has no real penalty at smaller lengths,
  // but it increases at larger lengths. There isn't enough data to get a good
  // idea of the penalty at lengths > 1000px.
  //
  // There is also a kStrokeAndFill_Style that Skia exposes, but we do not
  // currently use it anywhere in Flutter.
  if (Style() == SkPaint::Style::kFill_Style) {
    // No real difference for AA with filled styles
    unsigned int area = rect.width() * rect.height();

    // m = 1/9000
    // c = 0
    complexity = area / 225;
  } else {
    // Take the average of the width and height
    unsigned int length = (rect.width() + rect.height()) / 2;

    // There is a penalty for AA being *disabled*
    if (IsAntiAliased()) {
      // m = 1/65
      // c = 0
      complexity = length * 8 / 13;
    } else {
      // m = 1/35
      // c = 0
      complexity = length * 8 / 7;
    }
  }

  AccumulateComplexity(complexity);
}

void DisplayListMetalComplexityCalculator::MetalHelper::drawOval(
    const SkRect& bounds) {
  if (is_complex_) {
    return;
  }
  // DrawOval scales very roughly linearly with the bounding box width/height
  // (not area) for stroked styles without AA.
  //
  // Filled styles and stroked styles with AA scale linearly with the bounding
  // box area.
  unsigned int area = bounds.width() * bounds.height();

  unsigned int complexity;

  // There is also a kStrokeAndFill_Style that Skia exposes, but we do not
  // currently use it anywhere in Flutter.
  if (Style() == SkPaint::Style::kFill_Style) {
    // With filled styles, there is no significant AA penalty
    // m = 1/16000
    // c = 0
    complexity = area / 80;
  } else {
    if (IsAntiAliased()) {
      // m = 1/7500
      // c = 0
      complexity = area * 2 / 75;
    } else {
      // Take the average of the width and height
      unsigned int length = (bounds.width() + bounds.height()) / 2;

      // m = 1/80
      // c = 0
      complexity = length * 5 / 2;
    }
  }

  AccumulateComplexity(complexity);
}

void DisplayListMetalComplexityCalculator::MetalHelper::drawCircle(
    const SkPoint& center,
    SkScalar radius) {
  if (is_complex_) {
    return;
  }

  unsigned int complexity;

  // There is also a kStrokeAndFill_Style that Skia exposes, but we do not
  // currently use it anywhere in Flutter.
  if (Style() == SkPaint::Style::kFill_Style) {
    // We can ignore pi here
    unsigned int area = radius * radius;
    // m = 1/1300
    // c = 5
    complexity = (area + 6500) * 2 / 65;

    // Penalty of around 5% when AA is disabled
    if (!IsAntiAliased()) {
      complexity *= 1.05f;
    }
  } else {
    // Hairline vs non-hairline has no significant performance difference
    if (IsAntiAliased()) {
      // m = 1/7
      // c = 7
      complexity = (radius + 49) * 40 / 7;
    } else {
      // m = 1/16
      // c = 8
      complexity = (radius + 128) * 5 / 2;
    }
  }

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
  // There are roughly four classes here:
  // a) Filled style with AA enabled
  // b) Filled style with AA disabled
  // c) Complex RRect type with AA enabled and filled style
  // d) Everything else
  //
  // a) and c) scale linearly with the area, b) and d) scale linearly with
  // a single dimension (length). In both cases, the dimensions refer to
  // the outer RRect.
  unsigned int length = (outer.width() + outer.height()) / 2;

  unsigned int complexity;

  // These values were worked out by creating a straight line graph (y=mx+c)
  // approximately matching the measured data, normalising the data so that
  // 0.0005ms resulted in a score of 100 then simplifying down the formula.
  //
  // There is also a kStrokeAndFill_Style that Skia exposes, but we do not
  // currently use it anywhere in Flutter.
  if (Style() == SkPaint::Style::kFill_Style) {
    unsigned int area = outer.width() * outer.height();
    if (outer.getType() == SkRRect::Type::kComplex_Type) {
      // m = 1/1000
      // c = 1
      complexity = (area + 1000) / 10;
    } else {
      if (IsAntiAliased()) {
        // m = 1/3500
        // c = 1.5
        complexity = (area + 5250) / 35;
      } else {
        // m = 1/30
        // c = 1
        complexity = (300 + (10 * length)) / 3;
      }
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
  //
  // There is also a kStrokeAndFill_Style that Skia exposes, but we do not
  // currently use it anywhere in Flutter.
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

void DisplayListMetalComplexityCalculator::MetalHelper::drawPicture(
    const sk_sp<SkPicture> picture,
    const SkMatrix* matrix,
    bool render_with_attributes) {
  // This API shouldn't be used, but for now just take the approximateOpCount()
  // and multiply by 50 as a placeholder.
  AccumulateComplexity(picture->approximateOpCount() * 50);
}

void DisplayListMetalComplexityCalculator::MetalHelper::drawDisplayList(
    const sk_sp<DisplayList> display_list) {
  if (is_complex_) {
    return;
  }
  MetalHelper helper(ceiling_ - complexity_score_);
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

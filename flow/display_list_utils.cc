// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <math.h>
#include <type_traits>

#include "flutter/flow/display_list_utils.h"
#include "flutter/flow/layers/physical_shape_layer.h"
#include "flutter/fml/logging.h"

#include "third_party/skia/include/core/SkMaskFilter.h"
#include "third_party/skia/include/core/SkPath.h"
#include "third_party/skia/include/core/SkRSXform.h"
#include "third_party/skia/include/core/SkTextBlob.h"
#include "third_party/skia/include/utils/SkShadowUtils.h"

// This header file cannot be included here, but we cannot
// record calls made by the SkShadowUtils without it.
// #include "third_party/skia/src/core/SkDrawShadowInfo.h"

namespace flutter {

// clang-format off
constexpr float invert_color_matrix[20] = {
  -1.0,    0,    0, 1.0, 0,
     0, -1.0,    0, 1.0, 0,
     0,    0, -1.0, 1.0, 0,
   1.0,  1.0,  1.0, 1.0, 0
};
// clang-format on

void SkPaintDispatchHelper::setAA(bool aa) {
  paint_.setAntiAlias(aa);
}
void SkPaintDispatchHelper::setDither(bool dither) {
  paint_.setDither(dither);
}
void SkPaintDispatchHelper::setInvertColors(bool invert) {
  invert_colors_ = invert;
  paint_.setColorFilter(makeColorFilter());
}
void SkPaintDispatchHelper::setCaps(SkPaint::Cap cap) {
  paint_.setStrokeCap(cap);
}
void SkPaintDispatchHelper::setJoins(SkPaint::Join join) {
  paint_.setStrokeJoin(join);
}
void SkPaintDispatchHelper::setDrawStyle(SkPaint::Style style) {
  paint_.setStyle(style);
}
void SkPaintDispatchHelper::setStrokeWidth(SkScalar width) {
  paint_.setStrokeWidth(width);
}
void SkPaintDispatchHelper::setMiterLimit(SkScalar limit) {
  paint_.setStrokeMiter(limit);
}
void SkPaintDispatchHelper::setColor(SkColor color) {
  paint_.setColor(color);
}
void SkPaintDispatchHelper::setBlendMode(SkBlendMode mode) {
  paint_.setBlendMode(mode);
}
void SkPaintDispatchHelper::setFilterQuality(SkFilterQuality quality) {
  paint_.setFilterQuality(quality);
}
void SkPaintDispatchHelper::setShader(sk_sp<SkShader> shader) {
  paint_.setShader(shader);
}
void SkPaintDispatchHelper::setImageFilter(sk_sp<SkImageFilter> filter) {
  paint_.setImageFilter(filter);
}
void SkPaintDispatchHelper::setColorFilter(sk_sp<SkColorFilter> filter) {
  color_filter_ = filter;
  paint_.setColorFilter(makeColorFilter());
}
void SkPaintDispatchHelper::setMaskFilter(sk_sp<SkMaskFilter> filter) {
  paint_.setMaskFilter(filter);
}
void SkPaintDispatchHelper::setMaskBlurFilter(SkBlurStyle style,
                                              SkScalar sigma) {
  paint_.setMaskFilter(SkMaskFilter::MakeBlur(style, sigma));
}

sk_sp<SkColorFilter> SkPaintDispatchHelper::makeColorFilter() {
  if (!invert_colors_) {
    return color_filter_;
  }
  sk_sp<SkColorFilter> invert_filter =
      SkColorFilters::Matrix(invert_color_matrix);
  if (color_filter_) {
    invert_filter = invert_filter->makeComposed(color_filter_);
  }
  return invert_filter;
}

void SkMatrixDispatchHelper::translate(SkScalar tx, SkScalar ty) {
  matrix_.preTranslate(tx, ty);
}
void SkMatrixDispatchHelper::scale(SkScalar sx, SkScalar sy) {
  matrix_.preScale(sx, sy);
}
void SkMatrixDispatchHelper::rotate(SkScalar degrees) {
  matrix_.preRotate(degrees);
}
void SkMatrixDispatchHelper::skew(SkScalar sx, SkScalar sy) {
  matrix_.preSkew(sx, sy);
}
void SkMatrixDispatchHelper::transform2x3(SkScalar mxx,
                                          SkScalar mxy,
                                          SkScalar mxt,
                                          SkScalar myx,
                                          SkScalar myy,
                                          SkScalar myt) {
  matrix_.preConcat(SkMatrix::MakeAll(mxx, mxy, mxt, myx, myy, myt, 0, 0, 1));
}
void SkMatrixDispatchHelper::transform3x3(SkScalar mxx,
                                          SkScalar mxy,
                                          SkScalar mxt,
                                          SkScalar myx,
                                          SkScalar myy,
                                          SkScalar myt,
                                          SkScalar px,
                                          SkScalar py,
                                          SkScalar pt) {
  matrix_.preConcat(
      SkMatrix::MakeAll(mxx, mxy, mxt, myx, myy, myt, px, py, pt));
}
void SkMatrixDispatchHelper::save() {
  saved_.push_back(matrix_);
}
void SkMatrixDispatchHelper::restore() {
  matrix_ = saved_.back();
  saved_.pop_back();
}
void SkMatrixDispatchHelper::reset() {
  matrix_.reset();
}

void ClipBoundsDispatchHelper::clipRect(const SkRect& rect,
                                        bool isAA,
                                        SkClipOp clip_op) {
  if (clip_op == SkClipOp::kIntersect) {
    intersect(rect);
  }
}
void ClipBoundsDispatchHelper::clipRRect(const SkRRect& rrect,
                                         bool isAA,
                                         SkClipOp clip_op) {
  if (clip_op == SkClipOp::kIntersect) {
    intersect(rrect.getBounds());
  }
}
void ClipBoundsDispatchHelper::clipPath(const SkPath& path,
                                        bool isAA,
                                        SkClipOp clip_op) {
  if (clip_op == SkClipOp::kIntersect) {
    intersect(path.getBounds());
  }
}
void ClipBoundsDispatchHelper::intersect(const SkRect& rect) {
  SkRect devClipBounds = matrix().mapRect(rect);
  if (!bounds_.intersect(devClipBounds)) {
    bounds_.setEmpty();
  }
}
void ClipBoundsDispatchHelper::save() {
  saved_.push_back(bounds_);
}
void ClipBoundsDispatchHelper::restore() {
  bounds_ = saved_.back();
  saved_.pop_back();
}

void DisplayListBoundsCalculator::saveLayer(const SkRect* bounds,
                                            bool withPaint) {
  SkMatrixDispatchHelper::save();
  ClipBoundsDispatchHelper::save();
  SaveInfo info =
      withPaint ? SaveLayerWithPaintInfo(this, accumulator_, matrix(), paint())
                : SaveLayerInfo(accumulator_, matrix());
  savedInfos_.push_back(info);
  accumulator_ = info.save();
  SkMatrixDispatchHelper::reset();
}
void DisplayListBoundsCalculator::save() {
  SkMatrixDispatchHelper::save();
  ClipBoundsDispatchHelper::save();
  SaveInfo info = SaveInfo(accumulator_);
  savedInfos_.push_back(info);
  accumulator_ = info.save();
}
void DisplayListBoundsCalculator::restore() {
  if (!savedInfos_.empty()) {
    SkMatrixDispatchHelper::restore();
    ClipBoundsDispatchHelper::restore();
    SaveInfo info = savedInfos_.back();
    savedInfos_.pop_back();
    accumulator_ = info.restore();
  }
}

void DisplayListBoundsCalculator::drawPaint() {
  if (!boundsCull_.isEmpty()) {
    baseAccumulator_.accumulate(boundsCull_);
  }
}
void DisplayListBoundsCalculator::drawColor(SkColor color, SkBlendMode mode) {
  if (!boundsCull_.isEmpty()) {
    baseAccumulator_.accumulate(boundsCull_);
  }
}
void DisplayListBoundsCalculator::drawLine(const SkPoint& p0,
                                           const SkPoint& p1) {
  SkRect bounds = SkRect::MakeLTRB(p0.fX, p0.fY, p1.fX, p1.fY).makeSorted();
  accumulateRect(bounds, true);
}
void DisplayListBoundsCalculator::drawRect(const SkRect& rect) {
  accumulateRect(rect);
}
void DisplayListBoundsCalculator::drawOval(const SkRect& bounds) {
  accumulateRect(bounds);
}
void DisplayListBoundsCalculator::drawCircle(const SkPoint& center,
                                             SkScalar radius) {
  accumulateRect(SkRect::MakeLTRB(center.fX - radius, center.fY - radius,
                                  center.fX + radius, center.fY + radius));
}
void DisplayListBoundsCalculator::drawRRect(const SkRRect& rrect) {
  accumulateRect(rrect.getBounds());
}
void DisplayListBoundsCalculator::drawDRRect(const SkRRect& outer,
                                             const SkRRect& inner) {
  accumulateRect(outer.getBounds());
}
void DisplayListBoundsCalculator::drawPath(const SkPath& path) {
  accumulateRect(path.getBounds());
}
void DisplayListBoundsCalculator::drawArc(const SkRect& bounds,
                                          SkScalar start,
                                          SkScalar sweep,
                                          bool useCenter) {
  // This could be tighter if we compute where the start and end
  // angles are and then also consider the quadrants swept and
  // the center if specified.
  accumulateRect(bounds);
}
void DisplayListBoundsCalculator::drawPoints(SkCanvas::PointMode mode,
                                             uint32_t count,
                                             const SkPoint pts[]) {
  if (count > 0) {
    BoundsAccumulator ptBounds;
    for (size_t i = 0; i < count; i++) {
      ptBounds.accumulate(pts[i]);
    }
    accumulateRect(ptBounds.getBounds(), true);
  }
}
void DisplayListBoundsCalculator::drawVertices(const sk_sp<SkVertices> vertices,
                                               SkBlendMode mode) {
  accumulateRect(vertices->bounds());
}
void DisplayListBoundsCalculator::drawImage(const sk_sp<SkImage> image,
                                            const SkPoint point,
                                            const SkSamplingOptions& sampling) {
  SkRect bounds = SkRect::Make(image->bounds());
  bounds.offset(point);
  accumulateRect(bounds);
}
void DisplayListBoundsCalculator::drawImageRect(
    const sk_sp<SkImage> image,
    const SkRect& src,
    const SkRect& dst,
    const SkSamplingOptions& sampling) {
  accumulateRect(dst);
}
void DisplayListBoundsCalculator::drawImageNine(const sk_sp<SkImage> image,
                                                const SkIRect& center,
                                                const SkRect& dst,
                                                SkFilterMode filter) {
  accumulateRect(dst);
}
void DisplayListBoundsCalculator::drawImageLattice(
    const sk_sp<SkImage> image,
    const SkCanvas::Lattice& lattice,
    const SkRect& dst,
    SkFilterMode filter) {
  accumulateRect(dst);
}
void DisplayListBoundsCalculator::drawAtlas(const sk_sp<SkImage> atlas,
                                            const SkRSXform xform[],
                                            const SkRect tex[],
                                            const SkColor colors[],
                                            int count,
                                            SkBlendMode mode,
                                            const SkSamplingOptions& sampling,
                                            const SkRect* cullRect) {
  SkPoint quad[4];
  BoundsAccumulator atlasBounds;
  for (int i = 0; i < count; i++) {
    const SkRect& src = tex[i];
    xform[i].toQuad(src.width(), src.height(), quad);
    for (int j = 0; j < 4; j++) {
      atlasBounds.accumulate(quad[j]);
    }
  }
  if (atlasBounds.isNotEmpty()) {
    accumulateRect(atlasBounds.getBounds());
  }
}
void DisplayListBoundsCalculator::drawPicture(const sk_sp<SkPicture> picture) {
  // TODO(flar) cull rect really cannot be trusted in general, but
  // it will work for SkPictures generated from our own PictureRecorder.
  accumulateRect(picture->cullRect());
}
void DisplayListBoundsCalculator::drawDisplayList(
    const sk_sp<DisplayList> display_list) {
  accumulateRect(display_list->bounds());
}
void DisplayListBoundsCalculator::drawTextBlob(const sk_sp<SkTextBlob> blob,
                                               SkScalar x,
                                               SkScalar y) {
  accumulateRect(blob->bounds().makeOffset(x, y));
}
// void DisplayListBoundsCalculator::drawShadowRec(const SkPath& path,
//                                                 const SkDrawShadowRec& rec) {
//   SkRect bounds;
//   SkDrawShadowMetrics::GetLocalBounds(path, rec, SkMatrix::I(), &bounds);
//   accumulateRect(bounds, NON_GEOM);
// }
void DisplayListBoundsCalculator::drawShadow(const SkPath& path,
                                             const SkColor color,
                                             const SkScalar elevation,
                                             bool occludes) {
  SkRect bounds =
      PhysicalShapeLayer::ComputeShadowBounds(path.getBounds(), elevation, 1.0);
  accumulateRect(bounds);
}

void DisplayListBoundsCalculator::accumulateRect(const SkRect& rect,
                                                 bool forceStroke) {
  SkRect dstRect = rect;
  const SkPaint& p = paint();
  if (forceStroke) {
    if (p.getStyle() == SkPaint::kFill_Style) {
      setDrawStyle(SkPaint::kStroke_Style);
    } else {
      forceStroke = false;
    }
  }
  if (p.canComputeFastBounds()) {
    dstRect = p.computeFastBounds(rect, &dstRect);
    matrix().mapRect(&dstRect);
    accumulator_->accumulate(dstRect);
  } else {
    baseAccumulator_.accumulate(boundsCull_);
  }
  if (forceStroke) {
    setDrawStyle(SkPaint::kFill_Style);
  }
}

DisplayListBoundsCalculator::SaveInfo::SaveInfo(BoundsAccumulator* accumulator)
    : savedAccumulator_(accumulator) {}
BoundsAccumulator* DisplayListBoundsCalculator::SaveInfo::save() {
  // No need to swap out the accumulator for a normal save
  return savedAccumulator_;
}
BoundsAccumulator* DisplayListBoundsCalculator::SaveInfo::restore() {
  return savedAccumulator_;
}

DisplayListBoundsCalculator::SaveLayerInfo::SaveLayerInfo(
    BoundsAccumulator* accumulator,
    const SkMatrix& matrix)
    : SaveInfo(accumulator), matrix(matrix) {}
BoundsAccumulator* DisplayListBoundsCalculator::SaveLayerInfo::save() {
  // Use the local layerAccumulator until restore is called and
  // then transform (and adjust with paint if necessary) on restore()
  return &layerAccumulator_;
}
BoundsAccumulator* DisplayListBoundsCalculator::SaveLayerInfo::restore() {
  SkRect layerBounds = layerAccumulator_.getBounds();
  matrix.mapRect(&layerBounds);
  savedAccumulator_->accumulate(layerBounds);
  return savedAccumulator_;
}

DisplayListBoundsCalculator::SaveLayerWithPaintInfo::SaveLayerWithPaintInfo(
    DisplayListBoundsCalculator* calculator,
    BoundsAccumulator* accumulator,
    const SkMatrix& saveMatrix,
    const SkPaint& savePaint)
    : SaveLayerInfo(accumulator, saveMatrix),
      calculator_(calculator),
      paint_(savePaint) {}

BoundsAccumulator*
DisplayListBoundsCalculator::SaveLayerWithPaintInfo::restore() {
  SkRect layerBounds = layerAccumulator_.getBounds();
  if (paint_.canComputeFastBounds()) {
    layerBounds = paint_.computeFastBounds(layerBounds, &layerBounds);
    matrix.mapRect(&layerBounds);
    savedAccumulator_->accumulate(layerBounds);
  } else {
    calculator_->baseAccumulator_.accumulate(calculator_->boundsCull_);
  }
  return savedAccumulator_;
}

}  // namespace flutter

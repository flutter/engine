// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_DISPLAY_LIST_SKIA_DL_SK_CONVERSIONS_H_
#define FLUTTER_DISPLAY_LIST_SKIA_DL_SK_CONVERSIONS_H_

#include "flutter/display_list/dl_op_receiver.h"
#include "flutter/display_list/skia/dl_sk_types.h"

namespace flutter {

inline SkBlendMode ToSk(DlBlendMode mode) {
  return static_cast<SkBlendMode>(mode);
}

inline SkPaint::Style ToSk(DlDrawStyle style) {
  return static_cast<SkPaint::Style>(style);
}

inline SkPaint::Cap ToSk(DlStrokeCap cap) {
  return static_cast<SkPaint::Cap>(cap);
}

inline SkPaint::Join ToSk(DlStrokeJoin join) {
  return static_cast<SkPaint::Join>(join);
}

inline SkTileMode ToSk(DlTileMode dl_mode) {
  return static_cast<SkTileMode>(dl_mode);
}

inline SkBlurStyle ToSk(const DlBlurStyle blur_style) {
  return static_cast<SkBlurStyle>(blur_style);
}

inline SkFilterMode ToSk(const DlFilterMode filter_mode) {
  return static_cast<SkFilterMode>(filter_mode);
}

inline SkVertices::VertexMode ToSk(DlVertexMode dl_mode) {
  return static_cast<SkVertices::VertexMode>(dl_mode);
}

inline SkSamplingOptions ToSk(DlImageSampling sampling) {
  switch (sampling) {
    case DlImageSampling::kCubic:
      return SkSamplingOptions(SkCubicResampler{1 / 3.0f, 1 / 3.0f});
    case DlImageSampling::kLinear:
      return SkSamplingOptions(SkFilterMode::kLinear);
    case DlImageSampling::kMipmapLinear:
      return SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kLinear);
    case DlImageSampling::kNearestNeighbor:
      return SkSamplingOptions(SkFilterMode::kNearest);
  }
}

inline SkCanvas::SrcRectConstraint ToSk(
    DlCanvas::SrcRectConstraint constraint) {
  return static_cast<SkCanvas::SrcRectConstraint>(constraint);
}

inline SkClipOp ToSk(DlCanvas::ClipOp op) {
  return static_cast<SkClipOp>(op);
}

inline SkCanvas::PointMode ToSk(DlCanvas::PointMode mode) {
  return static_cast<SkCanvas::PointMode>(mode);
}

inline SkPoint ToSk(const DlFPoint& dl_point) {
  return SkPoint::Make(dl_point.x(), dl_point.y());
}

inline const SkPoint* ToSk(const DlFPoint* dl_points) {
  return reinterpret_cast<const SkPoint*>(dl_points);
}

inline DlISize ToDl(const SkISize& sk_size) {
  return sk_size.isEmpty() ? DlISize()
                           : DlISize(sk_size.fWidth, sk_size.fHeight);
}

inline SkIRect ToSk(const DlIRect& dl_rect) {
  return SkIRect::MakeLTRB(dl_rect.left(), dl_rect.top(),  //
                           dl_rect.right(), dl_rect.bottom());
}

inline SkRect ToSk(const DlFRect& dl_rect) {
  return SkRect::MakeLTRB(dl_rect.left(), dl_rect.top(),  //
                          dl_rect.right(), dl_rect.bottom());
}

inline const SkRect* ToSk(SkRect* scratch, const DlFRect* dl_rect) {
  if (dl_rect == nullptr) {
    return nullptr;
  }
  scratch->setLTRB(dl_rect->left(), dl_rect->top(),  //
                   dl_rect->right(), dl_rect->bottom());
  return scratch;
}

inline DlFRect ToDl(const SkRect& sk_rect) {
  return DlFRect::MakeLTRB(sk_rect.fLeft, sk_rect.fTop,  //
                           sk_rect.fRight, sk_rect.fBottom);
}

inline const SkRect* ToSk(const DlFRect* dl_rects) {
  return reinterpret_cast<const SkRect*>(dl_rects);
}

inline SkRRect ToSk(const DlFRRect& dl_rrect) {
  SkRRect sk_rrect;
  DlFPoint dl_radii[4];
  dl_rrect.GetRadii(dl_radii);
  sk_rrect.setRectRadii(ToSk(dl_rrect.Bounds()), ToSk(dl_radii));
  return sk_rrect;
}

inline SkRSXform ToSk(const DlRSTransform& dl_rs) {
  return SkRSXform::Make(dl_rs.scaled_cos(), dl_rs.scaled_sin(),
                         dl_rs.translate_x(), dl_rs.translate_y());
}

inline const SkRSXform* ToSk(const DlRSTransform* dl_rs) {
  return reinterpret_cast<const SkRSXform*>(dl_rs);
}

inline SkMatrix ToSk(const DlTransform& dl_transform) {
  return dl_transform.ToSkMatrix();
}

inline SkMatrix* ToSk(SkMatrix* scratch, const DlTransform* dl_transform) {
  if (dl_transform == nullptr) {
    return nullptr;
  }
  *scratch = dl_transform->ToSkMatrix();
  return scratch;
}

inline DlTransform ToDl(SkM44 transform) {
  SkScalar matrix[16];
  transform.getColMajor(matrix);
  return DlTransform::MakeColMajor(matrix);
}

extern sk_sp<SkShader> ToSk(const DlColorSource* source);

extern sk_sp<SkImageFilter> ToSk(const DlImageFilter* filter);

extern sk_sp<SkColorFilter> ToSk(const DlColorFilter* filter);

extern sk_sp<SkMaskFilter> ToSk(const DlMaskFilter* filter);

extern sk_sp<SkPathEffect> ToSk(const DlPathEffect* effect);

extern sk_sp<SkVertices> ToSk(const DlVertices* vertices);

}  // namespace flutter

#endif  // FLUTTER_DISPLAY_LIST_SKIA_DL_SK_CONVERSIONS_H_

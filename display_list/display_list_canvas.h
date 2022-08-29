// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_DISPLAY_LIST_DISPLAY_LIST_CANVAS_H_
#define FLUTTER_DISPLAY_LIST_DISPLAY_LIST_CANVAS_H_

#include "flutter/display_list/display_list_blend_mode.h"
#include "flutter/display_list/display_list_image.h"
#include "flutter/display_list/display_list_paint.h"
#include "flutter/display_list/display_list_sampling_options.h"
#include "flutter/display_list/display_list_vertices.h"

namespace flutter {

// The convenience API based on DisplayList objects that defines how
// to talk to either an SkCanvas adapter or a DisplayListBuilder in
// a loss-less fashion.
class DlCanvas {
 public:
  virtual void save() = 0;
  virtual void saveLayer(const SkRect* bounds,
                         const DlPaint* paint,
                         const DlImageFilter* backdrop = nullptr) = 0;
  virtual void restore() = 0;
  virtual int getSaveCount() = 0;
  virtual void restoreToCount(int restore_count) = 0;

  virtual void translate(SkScalar tx, SkScalar ty) = 0;
  virtual void scale(SkScalar sx, SkScalar sy) = 0;
  virtual void rotate(SkScalar degrees) = 0;
  virtual void skew(SkScalar sx, SkScalar sy) = 0;

  // clang-format off

  // 2x3 2D affine subset of a 4x4 transform in row major order
  virtual void transform2DAffine(SkScalar mxx, SkScalar mxy, SkScalar mxt,
                                 SkScalar myx, SkScalar myy, SkScalar myt) = 0;
  // full 4x4 transform in row major order
  virtual void transformFullPerspective(
      SkScalar mxx, SkScalar mxy, SkScalar mxz, SkScalar mxt,
      SkScalar myx, SkScalar myy, SkScalar myz, SkScalar myt,
      SkScalar mzx, SkScalar mzy, SkScalar mzz, SkScalar mzt,
      SkScalar mwx, SkScalar mwy, SkScalar mwz, SkScalar mwt) = 0;
  // clang-format on
  virtual void transformReset() = 0;
  virtual void transform(const SkMatrix* matrix) = 0;
  virtual void transform(const SkM44* matrix44) = 0;
  void transform(const SkMatrix& matrix) { transform(&matrix); }
  void transform(const SkM44& matrix44) { transform(&matrix44); }

  /// Returns the 4x4 full perspective transform representing all transform
  /// operations executed so far in this DisplayList within the enclosing
  /// save stack.
  virtual SkM44 getTransformFullPerspective() const = 0;
  /// Returns the 3x3 partial perspective transform representing all transform
  /// operations executed so far in this DisplayList within the enclosing
  /// save stack.
  virtual SkMatrix getTransform() const = 0;

  virtual void clipRect(const SkRect& rect, SkClipOp clip_op, bool is_aa) = 0;
  virtual void clipRRect(const SkRRect& rrect,
                         SkClipOp clip_op,
                         bool is_aa) = 0;
  virtual void clipPath(const SkPath& path, SkClipOp clip_op, bool is_aa) = 0;

  /// Conservative estimate of the bounds of all outstanding clip operations
  /// measured in the coordinate space within which this DisplayList will
  /// be rendered.
  virtual SkRect getDestinationClipBounds() const = 0;
  /// Conservative estimate of the bounds of all outstanding clip operations
  /// transformed into the local coordinate space in which currently
  /// recorded rendering operations are interpreted.
  virtual SkRect getLocalClipBounds() const = 0;

  virtual void drawPaint(const DlPaint& paint) = 0;
  virtual void drawColor(DlColor color, DlBlendMode mode) = 0;
  virtual void drawLine(const SkPoint& p0,
                        const SkPoint& p1,
                        const DlPaint& paint) = 0;
  virtual void drawRect(const SkRect& rect, const DlPaint& paint) = 0;
  virtual void drawOval(const SkRect& bounds, const DlPaint& paint) = 0;
  virtual void drawCircle(const SkPoint& center,
                          SkScalar radius,
                          const DlPaint& paint) = 0;
  virtual void drawRRect(const SkRRect& rrect, const DlPaint& paint) = 0;
  virtual void drawDRRect(const SkRRect& outer,
                          const SkRRect& inner,
                          const DlPaint& paint) = 0;
  virtual void drawPath(const SkPath& path, const DlPaint& paint) = 0;
  virtual void drawArc(const SkRect& bounds,
                       SkScalar start,
                       SkScalar sweep,
                       bool useCenter,
                       const DlPaint& paint) = 0;
  virtual void drawPoints(SkCanvas::PointMode mode,
                          uint32_t count,
                          const SkPoint pts[],
                          const DlPaint& paint) = 0;
  virtual void drawVertices(const DlVertices* vertices,
                            DlBlendMode mode,
                            const DlPaint& paint) = 0;
  void drawVertices(const std::shared_ptr<const DlVertices> vertices,
                    DlBlendMode mode,
                    const DlPaint& paint) {
    drawVertices(vertices.get(), mode, paint);
  }
  virtual void drawImage(const sk_sp<DlImage> image,
                         const SkPoint point,
                         DlImageSampling sampling,
                         const DlPaint* paint = nullptr) = 0;
  virtual void drawImageRect(
      const sk_sp<DlImage> image,
      const SkRect& src,
      const SkRect& dst,
      DlImageSampling sampling,
      const DlPaint* paint = nullptr,
      SkCanvas::SrcRectConstraint constraint =
          SkCanvas::SrcRectConstraint::kFast_SrcRectConstraint) = 0;
  virtual void drawImageNine(const sk_sp<DlImage> image,
                             const SkIRect& center,
                             const SkRect& dst,
                             DlFilterMode filter,
                             const DlPaint* paint = nullptr) = 0;
  virtual void drawAtlas(const sk_sp<DlImage> atlas,
                         const SkRSXform xform[],
                         const SkRect tex[],
                         const DlColor colors[],
                         int count,
                         DlBlendMode mode,
                         DlImageSampling sampling,
                         const SkRect* cullRect,
                         const DlPaint* paint = nullptr) = 0;
  virtual void drawDisplayList(const sk_sp<DisplayList> display_list) = 0;
  virtual void drawTextBlob(const sk_sp<SkTextBlob> blob,
                            SkScalar x,
                            SkScalar y,
                            const DlPaint& paint) = 0;
  virtual void drawShadow(const SkPath& path,
                          const DlColor color,
                          const SkScalar elevation,
                          bool transparent_occluder,
                          SkScalar dpr) = 0;
};

}  // namespace flutter

#endif  // FLUTTER_DISPLAY_LIST_DISPLAY_LIST_CANVAS_H_

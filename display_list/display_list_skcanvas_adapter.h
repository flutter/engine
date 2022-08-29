// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_DISPLAY_LIST_DISPLAY_LIST_SKCANVAS_ADAPTER_H_
#define FLUTTER_DISPLAY_LIST_DISPLAY_LIST_SKCANVAS_ADAPTER_H_

#include "flutter/display_list/display_list_canvas.h"

namespace flutter {

// This adapter object implements the developer-facing DisplayListCanvas
// interface but translates and directs the calls to an SkCanvas instance.
class DlSkCanvasAdapter : public virtual DlCanvas {
 public:
  DlSkCanvasAdapter(SkCanvas* delegate) : delegate_(delegate) {}

  void save() override;
  void restore() override;
  int getSaveCount() override;
  void restoreToCount(int restore_count) override;

  void translate(SkScalar tx, SkScalar ty) override;
  void scale(SkScalar sx, SkScalar sy) override;
  void rotate(SkScalar degrees) override;
  void skew(SkScalar sx, SkScalar sy) override;

  // clang-format off

  // 2x3 2D affine subset of a 4x4 transform in row major order
  void transform2DAffine(SkScalar mxx, SkScalar mxy, SkScalar mxt,
                         SkScalar myx, SkScalar myy, SkScalar myt) override;
  // full 4x4 transform in row major order
  void transformFullPerspective(
      SkScalar mxx, SkScalar mxy, SkScalar mxz, SkScalar mxt,
      SkScalar myx, SkScalar myy, SkScalar myz, SkScalar myt,
      SkScalar mzx, SkScalar mzy, SkScalar mzz, SkScalar mzt,
      SkScalar mwx, SkScalar mwy, SkScalar mwz, SkScalar mwt) override;
  // clang-format on
  void transformReset() override;
  void transform(const SkMatrix* matrix) override;
  void transform(const SkM44* matrix44) override;

  /// Returns the 4x4 full perspective transform representing all transform
  /// operations executed so far in this DisplayList within the enclosing
  /// save stack.
  SkM44 getTransformFullPerspective() const override;
  /// Returns the 3x3 partial perspective transform representing all transform
  /// operations executed so far in this DisplayList within the enclosing
  /// save stack.
  SkMatrix getTransform() const override;

  void clipRect(const SkRect& rect, SkClipOp clip_op, bool is_aa) override;
  void clipRRect(const SkRRect& rrect, SkClipOp clip_op, bool is_aa) override;
  void clipPath(const SkPath& path, SkClipOp clip_op, bool is_aa) override;

  /// Conservative estimate of the bounds of all outstanding clip operations
  /// measured in the coordinate space within which this DisplayList will
  /// be rendered.
  SkRect getDestinationClipBounds() const override;
  /// Conservative estimate of the bounds of all outstanding clip operations
  /// transformed into the local coordinate space in which currently
  /// recorded rendering operations are interpreted.
  SkRect getLocalClipBounds() const override;

  void drawPaint(const DlPaint& paint) override;
  void drawColor(DlColor color, DlBlendMode mode) override;
  void drawLine(const SkPoint& p0,
                const SkPoint& p1,
                const DlPaint& paint) override;
  void drawRect(const SkRect& rect, const DlPaint& paint) override;
  void drawOval(const SkRect& bounds, const DlPaint& paint) override;
  void drawCircle(const SkPoint& center,
                  SkScalar radius,
                  const DlPaint& paint) override;
  void drawRRect(const SkRRect& rrect, const DlPaint& paint) override;
  void drawDRRect(const SkRRect& outer,
                  const SkRRect& inner,
                  const DlPaint& paint) override;
  void drawPath(const SkPath& path, const DlPaint& paint) override;
  void drawArc(const SkRect& bounds,
               SkScalar start,
               SkScalar sweep,
               bool useCenter,
               const DlPaint& paint) override;
  void drawPoints(SkCanvas::PointMode mode,
                  uint32_t count,
                  const SkPoint pts[],
                  const DlPaint& paint) override;
  void drawVertices(const DlVertices* vertices,
                    DlBlendMode mode,
                    const DlPaint& paint) override;
  void drawDisplayList(const sk_sp<DisplayList> display_list) override;
  void drawTextBlob(const sk_sp<SkTextBlob> blob,
                    SkScalar x,
                    SkScalar y,
                    const DlPaint& paint) override;
  void drawShadow(const SkPath& path,
                  const DlColor color,
                  const SkScalar elevation,
                  bool transparent_occluder,
                  SkScalar dpr) override;

  // These using calls draw in the overloads in DlCanvas that implement
  // default parameters.
  using DlCanvas::drawAtlas;
  using DlCanvas::drawImage;
  using DlCanvas::drawImageNine;
  using DlCanvas::drawImageRect;
  using DlCanvas::drawVertices;
  using DlCanvas::saveLayer;
  using DlCanvas::transform;

 protected:
  // The "on" versios of these calls are used to avoid conflicts with
  // the methods in DlCanvas that have default parametrs.
  void onSaveLayer(const SkRect* bounds,
                   const DlPaint* paint,
                   const DlImageFilter* backdrop) override;
  void onDrawImage(const sk_sp<DlImage> image,
                   const SkPoint point,
                   DlImageSampling sampling,
                   const DlPaint* paint) override;
  void onDrawImageRect(const sk_sp<DlImage> image,
                       const SkRect& src,
                       const SkRect& dst,
                       DlImageSampling sampling,
                       const DlPaint* paint,
                       SkCanvas::SrcRectConstraint constraint) override;
  void onDrawImageNine(const sk_sp<DlImage> image,
                       const SkIRect& center,
                       const SkRect& dst,
                       DlFilterMode filter,
                       const DlPaint* paint) override;
  void onDrawAtlas(const sk_sp<DlImage> atlas,
                   const SkRSXform xform[],
                   const SkRect tex[],
                   const DlColor colors[],
                   int count,
                   DlBlendMode mode,
                   DlImageSampling sampling,
                   const SkRect* cullRect,
                   const DlPaint* paint) override;

 private:
  SkCanvas* delegate_;
};

}  // namespace flutter

#endif  // FLUTTER_DISPLAY_LIST_DISPLAY_LIST_SKCANVAS_ADAPTER_H_

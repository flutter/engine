// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_DISPLAY_LIST_SKIA_DL_SK_CANVAS_H_
#define FLUTTER_DISPLAY_LIST_SKIA_DL_SK_CANVAS_H_

#include "flutter/display_list/dl_canvas.h"
#include "third_party/skia/include/core/SkCanvas.h"

namespace flutter {

// An adapter to receive DlCanvas calls and dispatch them into
// an SkCanvas.
class DlSkCanvasAdapter final : public virtual DlCanvas {
 public:
  DlSkCanvasAdapter() : delegate_(nullptr) {}
  explicit DlSkCanvasAdapter(SkCanvas* canvas) : delegate_(canvas) {}
  ~DlSkCanvasAdapter() override = default;

  void set_canvas(SkCanvas* canvas);
  SkCanvas* canvas() { return delegate_; }

  DlISize GetBaseLayerSize() const override;
  SkImageInfo GetImageInfo() const override;

  void Save() override;
  void SaveLayer(const DlFRect* bounds,
                 const DlPaint* paint = nullptr,
                 const DlImageFilter* backdrop = nullptr) override;
  void Restore() override;
  int GetSaveCount() const override;
  void RestoreToCount(int restore_count) override;

  void Translate(DlScalar tx, DlScalar ty) override;
  void Scale(DlScalar sx, DlScalar sy) override;
  void Rotate(DlAngle angle) override;
  void Skew(DlScalar sx, DlScalar sy) override;

  // clang-format off

  // 2x3 2D affine subset of a 4x4 transform in row major order
  void Transform2DAffine(DlScalar mxx, DlScalar mxy, DlScalar mxt,
                         DlScalar myx, DlScalar myy, DlScalar myt) override;
  // full 4x4 transform in row major order
  void TransformFullPerspective(
      DlScalar mxx, DlScalar mxy, DlScalar mxz, DlScalar mxt,
      DlScalar myx, DlScalar myy, DlScalar myz, DlScalar myt,
      DlScalar mzx, DlScalar mzy, DlScalar mzz, DlScalar mzt,
      DlScalar mwx, DlScalar mwy, DlScalar mwz, DlScalar mwt) override;
  // clang-format on
  void TransformReset() override;
  void Transform(const DlTransform& matrix) override;
  void SetTransform(const DlTransform& matrix) override;
  using DlCanvas::SetTransform;
  using DlCanvas::Transform;

  /// Returns the 3x3 partial perspective transform representing all transform
  /// operations executed so far in this DisplayList within the enclosing
  /// save stack.
  DlTransform GetTransform() const override;

  void ClipRect(const DlFRect& rect, ClipOp clip_op, bool is_aa) override;
  void ClipRRect(const DlFRRect& rrect, ClipOp clip_op, bool is_aa) override;
  void ClipPath(const DlPath& path, ClipOp clip_op, bool is_aa) override;

  /// Conservative estimate of the bounds of all outstanding clip operations
  /// measured in the coordinate space within which this DisplayList will
  /// be rendered.
  DlFRect GetDestinationClipBounds() const override;
  /// Conservative estimate of the bounds of all outstanding clip operations
  /// transformed into the local coordinate space in which currently
  /// recorded rendering operations are interpreted.
  DlFRect GetLocalClipBounds() const override;

  /// Return true iff the supplied bounds are easily shown to be outside
  /// of the current clip bounds. This method may conservatively return
  /// false if it cannot make the determination.
  bool QuickReject(const DlFRect& bounds) const override;

  void DrawPaint(const DlPaint& paint) override;
  void DrawColor(DlColor color, DlBlendMode mode) override;
  void DrawLine(const DlFPoint& p0,
                const DlFPoint& p1,
                const DlPaint& paint) override;
  void DrawRect(const DlFRect& rect, const DlPaint& paint) override;
  void DrawOval(const DlFRect& bounds, const DlPaint& paint) override;
  void DrawCircle(const DlFPoint& center,
                  DlScalar radius,
                  const DlPaint& paint) override;
  void DrawRRect(const DlFRRect& rrect, const DlPaint& paint) override;
  void DrawDRRect(const DlFRRect& outer,
                  const DlFRRect& inner,
                  const DlPaint& paint) override;
  void DrawPath(const DlPath& path, const DlPaint& paint) override;
  void DrawArc(const DlFRect& bounds,
               DlScalar start,
               DlScalar sweep,
               bool useCenter,
               const DlPaint& paint) override;
  void DrawPoints(PointMode mode,
                  uint32_t count,
                  const DlFPoint pts[],
                  const DlPaint& paint) override;
  void DrawVertices(const DlVertices* vertices,
                    DlBlendMode mode,
                    const DlPaint& paint) override;
  void DrawImage(const sk_sp<DlImage>& image,
                 const DlFPoint point,
                 DlImageSampling sampling,
                 const DlPaint* paint = nullptr) override;
  void DrawImageRect(
      const sk_sp<DlImage>& image,
      const DlFRect& src,
      const DlFRect& dst,
      DlImageSampling sampling,
      const DlPaint* paint = nullptr,
      SrcRectConstraint constraint = SrcRectConstraint::kFast) override;
  void DrawImageNine(const sk_sp<DlImage>& image,
                     const DlIRect& center,
                     const DlFRect& dst,
                     DlFilterMode filter,
                     const DlPaint* paint = nullptr) override;
  void DrawAtlas(const sk_sp<DlImage>& atlas,
                 const DlRSTransform xform[],
                 const DlFRect tex[],
                 const DlColor colors[],
                 int count,
                 DlBlendMode mode,
                 DlImageSampling sampling,
                 const DlFRect* cullRect,
                 const DlPaint* paint = nullptr) override;
  void DrawDisplayList(const sk_sp<DisplayList> display_list,
                       DlScalar opacity = SK_Scalar1) override;
  void DrawTextBlob(const sk_sp<SkTextBlob>& blob,
                    DlScalar x,
                    DlScalar y,
                    const DlPaint& paint) override;
  void DrawShadow(const DlPath& path,
                  const DlColor color,
                  const DlScalar elevation,
                  bool transparent_occluder,
                  DlScalar dpr) override;

  void Flush() override;

 private:
  SkCanvas* delegate_;
};

}  // namespace flutter

#endif  // FLUTTER_DISPLAY_LIST_SKIA_DL_SK_CANVAS_H_

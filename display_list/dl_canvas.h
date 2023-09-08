// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_DISPLAY_LIST_DL_CANVAS_H_
#define FLUTTER_DISPLAY_LIST_DL_CANVAS_H_

#include "flutter/display_list/dl_blend_mode.h"
#include "flutter/display_list/dl_paint.h"
#include "flutter/display_list/dl_vertices.h"
#include "flutter/display_list/geometry/dl_path.h"
#include "flutter/display_list/geometry/dl_point.h"
#include "flutter/display_list/geometry/dl_rect.h"
#include "flutter/display_list/geometry/dl_round_rect.h"
#include "flutter/display_list/geometry/dl_rstransform.h"
#include "flutter/display_list/geometry/dl_transform.h"
#include "flutter/display_list/image/dl_image.h"

#include "third_party/skia/include/core/SkTextBlob.h"

namespace flutter {

// The primary class used to express rendering operations in the
// DisplayList ecosystem. This class is an API-only virtual class and
// can be used to talk to a DisplayListBuilder to record a series of
// rendering operations, or it could be the public facing API of an
// adapter that forwards the calls to another rendering module, like
// Skia.
//
// Developers familiar with Skia's SkCanvas API will be immediately
// familiar with the methods below as they follow that API closely
// but with DisplayList objects and values used as data instead.
class DlCanvas {
 public:
  enum class ClipOp {
    kDifference,
    kIntersect,
  };

  enum class PointMode {
    kPoints,   //!< draw each point separately
    kLines,    //!< draw each separate pair of points as a line segment
    kPolygon,  //!< draw each pair of overlapping points as a line segment
  };

  enum class SrcRectConstraint {
    kStrict,
    kFast,
  };

  virtual ~DlCanvas() = default;

  virtual DlISize GetBaseLayerSize() const = 0;
  virtual SkImageInfo GetImageInfo() const = 0;

  virtual void Save() = 0;
  virtual void SaveLayer(const DlFRect* bounds,
                         const DlPaint* paint = nullptr,
                         const DlImageFilter* backdrop = nullptr) = 0;
  virtual void Restore() = 0;
  virtual int GetSaveCount() const = 0;
  virtual void RestoreToCount(int restore_count) = 0;

  virtual void Translate(DlScalar tx, DlScalar ty) = 0;
  void Translate(DlFPoint p) { Translate(p.x(), p.y()); }
  virtual void Scale(DlScalar sx, DlScalar sy) = 0;
  virtual void Rotate(DlAngle angle) = 0;
  virtual void Skew(DlScalar sx, DlScalar sy) = 0;

  // clang-format off

  // 2x3 2D affine subset of a 4x4 transform in row major order
  virtual void Transform2DAffine(DlScalar mxx, DlScalar mxy, DlScalar mxt,
                                 DlScalar myx, DlScalar myy, DlScalar myt) = 0;
  // full 4x4 transform in row major order
  virtual void TransformFullPerspective(
      DlScalar mxx, DlScalar mxy, DlScalar mxz, DlScalar mxt,
      DlScalar myx, DlScalar myy, DlScalar myz, DlScalar myt,
      DlScalar mzx, DlScalar mzy, DlScalar mzz, DlScalar mzt,
      DlScalar mwx, DlScalar mwy, DlScalar mwz, DlScalar mwt) = 0;
  // clang-format on
  virtual void TransformReset() = 0;
  virtual void Transform(const DlTransform& matrix) = 0;
  virtual void SetTransform(const DlTransform& matrix) = 0;

  /// Returns the 3x3 partial perspective transform representing all transform
  /// operations executed so far in this DisplayList within the enclosing
  /// save stack.
  virtual DlTransform GetTransform() const = 0;

  virtual void ClipRect(const DlFRect& rect,
                        ClipOp clip_op = ClipOp::kIntersect,
                        bool is_aa = false) = 0;
  virtual void ClipRRect(const DlFRRect& rrect,
                         ClipOp clip_op = ClipOp::kIntersect,
                         bool is_aa = false) = 0;
  virtual void ClipPath(const DlPath& path,
                        ClipOp clip_op = ClipOp::kIntersect,
                        bool is_aa = false) = 0;

  /// Conservative estimate of the bounds of all outstanding clip operations
  /// measured in the coordinate space within which this DisplayList will
  /// be rendered.
  virtual DlFRect GetDestinationClipBounds() const = 0;
  /// Conservative estimate of the bounds of all outstanding clip operations
  /// transformed into the local coordinate space in which currently
  /// recorded rendering operations are interpreted.
  virtual DlFRect GetLocalClipBounds() const = 0;

  /// Return true iff the supplied bounds are easily shown to be outside
  /// of the current clip bounds. This method may conservatively return
  /// false if it cannot make the determination.
  virtual bool QuickReject(const DlFRect& bounds) const = 0;

  virtual void DrawPaint(const DlPaint& paint) = 0;
  virtual void DrawColor(DlColor color,
                         DlBlendMode mode = DlBlendMode::kSrcOver) = 0;
  void Clear(DlColor color) { DrawColor(color, DlBlendMode::kSrc); }
  virtual void DrawLine(const DlFPoint& p0,
                        const DlFPoint& p1,
                        const DlPaint& paint) = 0;
  virtual void DrawRect(const DlFRect& rect, const DlPaint& paint) = 0;
  virtual void DrawOval(const DlFRect& bounds, const DlPaint& paint) = 0;
  virtual void DrawCircle(const DlFPoint& center,
                          DlScalar radius,
                          const DlPaint& paint) = 0;
  virtual void DrawRRect(const DlFRRect& rrect, const DlPaint& paint) = 0;
  virtual void DrawDRRect(const DlFRRect& outer,
                          const DlFRRect& inner,
                          const DlPaint& paint) = 0;
  virtual void DrawPath(const DlPath& path, const DlPaint& paint) = 0;
  virtual void DrawArc(const DlFRect& bounds,
                       DlScalar start,
                       DlScalar sweep,
                       bool useCenter,
                       const DlPaint& paint) = 0;
  virtual void DrawPoints(PointMode mode,
                          uint32_t count,
                          const DlFPoint pts[],
                          const DlPaint& paint) = 0;
  virtual void DrawVertices(const DlVertices* vertices,
                            DlBlendMode mode,
                            const DlPaint& paint) = 0;
  void DrawVertices(const std::shared_ptr<const DlVertices> vertices,
                    DlBlendMode mode,
                    const DlPaint& paint) {
    DrawVertices(vertices.get(), mode, paint);
  }
  virtual void DrawImage(const sk_sp<DlImage>& image,
                         const DlFPoint point,
                         DlImageSampling sampling,
                         const DlPaint* paint = nullptr) = 0;
  virtual void DrawImageRect(
      const sk_sp<DlImage>& image,
      const DlFRect& src,
      const DlFRect& dst,
      DlImageSampling sampling,
      const DlPaint* paint = nullptr,
      SrcRectConstraint constraint = SrcRectConstraint::kFast) = 0;
  virtual void DrawImageRect(
      const sk_sp<DlImage>& image,
      const DlIRect& src,
      const DlFRect& dst,
      DlImageSampling sampling,
      const DlPaint* paint = nullptr,
      SrcRectConstraint constraint = SrcRectConstraint::kFast) {
    DrawImageRect(image, DlFRect::MakeBounds(src), dst, sampling, paint,
                  constraint);
  }
  virtual void DrawImageRect(
      const sk_sp<DlImage>& image,
      const DlFRect& dst,
      DlImageSampling sampling,
      const DlPaint* paint = nullptr,
      SrcRectConstraint constraint = SrcRectConstraint::kFast) {
    DrawImageRect(image, image->bounds(), dst, sampling, paint, constraint);
  }
  virtual void DrawImageNine(const sk_sp<DlImage>& image,
                             const DlIRect& center,
                             const DlFRect& dst,
                             DlFilterMode filter,
                             const DlPaint* paint = nullptr) = 0;
  virtual void DrawAtlas(const sk_sp<DlImage>& atlas,
                         const DlRSTransform xform[],
                         const DlFRect tex[],
                         const DlColor colors[],
                         int count,
                         DlBlendMode mode,
                         DlImageSampling sampling,
                         const DlFRect* cullRect,
                         const DlPaint* paint = nullptr) = 0;
  virtual void DrawDisplayList(const sk_sp<DisplayList> display_list,
                               DlScalar opacity = SK_Scalar1) = 0;
  virtual void DrawTextBlob(const sk_sp<SkTextBlob>& blob,
                            DlScalar x,
                            DlScalar y,
                            const DlPaint& paint) = 0;
  virtual void DrawShadow(const DlPath& path,
                          const DlColor color,
                          const DlScalar elevation,
                          bool transparent_occluder,
                          DlScalar dpr) = 0;

  virtual void Flush() = 0;

  static constexpr DlScalar kShadowLightHeight = 600;
  static constexpr DlScalar kShadowLightRadius = 800;

  static DlFRect ComputeShadowBounds(const DlPath& path,
                                     float elevation,
                                     DlScalar dpr,
                                     const DlTransform& ctm);
};

class DlAutoCanvasRestore {
 public:
  DlAutoCanvasRestore(DlCanvas* canvas, bool do_save) : canvas_(canvas) {
    if (canvas) {
      canvas_ = canvas;
      restore_count_ = canvas->GetSaveCount();
      if (do_save) {
        canvas_->Save();
      }
    } else {
      canvas_ = nullptr;
      restore_count_ = 0;
    }
  }

  ~DlAutoCanvasRestore() { Restore(); }

  void Restore() {
    if (canvas_) {
      canvas_->RestoreToCount(restore_count_);
      canvas_ = nullptr;
    }
  }

 private:
  DlCanvas* canvas_;
  int restore_count_;

  FML_DISALLOW_COPY_ASSIGN_AND_MOVE(DlAutoCanvasRestore);
};

}  // namespace flutter

#endif  // FLUTTER_DISPLAY_LIST_DL_CANVAS_H_

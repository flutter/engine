// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_DISPLAY_LIST_SKIA_DL_SK_CANVAS_DISPATCHER_H_
#define FLUTTER_DISPLAY_LIST_SKIA_DL_SK_CANVAS_DISPATCHER_H_

#include "flutter/display_list/display_list.h"
#include "flutter/display_list/dl_op_receiver.h"
#include "flutter/display_list/skia/dl_sk_paint_dispatcher.h"
#include "flutter/fml/macros.h"

#include "third_party/skia/include/core/SkCanvas.h"

namespace flutter {

//------------------------------------------------------------------------------
/// Can be fed to the dispatch() method of a DisplayList to feed the resulting
/// rendering operations to an SkCanvas instance.
///
/// Receives all methods on Dispatcher and sends them to an SkCanvas
///
class DlSkCanvasDispatcher : public virtual DlOpReceiver,
                             public DlSkPaintDispatchHelper {
 public:
  explicit DlSkCanvasDispatcher(SkCanvas* canvas, SkScalar opacity = SK_Scalar1)
      : DlSkPaintDispatchHelper(opacity),
        canvas_(canvas),
        original_transform_(canvas->getLocalToDevice()) {}

  const SkPaint* safe_paint(bool use_attributes);

  void save() override;
  void restore() override;
  void saveLayer(const DlFRect* bounds,
                 const SaveLayerOptions options,
                 const DlImageFilter* backdrop) override;

  void translate(DlScalar tx, DlScalar ty) override;
  void scale(DlScalar sx, DlScalar sy) override;
  void rotate(DlAngle angle) override;
  void skew(DlScalar sx, DlScalar sy) override;
  // clang-format off
  // 2x3 2D affine subset of a 4x4 transform in row major order
  void transform2DAffine(DlScalar mxx, DlScalar mxy, DlScalar mxt,
                         DlScalar myx, DlScalar myy, DlScalar myt) override;
  // full 4x4 transform in row major order
  void transformFullPerspective(
      DlScalar mxx, DlScalar mxy, DlScalar mxz, DlScalar mxt,
      DlScalar myx, DlScalar myy, DlScalar myz, DlScalar myt,
      DlScalar mzx, DlScalar mzy, DlScalar mzz, DlScalar mzt,
      DlScalar mwx, DlScalar mwy, DlScalar mwz, DlScalar mwt) override;
  // clang-format on
  void transformReset() override;

  void clipRect(const DlFRect& rect, ClipOp clip_op, bool is_aa) override;
  void clipRRect(const DlFRRect& rrect, ClipOp clip_op, bool is_aa) override;
  void clipPath(const DlPath& path, ClipOp clip_op, bool is_aa) override;

  void drawPaint() override;
  void drawColor(DlColor color, DlBlendMode mode) override;
  void drawLine(const DlFPoint& p0, const DlFPoint& p1) override;
  void drawRect(const DlFRect& rect) override;
  void drawOval(const DlFRect& bounds) override;
  void drawCircle(const DlFPoint& center, DlScalar radius) override;
  void drawRRect(const DlFRRect& rrect) override;
  void drawDRRect(const DlFRRect& outer, const DlFRRect& inner) override;
  void drawPath(const DlPath& path) override;
  void drawArc(const DlFRect& bounds,
               DlScalar start,
               DlScalar sweep,
               bool useCenter) override;
  void drawPoints(PointMode mode,
                  uint32_t count,
                  const DlFPoint pts[]) override;
  void drawVertices(const DlVertices* vertices, DlBlendMode mode) override;
  void drawImage(const sk_sp<DlImage> image,
                 const DlFPoint point,
                 DlImageSampling sampling,
                 bool render_with_attributes) override;
  void drawImageRect(const sk_sp<DlImage> image,
                     const DlFRect& src,
                     const DlFRect& dst,
                     DlImageSampling sampling,
                     bool render_with_attributes,
                     SrcRectConstraint constraint) override;
  void drawImageNine(const sk_sp<DlImage> image,
                     const DlIRect& center,
                     const DlFRect& dst,
                     DlFilterMode filter,
                     bool render_with_attributes) override;
  void drawAtlas(const sk_sp<DlImage> atlas,
                 const DlRSTransform xform[],
                 const DlFRect tex[],
                 const DlColor colors[],
                 int count,
                 DlBlendMode mode,
                 DlImageSampling sampling,
                 const DlFRect* cullRect,
                 bool render_with_attributes) override;
  void drawDisplayList(const sk_sp<DisplayList> display_list,
                       DlScalar opacity) override;
  void drawTextBlob(const sk_sp<SkTextBlob> blob,
                    DlScalar x,
                    DlScalar y) override;
  void drawShadow(const DlPath& path,
                  const DlColor color,
                  const DlScalar elevation,
                  bool transparent_occluder,
                  DlScalar dpr) override;

  static void DrawShadow(SkCanvas* canvas,
                         const DlPath& path,
                         DlColor color,
                         float elevation,
                         bool transparentOccluder,
                         DlScalar dpr);

 private:
  SkCanvas* canvas_;
  const SkM44 original_transform_;
  SkPaint temp_paint_;
};

}  // namespace flutter

#endif  // FLUTTER_DISPLAY_LIST_SKIA_DL_SK_CANVAS_DISPATCHER_H_

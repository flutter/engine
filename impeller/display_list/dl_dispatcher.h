// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "flutter/display_list/dl_op_receiver.h"
#include "flutter/display_list/geometry/dl_rect.h"
#include "flutter/fml/macros.h"
#include "impeller/aiks/canvas.h"
#include "impeller/aiks/paint.h"

namespace impeller {

class DlDispatcher final : public flutter::DlOpReceiver {
 public:
  DlDispatcher();

  explicit DlDispatcher(Rect cull_rect);

  explicit DlDispatcher(IRect cull_rect);

  ~DlDispatcher();

  Picture EndRecordingAsPicture();

  // |flutter::DlOpReceiver|
  void setAntiAlias(bool aa) override;

  // |flutter::DlOpReceiver|
  void setDither(bool dither) override;

  // |flutter::DlOpReceiver|
  void setDrawStyle(flutter::DlDrawStyle style) override;

  // |flutter::DlOpReceiver|
  void setColor(flutter::DlColor color) override;

  // |flutter::DlOpReceiver|
  void setStrokeWidth(flutter::DlScalar width) override;

  // |flutter::DlOpReceiver|
  void setStrokeMiter(flutter::DlScalar limit) override;

  // |flutter::DlOpReceiver|
  void setStrokeCap(flutter::DlStrokeCap cap) override;

  // |flutter::DlOpReceiver|
  void setStrokeJoin(flutter::DlStrokeJoin join) override;

  // |flutter::DlOpReceiver|
  void setColorSource(const flutter::DlColorSource* source) override;

  // |flutter::DlOpReceiver|
  void setColorFilter(const flutter::DlColorFilter* filter) override;

  // |flutter::DlOpReceiver|
  void setInvertColors(bool invert) override;

  // |flutter::DlOpReceiver|
  void setBlendMode(flutter::DlBlendMode mode) override;

  // |flutter::DlOpReceiver|
  void setPathEffect(const flutter::DlPathEffect* effect) override;

  // |flutter::DlOpReceiver|
  void setMaskFilter(const flutter::DlMaskFilter* filter) override;

  // |flutter::DlOpReceiver|
  void setImageFilter(const flutter::DlImageFilter* filter) override;

  // |flutter::DlOpReceiver|
  void save() override;

  // |flutter::DlOpReceiver|
  void saveLayer(const flutter::DlFRect* bounds,
                 const flutter::SaveLayerOptions options,
                 const flutter::DlImageFilter* backdrop) override;

  // |flutter::DlOpReceiver|
  void restore() override;

  // |flutter::DlOpReceiver|
  void translate(flutter::DlScalar tx, flutter::DlScalar ty) override;

  // |flutter::DlOpReceiver|
  void scale(flutter::DlScalar sx, flutter::DlScalar sy) override;

  // |flutter::DlOpReceiver|
  void rotate(flutter::DlAngle angle) override;

  // |flutter::DlOpReceiver|
  void skew(flutter::DlScalar sx, flutter::DlScalar sy) override;

  // |flutter::DlOpReceiver|
  void transform2DAffine(flutter::DlScalar mxx,
                         flutter::DlScalar mxy,
                         flutter::DlScalar mxt,
                         flutter::DlScalar myx,
                         flutter::DlScalar myy,
                         flutter::DlScalar myt) override;

  // |flutter::DlOpReceiver|
  void transformFullPerspective(flutter::DlScalar mxx,
                                flutter::DlScalar mxy,
                                flutter::DlScalar mxz,
                                flutter::DlScalar mxt,
                                flutter::DlScalar myx,
                                flutter::DlScalar myy,
                                flutter::DlScalar myz,
                                flutter::DlScalar myt,
                                flutter::DlScalar mzx,
                                flutter::DlScalar mzy,
                                flutter::DlScalar mzz,
                                flutter::DlScalar mzt,
                                flutter::DlScalar mwx,
                                flutter::DlScalar mwy,
                                flutter::DlScalar mwz,
                                flutter::DlScalar mwt) override;

  // |flutter::DlOpReceiver|
  void transformReset() override;

  // |flutter::DlOpReceiver|
  void clipRect(const flutter::DlFRect& rect,
                ClipOp clip_op,
                bool is_aa) override;

  // |flutter::DlOpReceiver|
  void clipRRect(const flutter::DlFRRect& rrect,
                 ClipOp clip_op,
                 bool is_aa) override;

  // |flutter::DlOpReceiver|
  void clipPath(const flutter::DlPath& path,
                ClipOp clip_op,
                bool is_aa) override;

  // |flutter::DlOpReceiver|
  void drawColor(flutter::DlColor color, flutter::DlBlendMode mode) override;

  // |flutter::DlOpReceiver|
  void drawPaint() override;

  // |flutter::DlOpReceiver|
  void drawLine(const flutter::DlFPoint& p0,
                const flutter::DlFPoint& p1) override;

  // |flutter::DlOpReceiver|
  void drawRect(const flutter::DlFRect& rect) override;

  // |flutter::DlOpReceiver|
  void drawOval(const flutter::DlFRect& bounds) override;

  // |flutter::DlOpReceiver|
  void drawCircle(const flutter::DlFPoint& center,
                  flutter::DlScalar radius) override;

  // |flutter::DlOpReceiver|
  void drawRRect(const flutter::DlFRRect& rrect) override;

  // |flutter::DlOpReceiver|
  void drawDRRect(const flutter::DlFRRect& outer,
                  const flutter::DlFRRect& inner) override;

  // |flutter::DlOpReceiver|
  void drawPath(const flutter::DlPath& path) override;

  // |flutter::DlOpReceiver|
  void drawArc(const flutter::DlFRect& oval_bounds,
               flutter::DlScalar start_degrees,
               flutter::DlScalar sweep_degrees,
               bool use_center) override;

  // |flutter::DlOpReceiver|
  void drawPoints(PointMode mode,
                  uint32_t count,
                  const flutter::DlFPoint points[]) override;

  // |flutter::DlOpReceiver|
  void drawVertices(const flutter::DlVertices* vertices,
                    flutter::DlBlendMode dl_mode) override;

  // |flutter::DlOpReceiver|
  void drawImage(const sk_sp<flutter::DlImage> image,
                 const flutter::DlFPoint point,
                 flutter::DlImageSampling sampling,
                 bool render_with_attributes) override;

  // |flutter::DlOpReceiver|
  void drawImageRect(const sk_sp<flutter::DlImage> image,
                     const flutter::DlFRect& src,
                     const flutter::DlFRect& dst,
                     flutter::DlImageSampling sampling,
                     bool render_with_attributes,
                     SrcRectConstraint constraint) override;

  // |flutter::DlOpReceiver|
  void drawImageNine(const sk_sp<flutter::DlImage> image,
                     const flutter::DlIRect& center,
                     const flutter::DlFRect& dst,
                     flutter::DlFilterMode filter,
                     bool render_with_attributes) override;

  // |flutter::DlOpReceiver|
  void drawAtlas(const sk_sp<flutter::DlImage> atlas,
                 const flutter::DlRSTransform xform[],
                 const flutter::DlFRect tex[],
                 const flutter::DlColor colors[],
                 int count,
                 flutter::DlBlendMode mode,
                 flutter::DlImageSampling sampling,
                 const flutter::DlFRect* cull_rect,
                 bool render_with_attributes) override;

  // |flutter::DlOpReceiver|
  void drawDisplayList(const sk_sp<flutter::DisplayList> display_list,
                       flutter::DlScalar opacity) override;

  // |flutter::DlOpReceiver|
  void drawTextBlob(const sk_sp<SkTextBlob> blob,
                    flutter::DlScalar x,
                    flutter::DlScalar y) override;

  // |flutter::DlOpReceiver|
  void drawShadow(const flutter::DlPath& path,
                  const flutter::DlColor color,
                  const flutter::DlScalar elevation,
                  bool transparent_occluder,
                  flutter::DlScalar dpr) override;

 private:
  Paint paint_;
  Canvas canvas_;
  Matrix initial_matrix_;

  FML_DISALLOW_COPY_AND_ASSIGN(DlDispatcher);
};

}  // namespace impeller

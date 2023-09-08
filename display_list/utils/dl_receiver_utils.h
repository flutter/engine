// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_DISPLAY_LIST_UTILS_DL_RECEIVER_UTILS_H_
#define FLUTTER_DISPLAY_LIST_UTILS_DL_RECEIVER_UTILS_H_

#include "flutter/display_list/dl_op_receiver.h"
#include "flutter/fml/logging.h"

// This file contains various utility classes to ease implementing
// a Flutter DisplayList DlOpReceiver, including:
//
// IgnoreAttributeDispatchHelper:
// IgnoreClipDispatchHelper:
// IgnoreTransformDispatchHelper
//     Empty overrides of all of the associated methods of DlOpReceiver
//     for receivers that only track some of the rendering operations

namespace flutter {

// A utility class that will ignore all DlOpReceiver methods relating
// to the setting of attributes.
class IgnoreAttributeDispatchHelper : public virtual DlOpReceiver {
 public:
  void setAntiAlias(bool aa) override {}
  void setDither(bool dither) override {}
  void setInvertColors(bool invert) override {}
  void setStrokeCap(DlStrokeCap cap) override {}
  void setStrokeJoin(DlStrokeJoin join) override {}
  void setDrawStyle(DlDrawStyle style) override {}
  void setStrokeWidth(float width) override {}
  void setStrokeMiter(float limit) override {}
  void setColor(DlColor color) override {}
  void setBlendMode(DlBlendMode mode) override {}
  void setColorSource(const DlColorSource* source) override {}
  void setImageFilter(const DlImageFilter* filter) override {}
  void setColorFilter(const DlColorFilter* filter) override {}
  void setPathEffect(const DlPathEffect* effect) override {}
  void setMaskFilter(const DlMaskFilter* filter) override {}
};

// A utility class that will ignore all DlOpReceiver methods relating
// to setting a clip.
class IgnoreClipDispatchHelper : public virtual DlOpReceiver {
  void clipRect(const DlFRect& rect,
                DlCanvas::ClipOp clip_op,
                bool is_aa) override {}
  void clipRRect(const DlFRRect& rrect,
                 DlCanvas::ClipOp clip_op,
                 bool is_aa) override {}
  void clipPath(const DlPath& path,
                DlCanvas::ClipOp clip_op,
                bool is_aa) override {}
};

// A utility class that will ignore all DlOpReceiver methods relating
// to modifying the transform.
class IgnoreTransformDispatchHelper : public virtual DlOpReceiver {
 public:
  void translate(DlScalar tx, DlScalar ty) override {}
  void scale(DlScalar sx, DlScalar sy) override {}
  void rotate(DlAngle angle) override {}
  void skew(DlScalar sx, DlScalar sy) override {}
  // clang-format off
  // 2x3 2D affine subset of a 4x4 transform in row major order
  void transform2DAffine(DlScalar mxx, DlScalar mxy, DlScalar mxt,
                         DlScalar myx, DlScalar myy, DlScalar myt) override {}
  // full 4x4 transform in row major order
  void transformFullPerspective(
      DlScalar mxx, DlScalar mxy, DlScalar mxz, DlScalar mxt,
      DlScalar myx, DlScalar myy, DlScalar myz, DlScalar myt,
      DlScalar mzx, DlScalar mzy, DlScalar mzz, DlScalar mzt,
      DlScalar mwx, DlScalar mwy, DlScalar mwz, DlScalar mwt) override {}
  // clang-format on
  void transformReset() override {}
};

class IgnoreDrawDispatchHelper : public virtual DlOpReceiver {
 public:
  void save() override {}
  void saveLayer(const DlFRect* bounds,
                 const SaveLayerOptions options,
                 const DlImageFilter* backdrop) override {}
  void restore() override {}
  void drawColor(DlColor color, DlBlendMode mode) override {}
  void drawPaint() override {}
  void drawLine(const DlFPoint& p0, const DlFPoint& p1) override {}
  void drawRect(const DlFRect& rect) override {}
  void drawOval(const DlFRect& bounds) override {}
  void drawCircle(const DlFPoint& center, DlScalar radius) override {}
  void drawRRect(const DlFRRect& rrect) override {}
  void drawDRRect(const DlFRRect& outer, const DlFRRect& inner) override {}
  void drawPath(const DlPath& path) override {}
  void drawArc(const DlFRect& oval_bounds,
               DlScalar start_degrees,
               DlScalar sweep_degrees,
               bool use_center) override {}
  void drawPoints(DlCanvas::PointMode mode,
                  uint32_t count,
                  const DlFPoint points[]) override {}
  void drawVertices(const DlVertices* vertices, DlBlendMode mode) override {}
  void drawImage(const sk_sp<DlImage> image,
                 const DlFPoint point,
                 DlImageSampling sampling,
                 bool render_with_attributes) override {}
  void drawImageRect(const sk_sp<DlImage> image,
                     const DlFRect& src,
                     const DlFRect& dst,
                     DlImageSampling sampling,
                     bool render_with_attributes,
                     SrcRectConstraint constraint) override {}
  void drawImageNine(const sk_sp<DlImage> image,
                     const DlIRect& center,
                     const DlFRect& dst,
                     DlFilterMode filter,
                     bool render_with_attributes) override {}
  void drawAtlas(const sk_sp<DlImage> atlas,
                 const DlRSTransform xform[],
                 const DlFRect tex[],
                 const DlColor colors[],
                 int count,
                 DlBlendMode mode,
                 DlImageSampling sampling,
                 const DlFRect* cull_rect,
                 bool render_with_attributes) override {}
  void drawDisplayList(const sk_sp<DisplayList> display_list,
                       DlScalar opacity) override {}
  void drawTextBlob(const sk_sp<SkTextBlob> blob,
                    DlScalar x,
                    DlScalar y) override {}
  void drawShadow(const DlPath& path,
                  const DlColor color,
                  const DlScalar elevation,
                  bool transparent_occluder,
                  DlScalar dpr) override {}
};

}  // namespace flutter

#endif  // FLUTTER_DISPLAY_LIST_UTILS_DL_RECEIVER_UTILS_H_

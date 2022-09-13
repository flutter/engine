// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_DISPLAY_LIST_DISPLAY_LIST_DELEGATE_H_
#define FLUTTER_DISPLAY_LIST_DISPLAY_LIST_DELEGATE_H_

#include "flutter/display_list/display_list.h"
#include "flutter/display_list/display_list_dispatcher.h"

namespace flutter {

//------------------------------------------------------------------------------
/// @brief      A utility class that dispatches all Dispatcher methods to
///             a delegate. Classes that want to filter a DisplayList dispatch
///             stream can inherit from this class and only override those
///             methods that pertain to their purpose.
///
class DispatcherDelegate : public virtual Dispatcher {
 public:
  DispatcherDelegate(Dispatcher& delegate) : delegate_(delegate) {}

  void setAntiAlias(bool aa) override { delegate_.setAntiAlias(aa); }
  void setDither(bool dither) override { delegate_.setDither(dither); }
  void setStyle(DlDrawStyle style) override { delegate_.setStyle(style); }
  void setColor(DlColor color) override { delegate_.setColor(color); }
  void setStrokeWidth(float width) override { delegate_.setStrokeWidth(width); }
  void setStrokeMiter(float limit) override { delegate_.setStrokeMiter(limit); }
  void setStrokeCap(DlStrokeCap cap) override { delegate_.setStrokeCap(cap); }
  void setStrokeJoin(DlStrokeJoin join) override {
    delegate_.setStrokeJoin(join);
  }
  void setColorSource(const DlColorSource* source) override {
    delegate_.setColorSource(source);
  }
  void setColorFilter(const DlColorFilter* filter) override {
    delegate_.setColorFilter(filter);
  }

  void setInvertColors(bool invert) override {
    delegate_.setInvertColors(invert);
  }
  void setBlendMode(DlBlendMode mode) override { delegate_.setBlendMode(mode); }
  void setBlender(sk_sp<SkBlender> blender) override {
    delegate_.setBlender(blender);
  }
  void setPathEffect(const DlPathEffect* effect) override {
    delegate_.setPathEffect(effect);
  }
  void setMaskFilter(const DlMaskFilter* filter) override {
    delegate_.setMaskFilter(filter);
  }
  void setImageFilter(const DlImageFilter* filter) override {
    delegate_.setImageFilter(filter);
  }

  void save() override { delegate_.save(); }
  void saveLayer(const SkRect* bounds,
                 RenderWith with,
                 const DlImageFilter* backdrop,
                 int optimizations) override {
    delegate_.saveLayer(bounds, with, backdrop, optimizations);
  }
  void restore() override { delegate_.restore(); }

  void translate(SkScalar tx, SkScalar ty) override {
    delegate_.translate(tx, ty);
  }
  void scale(SkScalar sx, SkScalar sy) override { delegate_.scale(sx, sy); }
  void rotate(SkScalar degrees) override { delegate_.rotate(degrees); }
  void skew(SkScalar sx, SkScalar sy) override { delegate_.skew(sx, sy); }

  // clang-format off
  void transform2DAffine(SkScalar mxx, SkScalar mxy, SkScalar mxt,
                         SkScalar myx, SkScalar myy, SkScalar myt) override {
    delegate_.transform2DAffine(mxx, mxy, mxt,
                                myx, myy, myt);
  }
  void transformFullPerspective(
      SkScalar mxx, SkScalar mxy, SkScalar mxz, SkScalar mxt,
      SkScalar myx, SkScalar myy, SkScalar myz, SkScalar myt,
      SkScalar mzx, SkScalar mzy, SkScalar mzz, SkScalar mzt,
      SkScalar mwx, SkScalar mwy, SkScalar mwz, SkScalar mwt) override {
    delegate_.transformFullPerspective(mxx, mxy, mxz, mxt,
                                       myx, myy, myz, myt,
                                       mzx, mzy, mzz, mzt,
                                       mwx, mwy, mwz, mwt);
  }
  // clang-format on

  void transformReset() override { delegate_.transformReset(); }

  void clipRect(const SkRect& rect, SkClipOp clip_op, bool is_aa) override {
    delegate_.clipRect(rect, clip_op, is_aa);
  }
  void clipRRect(const SkRRect& rrect, SkClipOp clip_op, bool is_aa) override {
    delegate_.clipRRect(rrect, clip_op, is_aa);
  }
  void clipPath(const SkPath& path, SkClipOp clip_op, bool is_aa) override {
    delegate_.clipPath(path, clip_op, is_aa);
  }

  void drawColor(DlColor color, DlBlendMode mode) override {
    delegate_.drawColor(color, mode);
  }
  void drawPaint() override { delegate_.drawPaint(); }
  void drawLine(const SkPoint& p0, const SkPoint& p1) override {
    delegate_.drawLine(p0, p1);
  }
  void drawRect(const SkRect& rect) override { delegate_.drawRect(rect); }
  void drawOval(const SkRect& bounds) override { delegate_.drawOval(bounds); }
  void drawCircle(const SkPoint& center, SkScalar radius) override {
    delegate_.drawCircle(center, radius);
  }
  void drawRRect(const SkRRect& rrect) override { delegate_.drawRRect(rrect); }
  void drawDRRect(const SkRRect& outer, const SkRRect& inner) override {
    delegate_.drawDRRect(outer, inner);
  }
  void drawPath(const SkPath& path) override { delegate_.drawPath(path); }
  void drawArc(const SkRect& oval_bounds,
               SkScalar start_degrees,
               SkScalar sweep_degrees,
               bool use_center) override {
    delegate_.drawArc(oval_bounds, start_degrees, sweep_degrees, use_center);
  }
  void drawPoints(SkCanvas::PointMode mode,
                  uint32_t count,
                  const SkPoint points[]) override {
    delegate_.drawPoints(mode, count, points);
  }
  void drawSkVertices(const sk_sp<SkVertices> vertices,
                      SkBlendMode mode) override {
    delegate_.drawSkVertices(vertices, mode);
  }
  void drawVertices(const DlVertices* vertices, DlBlendMode mode) override {
    delegate_.drawVertices(vertices, mode);
  }
  void drawImage(const sk_sp<DlImage> image,
                 const SkPoint point,
                 DlImageSampling sampling,
                 RenderWith with) override {
    delegate_.drawImage(image, point, sampling, with);
  }
  void drawImageRect(const sk_sp<DlImage> image,
                     const SkRect& src,
                     const SkRect& dst,
                     DlImageSampling sampling,
                     RenderWith with,
                     SkCanvas::SrcRectConstraint constraint) override {
    delegate_.drawImageRect(image, src, dst, sampling, with, constraint);
  }
  void drawImageNine(const sk_sp<DlImage> image,
                     const SkIRect& center,
                     const SkRect& dst,
                     DlFilterMode filter,
                     RenderWith with) override {
    delegate_.drawImageNine(image, center, dst, filter, with);
  }
  void drawImageLattice(const sk_sp<DlImage> image,
                        const SkCanvas::Lattice& lattice,
                        const SkRect& dst,
                        DlFilterMode filter,
                        RenderWith with) override {
    delegate_.drawImageLattice(image, lattice, dst, filter, with);
  }
  void drawAtlas(const sk_sp<DlImage> atlas,
                 const SkRSXform xform[],
                 const SkRect tex[],
                 const DlColor colors[],
                 int count,
                 DlBlendMode mode,
                 DlImageSampling sampling,
                 const SkRect* cull_rect,
                 RenderWith with) override {
    delegate_.drawAtlas(atlas, xform, tex, colors, count, mode, sampling,
                        cull_rect, with);
  }
  void drawPicture(const sk_sp<SkPicture> picture,
                   const SkMatrix* matrix,
                   RenderWith with) override {
    delegate_.drawPicture(picture, matrix, with);
  }
  void drawDisplayList(const sk_sp<DisplayList> display_list,
                       SkScalar opacity) override {
    delegate_.drawDisplayList(display_list, opacity);
  }
  void drawTextBlob(const sk_sp<SkTextBlob> blob,
                    SkScalar x,
                    SkScalar y) override {
    delegate_.drawTextBlob(blob, x, y);
  }
  void drawShadow(const SkPath& path,
                  const DlColor color,
                  const SkScalar elevation,
                  bool transparent_occluder,
                  SkScalar dpr) override {
    delegate_.drawShadow(path, color, elevation, transparent_occluder, dpr);
  }

 protected:
  Dispatcher& delegate_;
};

}  // namespace flutter

#endif  // FLUTTER_DISPLAY_LIST_DISPLAY_LIST_DELEGATE_H_

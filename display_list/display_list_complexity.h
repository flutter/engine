// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_DISPLAY_LIST_COMPLEXITY_H_
#define FLUTTER_FLOW_DISPLAY_LIST_COMPLEXITY_H_

#include "flutter/display_list/display_list_dispatcher.h"
#include "flutter/display_list/types.h"

namespace flutter {

class DisplayListComplexityCalculator : public virtual Dispatcher {
 public:
  virtual ~DisplayListComplexityCalculator() = default;
  virtual unsigned int complexity_score() = 0;
  virtual bool should_be_cached() = 0;
};

class DisplayListNaiveComplexityCalculator
    : public virtual DisplayListComplexityCalculator {
 public:
  DisplayListNaiveComplexityCalculator() : complexity_score_(0) {}

  ~DisplayListNaiveComplexityCalculator() override = default;

  void setAntiAlias(bool aa) override {}
  void setDither(bool dither) override {}
  void setInvertColors(bool invert) override {}
  void setStrokeCap(SkPaint::Cap cap) override {}
  void setStrokeJoin(SkPaint::Join join) override {}
  void setStyle(SkPaint::Style style) override {}
  void setStrokeWidth(SkScalar width) override {}
  void setStrokeMiter(SkScalar limit) override {}
  void setColor(SkColor color) override {}
  void setBlendMode(SkBlendMode mode) override {}
  void setBlender(sk_sp<SkBlender> blender) override {}
  void setShader(sk_sp<SkShader> shader) override {}
  void setImageFilter(sk_sp<SkImageFilter> filter) override {}
  void setColorFilter(sk_sp<SkColorFilter> filter) override {}
  void setPathEffect(sk_sp<SkPathEffect> effect) override {}
  void setMaskFilter(sk_sp<SkMaskFilter> filter) override {}
  void setMaskBlurFilter(SkBlurStyle style, SkScalar sigma) override {}

  void translate(SkScalar tx, SkScalar ty) override { complexity_score_++; }
  void scale(SkScalar sx, SkScalar sy) override { complexity_score_++; }
  void rotate(SkScalar degrees) override { complexity_score_++; }
  void skew(SkScalar sx, SkScalar sy) override { complexity_score_++; }
  // 2x3 2D affine subset of a 4x4 transform in row major order
  void transform2DAffine(SkScalar mxx,
                         SkScalar mxy,
                         SkScalar mxt,
                         SkScalar myx,
                         SkScalar myy,
                         SkScalar myt) override {
    complexity_score_++;
  }
  // full 4x4 transform in row major order
  void transformFullPerspective(SkScalar mxx,
                                SkScalar mxy,
                                SkScalar mxz,
                                SkScalar mxt,
                                SkScalar myx,
                                SkScalar myy,
                                SkScalar myz,
                                SkScalar myt,
                                SkScalar mzx,
                                SkScalar mzy,
                                SkScalar mzz,
                                SkScalar mzt,
                                SkScalar mwx,
                                SkScalar mwy,
                                SkScalar mwz,
                                SkScalar mwt) override {
    complexity_score_++;
  }

  void clipRect(const SkRect& rect, SkClipOp clip_op, bool is_aa) override {
    complexity_score_++;
  }
  void clipRRect(const SkRRect& rrect, SkClipOp clip_op, bool is_aa) override {
    complexity_score_++;
  }
  void clipPath(const SkPath& path, SkClipOp clip_op, bool is_aa) override {
    complexity_score_++;
  }

  void save() override { complexity_score_++; }
  void saveLayer(const SkRect* bounds, bool with_paint) override {
    complexity_score_++;
  }
  void restore() override { complexity_score_++; }

  void drawPaint() override { complexity_score_++; }
  void drawColor(SkColor color, SkBlendMode mode) override {
    complexity_score_++;
  }
  void drawLine(const SkPoint& p0, const SkPoint& p1) override {
    complexity_score_++;
  }
  void drawRect(const SkRect& rect) override { complexity_score_++; }
  void drawOval(const SkRect& bounds) override { complexity_score_++; }
  void drawCircle(const SkPoint& center, SkScalar radius) override {
    complexity_score_++;
  }
  void drawRRect(const SkRRect& rrect) override { complexity_score_++; }
  void drawDRRect(const SkRRect& outer, const SkRRect& inner) override {
    complexity_score_++;
  }
  void drawPath(const SkPath& path) override { complexity_score_++; }
  void drawArc(const SkRect& bounds,
               SkScalar start,
               SkScalar sweep,
               bool useCenter) override {
    complexity_score_++;
  }
  void drawPoints(SkCanvas::PointMode mode,
                  uint32_t count,
                  const SkPoint pts[]) override {
    complexity_score_++;
  }
  void drawVertices(const sk_sp<SkVertices> vertices,
                    SkBlendMode mode) override {
    complexity_score_++;
  }
  void drawImage(const sk_sp<SkImage> image,
                 const SkPoint point,
                 const SkSamplingOptions& sampling,
                 bool render_with_attributes) override {
    complexity_score_++;
  }
  void drawImageRect(const sk_sp<SkImage> image,
                     const SkRect& src,
                     const SkRect& dst,
                     const SkSamplingOptions& sampling,
                     bool render_with_attributes,
                     SkCanvas::SrcRectConstraint constraint) override {
    complexity_score_++;
  }
  void drawImageNine(const sk_sp<SkImage> image,
                     const SkIRect& center,
                     const SkRect& dst,
                     SkFilterMode filter,
                     bool render_with_attributes) override {
    complexity_score_++;
  }
  void drawImageLattice(const sk_sp<SkImage> image,
                        const SkCanvas::Lattice& lattice,
                        const SkRect& dst,
                        SkFilterMode filter,
                        bool render_with_attributes) override {
    complexity_score_++;
  }
  void drawAtlas(const sk_sp<SkImage> atlas,
                 const SkRSXform xform[],
                 const SkRect tex[],
                 const SkColor colors[],
                 int count,
                 SkBlendMode mode,
                 const SkSamplingOptions& sampling,
                 const SkRect* cullRect,
                 bool render_with_attributes) override {
    complexity_score_++;
  }
  void drawPicture(const sk_sp<SkPicture> picture,
                   const SkMatrix* matrix,
                   bool with_save_layer) override {}
  void drawDisplayList(const sk_sp<DisplayList> display_list) override {
    complexity_score_ += display_list->complexity_score();
  }
  void drawTextBlob(const sk_sp<SkTextBlob> blob,
                    SkScalar x,
                    SkScalar y) override {
    complexity_score_++;
  }
  void drawShadow(const SkPath& path,
                  const SkColor color,
                  const SkScalar elevation,
                  bool transparent_occluder,
                  SkScalar dpr) override {
    complexity_score_++;
  }

  unsigned int complexity_score() override { return complexity_score_; }
  bool should_be_cached() override { return complexity_score_ > 5; }

 private:
  unsigned int complexity_score_;
};

}  // namespace flutter

#endif  // FLUTTER_FLOW_DISPLAY_LIST_COMPLEXITY_H_

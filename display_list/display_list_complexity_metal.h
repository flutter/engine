// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_DISPLAY_LIST_COMPLEXITY_METAL_H_
#define FLUTTER_FLOW_DISPLAY_LIST_COMPLEXITY_METAL_H_

#include "flutter/display_list/display_list_complexity.h"
#include "flutter/display_list/display_list_dispatcher.h"

class DisplayListMetalComplexityCalculator
    : public DisplayListComplexityCalculator {
 public:
  static DisplayListComplexityCalculator* GetInstance();

  unsigned int compute(DisplayList* display_list) override {
    display_list->render_to(helper_);
  }

  bool should_be_cached(unsigned int complexity_score) override {
    return complexity_score > 5u;
  }

 private:
  class MetalHelper : public virtual Dispatcher,
                      public virtual IgnoreClipDispatchHelper,
                      public virtual IgnoreTransformDispatchHelper {
    MetalHelper();

    void setDither(bool dither) override {}
    void setInvertColors(bool invert) override {}
    void setStrokeCap(SkPaint::Cap cap) override {}
    void setStrokeJoin(SkPaint::Join join) override {}
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

    void setAntiAlias(bool aa) override;
    void setStyle(SkPaint::Style style) override;
    void setStrokeWidth(SkScalar width) override;

    void save() override;
    void saveLayer(const SkRect* bounds,
                  const SaveLayerOptions options) override;
    void restore() override;
    void drawColor(SkColor color, SkBlendMode mode) override;
    void drawPaint() override;
    void drawLine(const SkPoint& p0, const SkPoint& p1) override;
    void drawRect(const SkRect& rect) override;
    void drawOval(const SkRect& bounds) override;
    void drawCircle(const SkPoint& center, SkScalar radius) override;
    void drawRRect(const SkRRect& rrect) override;
    void drawDRRect(const SkRRect& outer, const SkRRect& inner) override;
    void drawPath(const SkPath& path) override;
    void drawArc(const SkRect& oval_bounds,
                SkScalar start_degrees,
                SkScalar sweep_degrees,
                bool use_center) override;
    void drawPoints(SkCanvas::PointMode mode,
                    uint32_t count,
                    const SkPoint points[]) override;
    void drawVertices(const sk_sp<SkVertices> vertices,
                      SkBlendMode mode) override;
    void drawImage(const sk_sp<SkImage> image,
                  const SkPoint point,
                  const SkSamplingOptions& sampling,
                  bool render_with_attributes) override;
    void drawImageRect(const sk_sp<SkImage> image,
                      const SkRect& src,
                      const SkRect& dst,
                      const SkSamplingOptions& sampling,
                      bool render_with_attributes,
                      SkCanvas::SrcRectConstraint constraint) override;
    void drawImageNine(const sk_sp<SkImage> image,
                      const SkIRect& center,
                      const SkRect& dst,
                      SkFilterMode filter,
                      bool render_with_attributes) override;
    void drawImageLattice(const sk_sp<SkImage> image,
                          const SkCanvas::Lattice& lattice,
                          const SkRect& dst,
                          SkFilterMode filter,
                          bool render_with_attributes) override;
    void drawAtlas(const sk_sp<SkImage> atlas,
                  const SkRSXform xform[],
                  const SkRect tex[],
                  const SkColor colors[],
                  int count,
                  SkBlendMode mode,
                  const SkSamplingOptions& sampling,
                  const SkRect* cull_rect,
                  bool render_with_attributes) override;
    void drawPicture(const sk_sp<SkPicture> picture,
                    const SkMatrix* matrix,
                    bool render_with_attributes) override;
    void drawDisplayList(const sk_sp<DisplayList> display_list) override;
    void drawTextBlob(const sk_sp<SkTextBlob> blob,
                      SkScalar x,
                      SkScalar y) override;
    void drawShadow(const SkPath& path,
                    const SkColor color,
                    const SkScalar elevation,
                    bool transparent_occluder,
                    SkScalar dpr) override;
  private:
    bool isAntiAliased() { return paint_stack_.top().isAntiAlias(); }
    bool isHairline() { return paint_stack_.top().getStrokeWidth() == 0.0f; }
    SkPaint::Style style() { return paint_stack_.top().getStyle(); }

    typedef enum {
      kSaveLayer,
      kSave
    } SaveType;

    std::stack<SkPaint> paint_stack_;
    std::stack<SaveType> save_types_;

    unsigned int complexity_score_;
  };

  std::unique_ptr<MetalHelper> helper_;

  DisplayListMetalComplexityCalculator();
  static DisplayListMetalComplexityCalculator* instance_;
};

#endif // FLUTTER_FLOW_DISPLAY_LIST_COMPLEXITY_METAL_H_
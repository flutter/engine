// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_DISPLAY_LIST_BENCHMARKING_DL_COMPLEXITY_GL_H_
#define FLUTTER_FLOW_DISPLAY_LIST_BENCHMARKING_DL_COMPLEXITY_GL_H_

#include "flutter/display_list/benchmarking/dl_complexity_helper.h"

namespace flutter {

class DisplayListGLComplexityCalculator
    : public DisplayListComplexityCalculator {
 public:
  static DisplayListGLComplexityCalculator* GetInstance();

  unsigned int Compute(const DisplayList* display_list) override {
    GLHelper helper(ceiling_);
    display_list->Dispatch(helper);
    return helper.ComplexityScore();
  }

  bool ShouldBeCached(unsigned int complexity_score) override {
    // Set cache threshold at 1ms
    return false;
  }

  void SetComplexityCeiling(unsigned int ceiling) override {
    ceiling_ = ceiling;
  }

 private:
  class GLHelper : public ComplexityCalculatorHelper {
   public:
    GLHelper(unsigned int ceiling)
        : ComplexityCalculatorHelper(ceiling),
          save_layer_count_(0),
          draw_text_blob_count_(0) {}

    void saveLayer(const DlFRect* bounds,
                   const SaveLayerOptions options,
                   const DlImageFilter* backdrop) override;

    void drawLine(const DlFPoint& p0, const DlFPoint& p1) override;
    void drawRect(const DlFRect& rect) override;
    void drawOval(const DlFRect& bounds) override;
    void drawCircle(const DlFPoint& center, DlScalar radius) override;
    void drawRRect(const DlFRRect& rrect) override;
    void drawDRRect(const DlFRRect& outer, const DlFRRect& inner) override;
    void drawPath(const DlPath& path) override;
    void drawArc(const DlFRect& oval_bounds,
                 DlScalar start_degrees,
                 DlScalar sweep_degrees,
                 bool use_center) override;
    void drawPoints(DlCanvas::PointMode mode,
                    uint32_t count,
                    const DlFPoint points[]) override;
    void drawVertices(const DlVertices* vertices, DlBlendMode mode) override;
    void drawImage(const sk_sp<DlImage> image,
                   const DlFPoint point,
                   DlImageSampling sampling,
                   bool render_with_attributes) override;
    void drawImageNine(const sk_sp<DlImage> image,
                       const DlIRect& center,
                       const DlFRect& dst,
                       DlFilterMode filter,
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

   protected:
    void ImageRect(const DlISize& size,
                   bool texture_backed,
                   bool render_with_attributes,
                   bool enforce_src_edges) override;

    unsigned int BatchedComplexity() override;

   private:
    unsigned int save_layer_count_;
    unsigned int draw_text_blob_count_;
  };

  DisplayListGLComplexityCalculator()
      : ceiling_(std::numeric_limits<unsigned int>::max()) {}
  static DisplayListGLComplexityCalculator* instance_;

  unsigned int ceiling_;
};

}  // namespace flutter

#endif  // FLUTTER_FLOW_DISPLAY_LIST_BENCHMARKING_DL_COMPLEXITY_GL_H_

// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_ENTITY_CONTENTS_TYPED_CONTENTS_VISITOR_H_
#define FLUTTER_IMPELLER_ENTITY_CONTENTS_TYPED_CONTENTS_VISITOR_H_

// All of these includes are unfortunate, but this is the price we pay to be
// able to use std::is_base_of_v.  The alternative is to maintain that in code
// which could introduce bugs.
#include "impeller/entity/contents/anonymous_contents.h"
#include "impeller/entity/contents/atlas_contents.h"
#include "impeller/entity/contents/checkerboard_contents.h"
#include "impeller/entity/contents/conical_gradient_contents.h"
#include "impeller/entity/contents/contents_visitor.h"
#include "impeller/entity/contents/filters/blend_filter_contents.h"
#include "impeller/entity/contents/filters/border_mask_blur_filter_contents.h"
#include "impeller/entity/contents/filters/color_matrix_filter_contents.h"
#include "impeller/entity/contents/filters/linear_to_srgb_filter_contents.h"
#include "impeller/entity/contents/filters/local_matrix_filter_contents.h"
#include "impeller/entity/contents/filters/matrix_filter_contents.h"
#include "impeller/entity/contents/filters/morphology_filter_contents.h"
#include "impeller/entity/contents/filters/srgb_to_linear_filter_contents.h"
#include "impeller/entity/contents/filters/yuv_to_rgb_filter_contents.h"
#include "impeller/entity/contents/framebuffer_blend_contents.h"
#include "impeller/entity/contents/linear_gradient_contents.h"
#include "impeller/entity/contents/radial_gradient_contents.h"
#include "impeller/entity/contents/runtime_effect_contents.h"
#include "impeller/entity/contents/solid_color_contents.h"
#include "impeller/entity/contents/solid_rrect_blur_contents.h"
#include "impeller/entity/contents/sweep_gradient_contents.h"
#include "impeller/entity/contents/text_contents.h"
#include "impeller/entity/contents/texture_contents.h"
#include "impeller/entity/contents/tiled_texture_contents.h"
#include "impeller/entity/contents/vertices_contents.h"

namespace impeller {

template <typename BaseT, typename DerivedPtrT>
struct IsDerivedOrSame
    : std::integral_constant<bool,
                             std::is_base_of_v<BaseT, DerivedPtrT> ||
                                 std::is_same_v<BaseT, DerivedPtrT>> {};

/// A ContentsVisitor that only visits Contents of type `T` and its subclasses.
template <typename T>
class TypedContentsVisitor : public ContentsVisitor {
 public:
  virtual void TypedVisit(T* contents) = 0;

  template <typename ContentsT>
  void DoVisit(ContentsT* contents) {
    if constexpr (IsDerivedOrSame<T, ContentsT>::value) {
      TypedVisit(contents);
    }
  }

  void Visit(AnonymousContents* contents) override { DoVisit(contents); }
  void Visit(AtlasColorContents* contents) override { DoVisit(contents); }
  void Visit(AtlasContents* contents) override { DoVisit(contents); }
  void Visit(AtlasTextureContents* contents) override { DoVisit(contents); }
  void Visit(BlendFilterContents* contents) override { DoVisit(contents); }
  void Visit(BorderMaskBlurFilterContents* contents) override {
    DoVisit(contents);
  }
  void Visit(CheckerboardContents* contents) override { DoVisit(contents); }
  void Visit(ClipContents* contents) override { DoVisit(contents); }
  void Visit(ClipRestoreContents* contents) override { DoVisit(contents); }
  void Visit(ColorMatrixFilterContents* contents) override {
    DoVisit(contents);
  }
  void Visit(ConicalGradientContents* contents) override { DoVisit(contents); }
  void Visit(DirectionalMorphologyFilterContents* contents) override {
    DoVisit(contents);
  }
  void Visit(FramebufferBlendContents* contents) override { DoVisit(contents); }
  void Visit(GaussianBlurFilterContents* contents) override {
    DoVisit(contents);
  }
  void Visit(LinearGradientContents* contents) override { DoVisit(contents); }
  void Visit(LinearToSrgbFilterContents* contents) override {
    DoVisit(contents);
  }
  void Visit(LocalMatrixFilterContents* contents) override {
    DoVisit(contents);
  }
  void Visit(MatrixFilterContents* contents) override { DoVisit(contents); }
  void Visit(RadialGradientContents* contents) override { DoVisit(contents); }
  void Visit(RuntimeEffectContents* contents) override { DoVisit(contents); }
  void Visit(SolidColorContents* contents) override { DoVisit(contents); }
  void Visit(SolidRRectBlurContents* contents) override { DoVisit(contents); }
  void Visit(SrgbToLinearFilterContents* contents) override {
    DoVisit(contents);
  }
  void Visit(SweepGradientContents* contents) override { DoVisit(contents); }
  void Visit(TextContents* contents) override { DoVisit(contents); }
  void Visit(TextureContents* contents) override { DoVisit(contents); }
  void Visit(TiledTextureContents* contents) override { DoVisit(contents); }
  void Visit(VerticesColorContents* contents) override { DoVisit(contents); }
  void Visit(VerticesContents* contents) override { DoVisit(contents); }
  void Visit(VerticesUVContents* contents) override { DoVisit(contents); }
  void Visit(YUVToRGBFilterContents* contents) override { DoVisit(contents); }
};

}  // namespace impeller
#endif  // FLUTTER_IMPELLER_ENTITY_CONTENTS_CONTENTS_VISITOR_H_

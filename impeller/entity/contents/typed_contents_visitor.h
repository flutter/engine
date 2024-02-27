// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_ENTITY_CONTENTS_TYPED_CONTENTS_VISITOR_H_
#define FLUTTER_IMPELLER_ENTITY_CONTENTS_TYPED_CONTENTS_VISITOR_H_

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
    : std::integral_constant<
          bool,
          std::is_base_of_v<BaseT, std::remove_pointer_t<DerivedPtrT>> ||
              std::is_same_v<BaseT, std::remove_pointer_t<DerivedPtrT>>> {};

template <typename T>
class TypedContentsVisitor : public ContentsVisitor {
  public:
  virtual void TypedVisit(T* contents) = 0;
  void Visit(AnonymousContents* contents) override { if constexpr (IsDerivedOrSame<T, decltype(contents)>::value) { TypedVisit(contents); }}
  void Visit(AtlasColorContents* contents) override { if constexpr (IsDerivedOrSame<T, decltype(contents)>::value) { TypedVisit(contents); }}
  void Visit(AtlasContents* contents) override { if constexpr (IsDerivedOrSame<T, decltype(contents)>::value) { TypedVisit(contents); }}
  void Visit(AtlasTextureContents* contents) override { if constexpr (IsDerivedOrSame<T, decltype(contents)>::value) { TypedVisit(contents); }}
  void Visit(BlendFilterContents* contents) override { if constexpr (IsDerivedOrSame<T, decltype(contents)>::value) { TypedVisit(contents); }}
  void Visit(BorderMaskBlurFilterContents* contents) override { if constexpr (IsDerivedOrSame<T, decltype(contents)>::value) { TypedVisit(contents); }}
  void Visit(CheckerboardContents* contents) override { if constexpr (IsDerivedOrSame<T, decltype(contents)>::value) { TypedVisit(contents); }}
  void Visit(ClipContents* contents) override { if constexpr (IsDerivedOrSame<T, decltype(contents)>::value) { TypedVisit(contents); }}
  void Visit(ClipRestoreContents* contents) override { if constexpr (IsDerivedOrSame<T, decltype(contents)>::value) { TypedVisit(contents); }}
  void Visit(ColorMatrixFilterContents* contents) override { if constexpr (IsDerivedOrSame<T, decltype(contents)>::value) { TypedVisit(contents); }}
  void Visit(ConicalGradientContents* contents) override { if constexpr (IsDerivedOrSame<T, decltype(contents)>::value) { TypedVisit(contents); }}
  void Visit(DirectionalMorphologyFilterContents* contents) override { if constexpr (IsDerivedOrSame<T, decltype(contents)>::value) { TypedVisit(contents); }}
  void Visit(FramebufferBlendContents* contents) override { if constexpr (IsDerivedOrSame<T, decltype(contents)>::value) { TypedVisit(contents); }}
  void Visit(GaussianBlurFilterContents* contents) override { if constexpr (IsDerivedOrSame<T, decltype(contents)>::value) { TypedVisit(contents); }}
  void Visit(LinearGradientContents* contents) override { if constexpr (IsDerivedOrSame<T, decltype(contents)>::value) { TypedVisit(contents); }}
  void Visit(LinearToSrgbFilterContents* contents) override { if constexpr (IsDerivedOrSame<T, decltype(contents)>::value) { TypedVisit(contents); }}
  void Visit(LocalMatrixFilterContents* contents) override { if constexpr (IsDerivedOrSame<T, decltype(contents)>::value) { TypedVisit(contents); }}
  void Visit(MatrixFilterContents* contents) override { if constexpr (IsDerivedOrSame<T, decltype(contents)>::value) { TypedVisit(contents); }}
  void Visit(RadialGradientContents* contents) override { if constexpr (IsDerivedOrSame<T, decltype(contents)>::value) { TypedVisit(contents); }}
  void Visit(RuntimeEffectContents* contents) override { if constexpr (IsDerivedOrSame<T, decltype(contents)>::value) { TypedVisit(contents); }}
  void Visit(SolidColorContents* contents) override { if constexpr (IsDerivedOrSame<T, decltype(contents)>::value) { TypedVisit(contents); }}
  void Visit(SolidRRectBlurContents* contents) override { if constexpr (IsDerivedOrSame<T, decltype(contents)>::value) { TypedVisit(contents); }}
  void Visit(SrgbToLinearFilterContents* contents) override { if constexpr (IsDerivedOrSame<T, decltype(contents)>::value) { TypedVisit(contents); }}
  void Visit(SweepGradientContents* contents) override { if constexpr (IsDerivedOrSame<T, decltype(contents)>::value) { TypedVisit(contents); }}
  void Visit(TextContents* contents) override { if constexpr (IsDerivedOrSame<T, decltype(contents)>::value) { TypedVisit(contents); }}
  void Visit(TextureContents* contents) override { if constexpr (IsDerivedOrSame<T, decltype(contents)>::value) { TypedVisit(contents); }}
  void Visit(TiledTextureContents* contents) override { if constexpr (IsDerivedOrSame<T, decltype(contents)>::value) { TypedVisit(contents); }}
  void Visit(VerticesColorContents* contents) override { if constexpr (IsDerivedOrSame<T, decltype(contents)>::value) { TypedVisit(contents); }}
  void Visit(VerticesContents* contents) override { if constexpr (IsDerivedOrSame<T, decltype(contents)>::value) { TypedVisit(contents); }}
  void Visit(VerticesUVContents* contents) override { if constexpr (IsDerivedOrSame<T, decltype(contents)>::value) { TypedVisit(contents); }}
  void Visit(YUVToRGBFilterContents* contents) override { if constexpr (IsDerivedOrSame<T, decltype(contents)>::value) { TypedVisit(contents); }}
};

}  // namespace impeller
#endif  // FLUTTER_IMPELLER_ENTITY_CONTENTS_CONTENTS_VISITOR_H_

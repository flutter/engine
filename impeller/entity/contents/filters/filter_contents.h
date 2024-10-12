// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_ENTITY_CONTENTS_FILTERS_FILTER_CONTENTS_H_
#define FLUTTER_IMPELLER_ENTITY_CONTENTS_FILTERS_FILTER_CONTENTS_H_

#include <memory>
#include <optional>
#include <variant>
#include <vector>

#include "impeller/core/formats.h"
#include "impeller/entity/contents/filters/inputs/filter_input.h"
#include "impeller/entity/entity.h"
#include "impeller/entity/geometry/geometry.h"
#include "impeller/geometry/matrix.h"
#include "impeller/geometry/sigma.h"

namespace impeller {

class FilterContents : public Contents {
 public:
  static const int32_t kBlurFilterRequiredMipCount;

  enum class BlurStyle {
    /// Blurred inside and outside.
    kNormal,
    /// Solid inside, blurred outside.
    kSolid,
    /// Nothing inside, blurred outside.
    kOuter,
    /// Blurred inside, nothing outside.
    kInner,
  };

  enum class MorphType { kDilate, kErode };

  /// Creates a gaussian blur that operates in 2 dimensions.
  /// See also: `MakeDirectionalGaussianBlur`
  static std::shared_ptr<FilterContents> MakeGaussianBlur(
      const FilterInput::Ref& input,
      Sigma sigma_x,
      Sigma sigma_y,
      Entity::TileMode tile_mode = Entity::TileMode::kDecal,
      BlurStyle mask_blur_style = BlurStyle::kNormal,
      const Geometry* mask_geometry = nullptr);

  static std::shared_ptr<FilterContents> MakeBorderMaskBlur(
      FilterInput::Ref input,
      Sigma sigma_x,
      Sigma sigma_y,
      BlurStyle blur_style = BlurStyle::kNormal);

  static std::shared_ptr<FilterContents> MakeDirectionalMorphology(
      FilterInput::Ref input,
      Radius radius,
      Vector2 direction,
      MorphType morph_type);

  static std::shared_ptr<FilterContents> MakeMorphology(FilterInput::Ref input,
                                                        Radius radius_x,
                                                        Radius radius_y,
                                                        MorphType morph_type);

  static std::shared_ptr<FilterContents> MakeMatrixFilter(
      FilterInput::Ref input,
      const Matrix& matrix,
      const SamplerDescriptor& desc);

  static std::shared_ptr<FilterContents> MakeLocalMatrixFilter(
      FilterInput::Ref input,
      const Matrix& matrix);

  static std::shared_ptr<FilterContents> MakeYUVToRGBFilter(
      std::shared_ptr<Texture> y_texture,
      std::shared_ptr<Texture> uv_texture,
      YUVColorSpace yuv_color_space);

  FilterContents();

  ~FilterContents() override;

  /// @brief  The input texture sources for this filter. Each input's emitted
  ///         texture is expected to have premultiplied alpha colors.
  ///
  ///         The number of required or optional textures depends on the
  ///         particular filter's implementation.
  void SetInputs(FilterInput::Vector inputs);

  /// @brief  Sets the transform which gets appended to the effect of this
  ///         filter. Note that this is in addition to the entity's transform.
  ///
  ///         This is useful for subpass rendering scenarios where it's
  ///         difficult to encode the current transform of the layer into the
  ///         Entity being rendered.
  void SetEffectTransform(const Matrix& effect_transform);

  /// @brief  Create an Entity that renders this filter's output.
  std::optional<Entity> GetEntity(
      const ContentContext& renderer,
      const Entity& entity,
      const std::optional<Rect>& coverage_hint) const;

  // |Contents|
  bool Render(const ContentContext& renderer,
              const Entity& entity,
              RenderPass& pass) const override;

  // |Contents|
  std::optional<Rect> GetCoverage(const Entity& entity) const override;

  // |Contents|
  std::optional<Snapshot> RenderToSnapshot(
      const ContentContext& renderer,
      const Entity& entity,
      std::optional<Rect> coverage_limit = std::nullopt,
      const std::optional<SamplerDescriptor>& sampler_descriptor = std::nullopt,
      bool msaa_enabled = true,
      int32_t mip_count = 1,
      const std::string& label = "Filter Snapshot") const override;

  virtual Matrix GetLocalTransform(const Matrix& parent_transform) const;

  Matrix GetTransform(const Matrix& parent_transform) const;

  /// @brief  Marks this filter chain as applying in a subpass scenario.
  ///
  ///         Subpasses render in screenspace, and this setting informs filters
  ///         that the current transform matrix of the entity is not stored
  ///         in the Entity transform matrix. Instead, the effect transform
  ///         is used in this case.
  virtual void SetRenderingMode(Entity::RenderingMode rendering_mode);

 private:
  /// @brief  Internal utility method for |GetLocalCoverage| that computes
  ///         the output coverage of this filter across the specified inputs,
  ///         ignoring the coverage hint.
  virtual std::optional<Rect> GetFilterCoverage(
      const FilterInput::Vector& inputs,
      const Entity& entity,
      const Matrix& effect_transform) const;

  /// @brief  Internal utility method for |GetSourceCoverage| that computes
  ///         the inverse effect of this transform on the specified output
  ///         coverage, ignoring the inputs which will be accommodated by
  ///         the caller.
  virtual std::optional<Rect> GetFilterSourceCoverage(
      const Matrix& effect_transform,
      const Rect& output_limit) const = 0;

  /// @brief  Converts zero or more filter inputs into a render instruction.
  virtual std::optional<Entity> RenderFilter(
      const FilterInput::Vector& inputs,
      const ContentContext& renderer,
      const Entity& entity,
      const Matrix& effect_transform,
      const Rect& coverage,
      const std::optional<Rect>& coverage_hint) const = 0;

  /// @brief  Internal utility method to compute the coverage of this
  ///         filter across its internally specified inputs and subject
  ///         to the coverage hint.
  ///
  ///         Uses |GetFilterCoverage|.
  std::optional<Rect> GetLocalCoverage(const Entity& local_entity) const;

  FilterInput::Vector inputs_;
  Matrix effect_transform_ = Matrix();

  FilterContents(const FilterContents&) = delete;

  FilterContents& operator=(const FilterContents&) = delete;
};

}  // namespace impeller

#endif  // FLUTTER_IMPELLER_ENTITY_CONTENTS_FILTERS_FILTER_CONTENTS_H_

// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <optional>
#include "impeller/entity/contents/filters/filter_contents.h"

namespace impeller {

class GaussianBlurFilterContents final : public FilterContents {
 public:
  GaussianBlurFilterContents(Scalar sigma = 0.0f);
  
  Scalar GetSigma() const { return sigma_; }

  // |FilterContents|
  std::optional<Rect> GetFilterSourceCoverage(
      const Matrix& effect_transform,
      const Rect& output_limit) const override;

  // |FilterContents|
  std::optional<Rect> GetFilterCoverage(
      const FilterInput::Vector& inputs,
      const Entity& entity,
      const Matrix& effect_transform) const override;

  static Scalar CalculateBlurRadius(Scalar sigma);

 private:
  // |FilterContents|
  std::optional<Entity> RenderFilter(
      const FilterInput::Vector& input_textures,
      const ContentContext& renderer,
      const Entity& entity,
      const Matrix& effect_transform,
      const Rect& coverage,
      const std::optional<Rect>& coverage_hint) const override;

  const Scalar sigma_ = 0.0;
};

} // namespace impeller

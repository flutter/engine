// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "impeller/entity/contents/filters/filter_contents.h"
#include "impeller/entity/contents/filters/inputs/filter_input.h"

namespace impeller {

class MPSGaussianBlurFilterContents final : public FilterContents {
 public:
  MPSGaussianBlurFilterContents();

  ~MPSGaussianBlurFilterContents() override;

  void SetSigma(Sigma sigma);

  void SetTileMode(Entity::TileMode tile_mode);

 private:
  // |FilterContents|
  std::optional<Entity> RenderFilter(
      const FilterInput::Vector& inputs,
      const ContentContext& renderer,
      const Entity& entity,
      const Matrix& effect_transform,
      const Rect& coverage,
      const std::optional<Rect>& coverage_hint) const override;

  Sigma sigma_;
  Entity::TileMode tile_mode_ = Entity::TileMode::kDecal;

  FML_DISALLOW_COPY_AND_ASSIGN(MPSGaussianBlurFilterContents);
};

}  // namespace impeller

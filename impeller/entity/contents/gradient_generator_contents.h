// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "flutter/fml/macros.h"
#include "impeller/entity/contents/contents.h"
#include "impeller/entity/entity.h"
#include "impeller/geometry/color.h"
#include "impeller/geometry/path.h"
#include "impeller/geometry/point.h"

namespace impeller {

/**
 * @brief A gradient texture generator designed for arbitrary count and
 * arbitrarily spaced gradient colors.
 */
class GradientGeneratorContents final : public Contents {
 public:
  GradientGeneratorContents();

  ~GradientGeneratorContents() override;

  // |Contents|
  bool Render(const ContentContext& renderer,
              const Entity& entity,
              RenderPass& pass) const override;

  // |Contents|
  std::optional<Rect> GetCoverage(const Entity& entity) const override;

  void SetColors(std::vector<Color> colors);

  void SetStops(std::vector<Scalar> stops);

  const std::vector<Color>& GetColors() const;

  const std::vector<Scalar>& GetStops() const;

 private:
  std::vector<Color> colors_;
  std::vector<Scalar> stops_;
  Scalar scale_ = 0.0;

  FML_DISALLOW_COPY_AND_ASSIGN(GradientGeneratorContents);
};

}  // namespace impeller

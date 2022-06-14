// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "impeller/entity/contents/contents.h"
#include "impeller/entity/contents/filters/filter_contents.h"
#include "impeller/geometry/color.h"

namespace impeller {

class Path;
class HostBuffer;
struct VertexBuffer;

class BoxShadowContents final : public Contents {
 public:
  BoxShadowContents();

  ~BoxShadowContents() override;

  void SetRect(std::optional<Rect> rect);

  void SetSigma(FilterContents::Sigma sigma);

  void SetColor(Color color);

  // |Contents|
  std::optional<Rect> GetCoverage(const Entity& entity) const override;

  // |Contents|
  bool Render(const ContentContext& renderer,
              const Entity& entity,
              RenderPass& pass) const override;

 private:
  std::optional<Rect> rect_;
  FilterContents::Sigma sigma_;

  Color color_;

  FML_DISALLOW_COPY_AND_ASSIGN(BoxShadowContents);
};

}  // namespace impeller

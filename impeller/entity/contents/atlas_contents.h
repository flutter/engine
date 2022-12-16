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
#include "impeller/entity/geometry.h"
#include "impeller/renderer/sampler_descriptor.h"

namespace impeller {

class AtlasContents final : public Contents {
 public:
  explicit AtlasContents();

  ~AtlasContents() override;

  void SetTexture(std::shared_ptr<Texture> texture);

  std::shared_ptr<Texture> GetTexture() const;

  void SetGeometry(std::unique_ptr<AtlasGeometry> geometry);

  void SetBlendMode(BlendMode blend_mode);

  void SetSamplerDescriptor(SamplerDescriptor desc);

  void SetAlpha(Scalar alpha);

  const SamplerDescriptor& GetSamplerDescriptor() const;

  // |Contents|
  std::optional<Rect> GetCoverage(const Entity& entity) const override;

  // |Contents|
  bool Render(const ContentContext& renderer,
              const Entity& entity,
              RenderPass& pass) const override;

 private:
  std::shared_ptr<Texture> texture_;
  std::unique_ptr<AtlasGeometry> geometry_;
  BlendMode blend_mode_;
  Scalar alpha_ = 1.0;
  SamplerDescriptor sampler_descriptor_ = {};

  FML_DISALLOW_COPY_AND_ASSIGN(AtlasContents);
};

}  // namespace impeller

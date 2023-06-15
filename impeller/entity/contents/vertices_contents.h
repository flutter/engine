// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "flutter/fml/macros.h"
#include "impeller/core/sampler_descriptor.h"
#include "impeller/entity/contents/contents.h"
#include "impeller/entity/entity.h"
#include "impeller/entity/geometry/geometry.h"
#include "impeller/entity/geometry/vertices_geometry.h"
#include "impeller/geometry/color.h"
#include "impeller/geometry/path.h"
#include "impeller/geometry/point.h"

namespace impeller {

namespace VerticesBindings {

static constexpr auto kInputPosition = ShaderStageIOSlot{
    // position
    "position",          // name
    0u,                  // attribute location
    0u,                  // attribute set
    0u,                  // attribute binding
    ShaderType::kFloat,  // type
    32u,                 // bit width of type
    2u,                  // vec size
    1u,                  // number of columns
    0u,                  // offset for interleaved layout
};

static constexpr auto kInputColor = ShaderStageIOSlot{
    // color
    "color",             // name
    1u,                  // attribute location
    0u,                  // attribute set
    1u,                  // attribute binding
    ShaderType::kFloat,  // type
    32u,                 // bit width of type
    4u,                  // vec size
    1u,                  // number of columns
    0u,                  // offset for interleaved layout
};

static constexpr std::array<const ShaderStageIOSlot*, 2> kAllShaderStageInputs =
    {
        &kInputPosition,  // position
        &kInputColor,     // color
};

static constexpr auto kInputBuffer = ShaderStageBufferLayout{
    sizeof(Vector2),  // stride for interleaved layout
    0u,               // attribute binding
};
static constexpr auto kColorBuffer = ShaderStageBufferLayout{
    sizeof(Color),  // stride for interleaved layout
    1u,             // attribute binding
};
static constexpr std::array<const ShaderStageBufferLayout*, 2>
    kNonInterleavedBufferLayout = {&kInputBuffer, &kColorBuffer};

}  // namespace VerticesBindings

class VerticesContents final : public Contents {
 public:
  VerticesContents();

  ~VerticesContents() override;

  void SetGeometry(std::shared_ptr<VerticesGeometry> geometry);

  void SetAlpha(Scalar alpha);

  void SetBlendMode(BlendMode blend_mode);

  void SetSourceContents(std::shared_ptr<Contents> contents);

  std::shared_ptr<VerticesGeometry> GetGeometry() const;

  const std::shared_ptr<Contents>& GetSourceContents() const;

  // |Contents|
  std::optional<Rect> GetCoverage(const Entity& entity) const override;

  // |Contents|
  bool Render(const ContentContext& renderer,
              const Entity& entity,
              RenderPass& pass) const override;

 private:
  Scalar alpha_;
  std::shared_ptr<VerticesGeometry> geometry_;
  BlendMode blend_mode_ = BlendMode::kSource;
  std::shared_ptr<Contents> src_contents_;

  FML_DISALLOW_COPY_AND_ASSIGN(VerticesContents);
};

class VerticesColorContents final : public Contents {
 public:
  explicit VerticesColorContents(const VerticesContents& parent);

  ~VerticesColorContents() override;

  // |Contents|
  std::optional<Rect> GetCoverage(const Entity& entity) const override;

  // |Contents|
  bool Render(const ContentContext& renderer,
              const Entity& entity,
              RenderPass& pass) const override;

  void SetAlpha(Scalar alpha);

 private:
  const VerticesContents& parent_;
  Scalar alpha_ = 1.0;

  FML_DISALLOW_COPY_AND_ASSIGN(VerticesColorContents);
};

class VerticesUVContents final : public Contents {
 public:
  explicit VerticesUVContents(const VerticesContents& parent);

  ~VerticesUVContents() override;

  // |Contents|
  std::optional<Rect> GetCoverage(const Entity& entity) const override;

  // |Contents|
  bool Render(const ContentContext& renderer,
              const Entity& entity,
              RenderPass& pass) const override;

  void SetAlpha(Scalar alpha);

 private:
  const VerticesContents& parent_;
  Scalar alpha_ = 1.0;

  FML_DISALLOW_COPY_AND_ASSIGN(VerticesUVContents);
};

}  // namespace impeller

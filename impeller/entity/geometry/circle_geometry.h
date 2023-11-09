// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "impeller/entity/geometry/geometry.h"

namespace impeller {

class CircleGeometry : public Geometry {
 public:
  CircleGeometry(std::vector<Point> points, Scalar radius, bool round);

  ~CircleGeometry();

  static size_t ComputeCircleDivisions(Scalar scaled_radius, bool round);

 private:
  // |Geometry|
  GeometryResult GetPositionBuffer(const ContentContext& renderer,
                                   const Entity& entity,
                                   RenderPass& pass) override;

  // |Geometry|
  GeometryResult GetPositionUVBuffer(Rect texture_coverage,
                                     Matrix effect_transform,
                                     const ContentContext& renderer,
                                     const Entity& entity,
                                     RenderPass& pass) override;

  // |Geometry|
  GeometryVertexType GetVertexType() const override;

  // |Geometry|
  std::optional<Rect> GetCoverage(const Matrix& transform) const override;

  GeometryResult GetPositionBufferGPU(
      const ContentContext& renderer,
      const Entity& entity,
      RenderPass& pass,
      std::optional<Rect> texture_coverage = std::nullopt,
      std::optional<Matrix> effect_transform = std::nullopt);

  VertexBuffer GetPositionBufferCPU(const ContentContext& renderer,
                                    const Entity& entity,
                                    RenderPass& pass);

  std::optional<VertexBufferBuilder<SolidFillVertexShader::PerVertexData>>
  GetPositionUVBufferCPU(const ContentContext& renderer,
                         const Entity& entity,
                         RenderPass& pass);

  std::vector<Point> points_;
  Scalar radius_;
  bool round_;

  CircleGeometry(const CircleGeometry&) = delete;

  CircleGeometry& operator=(const CircleGeometry&) = delete;
};

}  // namespace impeller

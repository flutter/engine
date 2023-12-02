// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "impeller/entity/geometry/geometry.h"

namespace impeller {

class VerticesBase : public Geometry {
 public:
  virtual GeometryResult GetPositionColorBuffer(const ContentContext& renderer,
                                                const Entity& entity,
                                                RenderPass& pass) const = 0;

  virtual bool HasVertexColors() const = 0;

  virtual bool HasTextureCoordinates() const = 0;

  virtual std::optional<Rect> GetTextureCoordinateCoverge() const = 0;
};

/// @brief A geometry that is created from a vertices object.
class VerticesGeometry final : public VerticesBase {
 public:
  enum class VertexMode {
    kTriangles,
    kTriangleStrip,
    kTriangleFan,
  };

  VerticesGeometry(std::vector<Point> vertices,
                   std::vector<uint16_t> indices,
                   std::vector<Point> texture_coordinates,
                   std::vector<Color> colors,
                   Rect bounds,
                   VerticesGeometry::VertexMode vertex_mode);

  ~VerticesGeometry() override = default;

  // |VerticesBase|
  GeometryResult GetPositionColorBuffer(const ContentContext& renderer,
                                        const Entity& entity,
                                        RenderPass& pass) const override;

  // |Geometry|
  GeometryResult GetPositionUVBuffer(Rect texture_coverage,
                                     Matrix effect_transform,
                                     const ContentContext& renderer,
                                     const Entity& entity,
                                     RenderPass& pass) const override;

  // |Geometry|
  GeometryResult GetPositionBuffer(const ContentContext& renderer,
                                   const Entity& entity,
                                   RenderPass& pass) const override;

  // |Geometry|
  std::optional<Rect> GetCoverage(const Matrix& transform) const override;

  // |Geometry|
  GeometryVertexType GetVertexType() const override;

  // |VerticesBase|
  bool HasVertexColors() const override;

  // |VerticesBase|
  bool HasTextureCoordinates() const override;

  // |VerticesBase|
  std::optional<Rect> GetTextureCoordinateCoverge() const override;

 private:
  void NormalizeIndices();

  PrimitiveType GetPrimitiveType() const;

  std::vector<Point> vertices_;
  std::vector<Color> colors_;
  std::vector<Point> texture_coordinates_;
  std::vector<uint16_t> indices_;
  Rect bounds_;
  VerticesGeometry::VertexMode vertex_mode_ =
      VerticesGeometry::VertexMode::kTriangles;
};

class UniqueVerticesWrapper final : public VerticesBase {
 public:
  explicit UniqueVerticesWrapper(std::unique_ptr<VerticesGeometry> geometry);

  ~UniqueVerticesWrapper() override = default;

  GeometryResult GetPositionColorBuffer(const ContentContext& renderer,
                                        const Entity& entity,
                                        RenderPass& pass) const override;

  // |Geometry|
  GeometryResult GetPositionUVBuffer(Rect texture_coverage,
                                     Matrix effect_transform,
                                     const ContentContext& renderer,
                                     const Entity& entity,
                                     RenderPass& pass) const override;

  // |Geometry|
  GeometryResult GetPositionBuffer(const ContentContext& renderer,
                                   const Entity& entity,
                                   RenderPass& pass) const override;

  // |Geometry|
  std::optional<Rect> GetCoverage(const Matrix& transform) const override;

  // |Geometry|
  GeometryVertexType GetVertexType() const override;

  // |VerticesBase|
  bool HasVertexColors() const override;

  // |VerticesBase|
  bool HasTextureCoordinates() const override;

  // |VerticesBase|
  std::optional<Rect> GetTextureCoordinateCoverge() const override;

 private:
  std::unique_ptr<VerticesGeometry> geometry_;
};

}  // namespace impeller

// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <optional>
#include <vector>

#include "flutter/display_list/display_list_vertices.h"

#include "impeller/entity/geometry.h"
#include "impeller/geometry/color.h"
#include "impeller/geometry/point.h"
#include "impeller/geometry/rect.h"

namespace impeller {

/// @brief A geometry that is created from a vertices object.
class VerticesGeometry : public Geometry {
 public:
  explicit VerticesGeometry(const flutter::DlVertices* vertices);

  ~VerticesGeometry();

  static std::unique_ptr<VerticesGeometry> MakeVertices(
      const flutter::DlVertices* vertices);

  GeometryResult GetPositionColorBuffer(const ContentContext& renderer,
                                        const Entity& entity,
                                        RenderPass& pass,
                                        Color paint_color,
                                        BlendMode blend_mode);

  GeometryResult GetPositionUVBuffer(const ContentContext& renderer,
                                     const Entity& entity,
                                     RenderPass& pass);

  // |Geometry|
  GeometryResult GetPositionBuffer(const ContentContext& renderer,
                                   const Entity& entity,
                                   RenderPass& pass) override;

  // |Geometry|
  std::optional<Rect> GetCoverage(const Matrix& transform) const override;

  // |Geometry|
  GeometryVertexType GetVertexType() const override;

 private:
  void NormalizeIndices();

  const flutter::DlVertices* vertices_;
  std::vector<uint16_t> normalized_indices_;

  FML_DISALLOW_COPY_AND_ASSIGN(VerticesGeometry);
};

}  // namespace impeller

// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <optional>
#include <vector>

#include "flutter/display_list/display_list.h"
#include "flutter/display_list/display_list_blend_mode.h"
#include "flutter/display_list/display_list_color.h"

#include "impeller/entity/geometry.h"
#include "impeller/geometry/color.h"
#include "impeller/geometry/point.h"
#include "impeller/geometry/rect.h"
#include "impeller/renderer/texture.h"

namespace impeller {

/// @brief A geometry that is created from a draw atlas call.
class DLAtlasGeometry : public AtlasGeometry {
 public:
  DLAtlasGeometry(const SkRSXform* xform,
                  const SkRect* tex,
                  const flutter::DlColor* colors,
                  size_t count,
                  std::optional<Rect> cull_rect,
                  ISize texture_size);

  ~DLAtlasGeometry();

  static std::unique_ptr<AtlasGeometry> MakeAtlas(
      const SkRSXform* xform,
      const SkRect* tex,
      const flutter::DlColor* colors,
      size_t count,
      std::optional<Rect> cull_rect,
      ISize texture_size);

  // |AtlasGeometry|
  GeometryResult GetPositionColorBuffer(const ContentContext& renderer,
                                        const Entity& entity,
                                        RenderPass& pass) override;

  // |AtlasGeometry|
  GeometryResult GetPositionUVBuffer(const ContentContext& renderer,
                                     const Entity& entity,
                                     RenderPass& pass) override;

  // |Geometry|
  GeometryResult GetPositionBuffer(const ContentContext& renderer,
                                   const Entity& entity,
                                   RenderPass& pass) override;

  // |Geometry|
  std::optional<Rect> GetCoverage(const Matrix& transform) const override;

  // |Geometry|
  GeometryVertexType GetVertexType() const override;

 private:
  void TransformPoints(const SkRSXform* xform);

  const flutter::DlColor* colors_;
  const SkRect* tex_;
  std::vector<std::array<Point, 4>> transformed_coords_;
  size_t count_;
  std::optional<Rect> cull_rect_;
  ISize texture_size_;

  FML_DISALLOW_COPY_AND_ASSIGN(DLAtlasGeometry);
};

}  // namespace impeller

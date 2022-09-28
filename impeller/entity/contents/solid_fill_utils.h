// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "impeller/geometry/vertices.h"
#include "impeller/renderer/vertex_buffer_builder.h"
#include "impeller/tessellator/tessellator.h"

namespace impeller {

template <typename PerVertexData>
VertexBuffer CreateSolidFillVertices(const std::optional<Path>& path,
                                     const std::optional<Vertices>& vertices,
                                     HostBuffer& buffer) {
  VertexBufferBuilder<PerVertexData> vtx_builder;

  if (path.has_value()) {
    auto tesselation_result = Tessellator{}.Tessellate(
        path->GetFillType(), path->CreatePolyline(),
        [&vtx_builder](auto point) { vtx_builder.AppendVertex({point}); });
    if (tesselation_result != Tessellator::Result::kSuccess) {
      return {};
    }

    return vtx_builder.CreateVertexBuffer(buffer);
  }

  const auto& positions = vertices->GetPositions();
  const auto& indices = vertices->GetIndices();
  vtx_builder.Reserve(positions.size());
  vtx_builder.ReserveIndices(indices.size());
  for (size_t i = 0; i < positions.size(); i++) {
    vtx_builder.AppendVertex({positions[i]});
  }
  for (size_t i = 0; i < indices.size(); i++) {
    vtx_builder.AppendIndex({indices[i]});
  }
  return vtx_builder.CreateVertexBuffer(buffer);
}

}  // namespace impeller

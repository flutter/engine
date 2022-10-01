// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/entity/contents/solid_fill_utils.h"
#include <iostream>
#include "impeller/geometry/path.h"
#include "impeller/renderer/device_buffer.h"
#include "impeller/renderer/host_buffer.h"
#include "impeller/tessellator/tessellator.h"

namespace impeller {

bool CreateSolidFillVertices(std::shared_ptr<Tessellator> tessellator,
                             const Path& path,
                             HostBuffer& buffer,
                             VertexBuffer* out_buffer) {
  auto tesselation_result = tessellator->TessellateBuilder(
      path.GetFillType(), path.CreatePolyline(),
      [&out_buffer, &buffer](const float* vertices, size_t vertices_count,
                             const uint16_t* indices, size_t indices_count) {
        out_buffer->vertex_buffer = buffer.Emplace(
            vertices, vertices_count * sizeof(float), alignof(float));
        out_buffer->index_buffer = buffer.Emplace(
            indices, indices_count * sizeof(uint16_t), alignof(uint16_t));
        out_buffer->index_count = indices_count;
        out_buffer->index_type = IndexType::k16bit;
        return true;
      });
  return tesselation_result == Tessellator::Result::kSuccess;
}

}  // namespace impeller

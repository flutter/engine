// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/command.h"

#include <utility>

#include "impeller/base/validation.h"
#include "impeller/core/formats.h"
#include "impeller/renderer/vertex_descriptor.h"

namespace impeller {

bool Command::BindVertices(VertexBuffer buffer) {
  if (buffer.index_type == IndexType::kUnknown) {
    VALIDATION_LOG << "Cannot bind vertex buffer with an unknown index type.";
    return false;
  }

  vertex_buffer = std::move(buffer);
  return true;
}

bool Command::BindResource(ShaderStage stage,
                           const ShaderUniformSlot& slot,
                           const ShaderMetadata& metadata,
                           BufferView view) {
  return DoBindResource(stage, slot, &metadata, std::move(view));
}

bool Command::BindResource(
    ShaderStage stage,
    const ShaderUniformSlot& slot,
    const std::shared_ptr<const ShaderMetadata>& metadata,
    BufferView view) {
  return DoBindResource(stage, slot, metadata, std::move(view));
}

template <class T>
bool Command::DoBindResource(ShaderStage stage,
                             const ShaderUniformSlot& slot,
                             const T metadata,
                             BufferView view) {
  FML_DCHECK(slot.ext_res_0 != VertexDescriptor::kReservedVertexBufferIndex);
  if (!view || bindings.buffer_offset >= kMaxBindings) {
    return false;
  }
  bindings.bound_buffers[bindings.buffer_offset++] =
      BoundBuffer{.stage = stage,
                  .slot = slot,
                  .view = BufferResource(metadata, std::move(view))};
  return true;
}

bool Command::BindResource(ShaderStage stage,
                           const SampledImageSlot& slot,
                           const ShaderMetadata& metadata,
                           std::shared_ptr<const Texture> texture,
                           std::shared_ptr<const Sampler> sampler) {
  if (!sampler || !sampler->IsValid() || !texture || !texture->IsValid() ||
      bindings.texture_offset >= kMaxBindings) {
    return false;
  }
  bindings.bound_textures[bindings.texture_offset++] = BoundTexture{
      .stage = stage,
      .slot = slot,
      .texture = {&metadata, std::move(texture)},
      .sampler = std::move(sampler),
  };
  return true;
}

}  // namespace impeller

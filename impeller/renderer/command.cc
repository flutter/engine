// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/command.h"

#include <utility>

#include "impeller/core/formats.h"
#include "impeller/renderer/vertex_descriptor.h"

namespace impeller {

void Command::BindVertices(VertexBuffer&& buffer) {
  vertex_buffer = buffer;
}

void Command::BindVertices(const VertexBuffer& buffer) {
  vertex_buffer = buffer;
}

const BufferView& Command::GetVertexBuffer() const {
  return vertex_buffer.vertex_buffer;
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
  switch (stage) {
    case ShaderStage::kVertex:
      vertex_bindings.buffers.emplace_back(BufferAndUniformSlot{
          .slot = slot, .view = BufferResource(metadata, std::move(view))});
      return true;
    case ShaderStage::kFragment:
      fragment_bindings.buffers.emplace_back(BufferAndUniformSlot{
          .slot = slot, .view = BufferResource(metadata, std::move(view))});
      return true;
    case ShaderStage::kCompute:
      return false;
  }
}

bool Command::BindResource(ShaderStage stage,
                           const ShaderUniformSlot& slot,
                           const ShaderMetadata& metadata,
                           std::shared_ptr<const Texture> texture,
                           std::shared_ptr<const Sampler> sampler) {
  switch (stage) {
    case ShaderStage::kVertex:
      vertex_bindings.sampled_images.emplace_back(TextureAndSampler{
          .slot = slot,
          .texture = {&metadata, std::move(texture)},
          .sampler = std::move(sampler),
      });
      return true;
    case ShaderStage::kFragment:
      fragment_bindings.sampled_images.emplace_back(TextureAndSampler{
          .slot = slot,
          .texture = {&metadata, std::move(texture)},
          .sampler = std::move(sampler),
      });
      return true;
    case ShaderStage::kCompute:
      return false;
  }
}

}  // namespace impeller

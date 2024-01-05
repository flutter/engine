// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/compute_command.h"

#include <utility>

#include "impeller/core/formats.h"
#include "impeller/core/shader_types.h"
#include "impeller/renderer/command.h"

namespace impeller {

bool ComputeCommand::BindResource(ShaderStage stage,
                                  const ShaderUniformSlot& slot,
                                  const ShaderMetadata& metadata,
                                  BufferView view) {
  if (!view || bindings.buffer_offset >= kMaxBindings) {
    return false;
  }

  bindings.bound_buffers[bindings.buffer_offset++] =
      BoundBuffer{.stage = ShaderStage::kCompute,
                  .slot = slot,
                  .view = {&metadata, std::move(view)}};
  return true;
}

bool ComputeCommand::BindResource(ShaderStage stage,
                                  const SampledImageSlot& slot,
                                  const ShaderMetadata& metadata,
                                  std::shared_ptr<const Texture> texture,
                                  std::shared_ptr<const Sampler> sampler) {
  if (!sampler || !sampler->IsValid() || !texture || !texture->IsValid() ||
      bindings.buffer_offset >= kMaxBindings) {
    return false;
  }

  bindings.bound_textures[bindings.texture_offset++] = BoundTexture{
      .stage = ShaderStage::kCompute,
      .slot = slot,
      .texture = {&metadata, std::move(texture)},
      .sampler = std::move(sampler),
  };

  return true;
}

}  // namespace impeller

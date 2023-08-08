// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/compute_command.h"

#include <utility>

#include "impeller/base/validation.h"
#include "impeller/core/formats.h"

namespace impeller {

bool ComputeCommand::BindResource(ShaderStage stage,
                                  const ShaderUniformSlot& slot,
                                  const ShaderMetadata& metadata,
                                  const BufferView& view) {
  if (stage != ShaderStage::kCompute) {
    VALIDATION_LOG << "Use Command for non-compute shader stages.";
    return false;
  }
  if (!view) {
    return false;
  }

  bindings.uniforms.insert(std::make_pair(slot.ext_res_0, slot));
  bindings.buffers.insert(
      std::make_pair(slot.ext_res_0, BufferResource{&metadata, view}));
  return true;
}

bool ComputeCommand::BindResource(
    ShaderStage stage,
    const SampledImageSlot& slot,
    const ShaderMetadata& metadata,
    const std::shared_ptr<const Texture>& texture) {
  if (stage != ShaderStage::kCompute) {
    VALIDATION_LOG << "Use Command for non-compute shader stages.";
    return false;
  }
  if (!texture || !texture->IsValid()) {
    return false;
  }

  if (!slot.HasTexture()) {
    return true;
  }

  bindings.textures.insert(
      std::make_pair(slot.texture_index, TextureResource{&metadata, texture}));
  return true;
}

bool ComputeCommand::BindResource(
    ShaderStage stage,
    const SampledImageSlot& slot,
    const ShaderMetadata& metadata,
    const std::shared_ptr<const Sampler>& sampler) {
  if (stage != ShaderStage::kCompute) {
    VALIDATION_LOG << "Use Command for non-compute shader stages.";
    return false;
  }
  if (!sampler || !sampler->IsValid()) {
    return false;
  }

  if (!slot.HasSampler()) {
    return true;
  }

  bindings.samplers.insert(
      std::make_pair(slot.sampler_index, SamplerResource{&metadata, sampler}));
  return true;
}

bool ComputeCommand::BindResource(
    ShaderStage stage,
    const SampledImageSlot& slot,
    const ShaderMetadata& metadata,
    const std::shared_ptr<const Texture>& texture,
    const std::shared_ptr<const Sampler>& sampler) {
  if (stage != ShaderStage::kCompute) {
    VALIDATION_LOG << "Use Command for non-compute shader stages.";
    return false;
  }
  return BindResource(stage, slot, metadata, texture) &&
         BindResource(stage, slot, metadata, sampler);
}

}  // namespace impeller

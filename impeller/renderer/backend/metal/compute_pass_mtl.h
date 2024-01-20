// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_RENDERER_BACKEND_METAL_COMPUTE_PASS_MTL_H_
#define FLUTTER_IMPELLER_RENDERER_BACKEND_METAL_COMPUTE_PASS_MTL_H_

#include <Metal/Metal.h>

#include "flutter/fml/macros.h"
#include "impeller/renderer/compute_pass.h"
#include "impeller/renderer/pipeline_descriptor.h"

namespace impeller {

//-----------------------------------------------------------------------------
/// @brief      Ensures that bindings on the pass are not redundantly set or
///             updated. Avoids making the driver do additional checks and makes
///             the frame insights during profiling and instrumentation not
///             complain about the same.
///
///             There should be no change to rendering if this caching was
///             absent.
///
struct ComputePassBindingsCache {
  explicit ComputePassBindingsCache() {}

  ComputePassBindingsCache(const ComputePassBindingsCache&) = delete;

  ComputePassBindingsCache(ComputePassBindingsCache&&) = delete;

  void SetComputePipelineState(id<MTLComputePipelineState> pipeline) {
    if (pipeline == pipeline_) {
      return;
    }
    pipeline_ = pipeline;
    [encoder_ setComputePipelineState:pipeline_];
  }

  id<MTLComputePipelineState> GetPipeline() const { return pipeline_; }

  void SetEncoder(id<MTLComputeCommandEncoder> encoder) { encoder_ = encoder; }

  void SetBuffer(uint64_t index, uint64_t offset, id<MTLBuffer> buffer) {
    auto found = buffers_.find(index);
    if (found != buffers_.end() && found->second.buffer == buffer) {
      // The right buffer is bound. Check if its offset needs to be updated.
      if (found->second.offset == offset) {
        // Buffer and its offset is identical. Nothing to do.
        return;
      }

      // Only the offset needs to be updated.
      found->second.offset = offset;

      [encoder_ setBufferOffset:offset atIndex:index];
      return;
    }

    buffers_[index] = {buffer, static_cast<size_t>(offset)};
    [encoder_ setBuffer:buffer offset:offset atIndex:index];
  }

  void SetTexture(uint64_t index, id<MTLTexture> texture) {
    auto found = textures_.find(index);
    if (found != textures_.end() && found->second == texture) {
      // Already bound.
      return;
    }
    textures_[index] = texture;
    [encoder_ setTexture:texture atIndex:index];
    return;
  }

  void SetSampler(uint64_t index, id<MTLSamplerState> sampler) {
    auto found = samplers_.find(index);
    if (found != samplers_.end() && found->second == sampler) {
      // Already bound.
      return;
    }
    samplers_[index] = sampler;
    [encoder_ setSamplerState:sampler atIndex:index];
    return;
  }

 private:
  struct BufferOffsetPair {
    id<MTLBuffer> buffer = nullptr;
    size_t offset = 0u;
  };
  using BufferMap = std::map<uint64_t, BufferOffsetPair>;
  using TextureMap = std::map<uint64_t, id<MTLTexture>>;
  using SamplerMap = std::map<uint64_t, id<MTLSamplerState>>;

  id<MTLComputeCommandEncoder> encoder_;
  id<MTLComputePipelineState> pipeline_ = nullptr;
  BufferMap buffers_;
  TextureMap textures_;
  SamplerMap samplers_;
};

class ComputePassMTL final : public ComputePass {
 public:
  // |RenderPass|
  ~ComputePassMTL() override;

 private:
  friend class CommandBufferMTL;

  id<MTLCommandBuffer> buffer_ = nil;
  id<MTLComputeCommandEncoder> encoder_ = nil;
  ComputePassBindingsCache pass_bindings_cache_ = ComputePassBindingsCache();
  bool is_valid_ = false;
  bool has_label_ = false;

  ComputePassMTL(std::shared_ptr<const Context> context,
                 id<MTLCommandBuffer> buffer);

  // |ComputePass|
  bool IsValid() const override;

  // |ComputePass|
  fml::Status Compute(const ISize& grid_size) override;

  // |ComputePass|
  void SetCommandLabel(std::string_view label) override;

  // |ComputePass|
  void OnSetLabel(const std::string& label) override;

  // |ComputePass|
  void SetPipeline(const std::shared_ptr<Pipeline<ComputePipelineDescriptor>>&
                       pipeline) override;

  // |ComputePass|
  bool BindResource(ShaderStage stage,
                    DescriptorType type,
                    const ShaderUniformSlot& slot,
                    const ShaderMetadata& metadata,
                    BufferView view) override;

  // |ComputePass|
  bool BindResource(ShaderStage stage,
                    DescriptorType type,
                    const SampledImageSlot& slot,
                    const ShaderMetadata& metadata,
                    std::shared_ptr<const Texture> texture,
                    std::shared_ptr<const Sampler> sampler) override;

  // |ComputePass|
  bool EncodeCommands() const override;

  ComputePassMTL(const ComputePassMTL&) = delete;

  ComputePassMTL& operator=(const ComputePassMTL&) = delete;
};

}  // namespace impeller

#endif  // FLUTTER_IMPELLER_RENDERER_BACKEND_METAL_COMPUTE_PASS_MTL_H_

// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_RENDERER_BACKEND_METAL_RENDER_PASS_MTL_H_
#define FLUTTER_IMPELLER_RENDERER_BACKEND_METAL_RENDER_PASS_MTL_H_

#include <Metal/Metal.h>

#include "flutter/fml/macros.h"
#include "impeller/renderer/render_pass.h"
#include "impeller/renderer/render_target.h"

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
struct PassBindingsCache {
  explicit PassBindingsCache() {}

  ~PassBindingsCache() = default;

  PassBindingsCache(const PassBindingsCache&) = delete;

  PassBindingsCache(PassBindingsCache&&) = delete;

  void SetEncoder(id<MTLRenderCommandEncoder> encoder) { encoder_ = encoder; }

  void SetRenderPipelineState(id<MTLRenderPipelineState> pipeline) {
    if (pipeline == pipeline_) {
      return;
    }
    pipeline_ = pipeline;
    [encoder_ setRenderPipelineState:pipeline_];
  }

  void SetDepthStencilState(id<MTLDepthStencilState> depth_stencil) {
    if (depth_stencil_ == depth_stencil) {
      return;
    }
    depth_stencil_ = depth_stencil;
    [encoder_ setDepthStencilState:depth_stencil_];
  }

  bool SetBuffer(ShaderStage stage,
                 uint64_t index,
                 uint64_t offset,
                 id<MTLBuffer> buffer) {
    auto& buffers_map = buffers_[stage];
    auto found = buffers_map.find(index);
    if (found != buffers_map.end() && found->second.buffer == buffer) {
      // The right buffer is bound. Check if its offset needs to be updated.
      if (found->second.offset == offset) {
        // Buffer and its offset is identical. Nothing to do.
        return true;
      }

      // Only the offset needs to be updated.
      found->second.offset = offset;

      switch (stage) {
        case ShaderStage::kVertex:
          [encoder_ setVertexBufferOffset:offset atIndex:index];
          return true;
        case ShaderStage::kFragment:
          [encoder_ setFragmentBufferOffset:offset atIndex:index];
          return true;
        default:
          VALIDATION_LOG << "Cannot update buffer offset of an unknown stage.";
          return false;
      }
      return true;
    }
    buffers_map[index] = {buffer, static_cast<size_t>(offset)};
    switch (stage) {
      case ShaderStage::kVertex:
        [encoder_ setVertexBuffer:buffer offset:offset atIndex:index];
        return true;
      case ShaderStage::kFragment:
        [encoder_ setFragmentBuffer:buffer offset:offset atIndex:index];
        return true;
      default:
        VALIDATION_LOG << "Cannot bind buffer to unknown shader stage.";
        return false;
    }
    return false;
  }

  bool SetTexture(ShaderStage stage, uint64_t index, id<MTLTexture> texture) {
    auto& texture_map = textures_[stage];
    auto found = texture_map.find(index);
    if (found != texture_map.end() && found->second == texture) {
      // Already bound.
      return true;
    }
    texture_map[index] = texture;
    switch (stage) {
      case ShaderStage::kVertex:
        [encoder_ setVertexTexture:texture atIndex:index];
        return true;
      case ShaderStage::kFragment:
        [encoder_ setFragmentTexture:texture atIndex:index];
        return true;
      default:
        VALIDATION_LOG << "Cannot bind buffer to unknown shader stage.";
        return false;
    }
    return false;
  }

  bool SetSampler(ShaderStage stage,
                  uint64_t index,
                  id<MTLSamplerState> sampler) {
    auto& sampler_map = samplers_[stage];
    auto found = sampler_map.find(index);
    if (found != sampler_map.end() && found->second == sampler) {
      // Already bound.
      return true;
    }
    sampler_map[index] = sampler;
    switch (stage) {
      case ShaderStage::kVertex:
        [encoder_ setVertexSamplerState:sampler atIndex:index];
        return true;
      case ShaderStage::kFragment:
        [encoder_ setFragmentSamplerState:sampler atIndex:index];
        return true;
      default:
        VALIDATION_LOG << "Cannot bind buffer to unknown shader stage.";
        return false;
    }
    return false;
  }

  void SetViewport(const Viewport& viewport) {
    if (viewport_.has_value() && viewport_.value() == viewport) {
      return;
    }
    [encoder_ setViewport:MTLViewport{
                              .originX = viewport.rect.GetX(),
                              .originY = viewport.rect.GetY(),
                              .width = viewport.rect.GetWidth(),
                              .height = viewport.rect.GetHeight(),
                              .znear = viewport.depth_range.z_near,
                              .zfar = viewport.depth_range.z_far,
                          }];
    viewport_ = viewport;
  }

  void SetScissor(const IRect& scissor) {
    if (scissor_.has_value() && scissor_.value() == scissor) {
      return;
    }
    [encoder_
        setScissorRect:MTLScissorRect{
                           .x = static_cast<NSUInteger>(scissor.GetX()),
                           .y = static_cast<NSUInteger>(scissor.GetY()),
                           .width = static_cast<NSUInteger>(scissor.GetWidth()),
                           .height =
                               static_cast<NSUInteger>(scissor.GetHeight()),
                       }];
    scissor_ = scissor;
  }

 private:
  struct BufferOffsetPair {
    id<MTLBuffer> buffer = nullptr;
    size_t offset = 0u;
  };
  using BufferMap = std::map<uint64_t, BufferOffsetPair>;
  using TextureMap = std::map<uint64_t, id<MTLTexture>>;
  using SamplerMap = std::map<uint64_t, id<MTLSamplerState>>;

  id<MTLRenderCommandEncoder> encoder_;
  id<MTLRenderPipelineState> pipeline_ = nullptr;
  id<MTLDepthStencilState> depth_stencil_ = nullptr;
  std::map<ShaderStage, BufferMap> buffers_;
  std::map<ShaderStage, TextureMap> textures_;
  std::map<ShaderStage, SamplerMap> samplers_;
  std::optional<Viewport> viewport_;
  std::optional<IRect> scissor_;
};

class RenderPassMTL final : public RenderPass {
 public:
  // |RenderPass|
  ~RenderPassMTL() override;

 private:
  friend class CommandBufferMTL;

  id<MTLCommandBuffer> buffer_ = nil;
  id<MTLRenderCommandEncoder> encoder_ = nil;
  MTLRenderPassDescriptor* desc_ = nil;
  std::string label_;
  bool is_metal_trace_active_ = false;
  bool is_valid_ = false;

  PassBindingsCache pass_bindings_;

  // Per-command state
  size_t instance_count_ = 1u;
  size_t base_vertex_ = 0u;
  size_t vertex_count_ = 0u;
  bool has_valid_pipeline_ = false;
  bool has_label_ = false;
  BufferView index_buffer_;
  PrimitiveType primitive_type_;
  MTLIndexType index_type_;

  RenderPassMTL(std::shared_ptr<const Context> context,
                const RenderTarget& target,
                id<MTLCommandBuffer> buffer);

  // |RenderPass|
  void ReserveCommands(size_t command_count) override {}

  // |RenderPass|
  bool IsValid() const override;

  // |RenderPass|
  void OnSetLabel(std::string label) override;

  // |RenderPass|
  bool OnEncodeCommands(const Context& context) const override;

  // |RenderPass|
  void SetPipeline(
      const std::shared_ptr<Pipeline<PipelineDescriptor>>& pipeline) override;

  // |RenderPass|
  void SetCommandLabel(std::string_view label) override;

  // |RenderPass|
  void SetStencilReference(uint32_t value) override;

  // |RenderPass|
  void SetBaseVertex(uint64_t value) override;

  // |RenderPass|
  void SetViewport(Viewport viewport) override;

  // |RenderPass|
  void SetScissor(IRect scissor) override;

  // |RenderPass|
  void SetInstanceCount(size_t count) override;

  // |RenderPass|
  bool SetVertexBuffer(VertexBuffer buffer) override;

  // |RenderPass|
  fml::Status Draw() override;

  // |RenderPass|
  bool BindResource(ShaderStage stage,
                    DescriptorType type,
                    const ShaderUniformSlot& slot,
                    const ShaderMetadata& metadata,
                    BufferView view) override;

  // |RenderPass|
  bool BindResource(ShaderStage stage,
                    DescriptorType type,
                    const ShaderUniformSlot& slot,
                    const std::shared_ptr<const ShaderMetadata>& metadata,
                    BufferView view) override;

  // |RenderPass|
  bool BindResource(ShaderStage stage,
                    DescriptorType type,
                    const SampledImageSlot& slot,
                    const ShaderMetadata& metadata,
                    std::shared_ptr<const Texture> texture,
                    std::shared_ptr<const Sampler> sampler) override;

  RenderPassMTL(const RenderPassMTL&) = delete;

  RenderPassMTL& operator=(const RenderPassMTL&) = delete;
};

}  // namespace impeller

#endif  // FLUTTER_IMPELLER_RENDERER_BACKEND_METAL_RENDER_PASS_MTL_H_

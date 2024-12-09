// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_RENDERER_BACKEND_GLES_RENDER_PASS_GLES3_H_
#define FLUTTER_IMPELLER_RENDERER_BACKEND_GLES_RENDER_PASS_GLES3_H_

#include <cstdint>
#include <memory>

#include "flutter/impeller/renderer/backend/gles/reactor_gles.h"
#include "flutter/impeller/renderer/render_pass.h"
#include "impeller/core/buffer_view.h"
#include "impeller/core/formats.h"
#include "impeller/renderer/backend/gles/pipeline_gles.h"
#include "impeller/renderer/command.h"

namespace impeller {

class RenderPassGLES3 final
    : public RenderPass,
      public std::enable_shared_from_this<RenderPassGLES3> {
 public:
  // |RenderPass|
  ~RenderPassGLES3() override;

  static void ResetGLState(const ProcTableGLES& gl);

 private:
  friend class CommandBufferGLES;

  //------------------------------------------------------------------------------
  /// @brief      Encapsulates data that will be needed in the reactor for the
  ///             encoding of commands for this render pass.
  ///
  struct RenderPassData {
    Viewport viewport;

    Color clear_color;
    uint32_t clear_stencil = 0u;
    Scalar clear_depth = 1.0;

    std::shared_ptr<Texture> color_attachment;
    std::shared_ptr<Texture> depth_attachment;
    std::shared_ptr<Texture> stencil_attachment;

    bool clear_color_attachment = true;
    bool clear_depth_attachment = true;
    bool clear_stencil_attachment = true;

    bool discard_color_attachment = true;
    bool discard_depth_attachment = true;
    bool discard_stencil_attachment = true;

    std::string label;
  };

  std::shared_ptr<ReactorGLES> reactor_;
  std::string label_;
  RenderPassData render_pass_data_;
  uint32_t stencil_reference_ = 0;
  bool is_valid_ = false;

  // Per command state.
  std::array<TextureAndSampler, 16> bound_textures_;
  size_t bound_texture_index_ = 0;
  std::array<BufferResource, 16> bound_buffers_;
  size_t bound_buffers_index_ = 0;
  std::array<BufferView, 16> vertex_buffers_;
  size_t vertex_buffer_count_ = 0;
  uint64_t base_vertex_ = 0;
  size_t element_count_ = 0;
  size_t instance_count_ = 0;
  BufferView index_buffer_ = {};
  IndexType index_type_ = IndexType::kNone;
  const PipelineGLES* pipeline_ = nullptr;

  // cached state.
  GLint current_fbo_ = GL_NONE;
  GLint prev_framebuffer_ = GL_NONE;
  std::optional<ISize> prev_size_ = std::nullopt;
  std::optional<Viewport> prev_viewport_ = std::nullopt;

  RenderPassGLES3(std::shared_ptr<const Context> context,
                  const RenderTarget& target,
                  std::shared_ptr<ReactorGLES> reactor);

  // |RenderPass|
  bool IsValid() const override;

  // |RenderPass|
  void OnSetLabel(std::string_view label) override;

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
  void SetElementCount(size_t count) override;

  // |RenderPass|
  void SetInstanceCount(size_t count) override;

  // |RenderPass|
  bool SetVertexBuffer(BufferView vertex_buffers[],
                       size_t vertex_buffer_count) override;

  // |RenderPass|
  bool SetIndexBuffer(BufferView index_buffer, IndexType index_type) override;

  // |RenderPass|
  fml::Status Draw() override;

  // |ResourceBinder|
  bool BindResource(ShaderStage stage,
                    DescriptorType type,
                    const ShaderUniformSlot& slot,
                    const ShaderMetadata* metadata,
                    BufferView view) override;

  // |ResourceBinder|
  bool BindResource(ShaderStage stage,
                    DescriptorType type,
                    const SampledImageSlot& slot,
                    const ShaderMetadata* metadata,
                    std::shared_ptr<const Texture> texture,
                    const std::unique_ptr<const Sampler>& sampler) override;

  // |RenderPass|
  bool BindDynamicResource(
      ShaderStage stage,
      DescriptorType type,
      const SampledImageSlot& slot,
      std::unique_ptr<ShaderMetadata> metadata,
      std::shared_ptr<const Texture> texture,
      const std::unique_ptr<const Sampler>& sampler) override;

  // |RenderPass|
  bool BindDynamicResource(ShaderStage stage,
                           DescriptorType type,
                           const ShaderUniformSlot& slot,
                           std::unique_ptr<ShaderMetadata> metadata,
                           BufferView view) override;

  RenderPassGLES3(const RenderPassGLES3&) = delete;

  RenderPassGLES3& operator=(const RenderPassGLES3&) = delete;
};

}  // namespace impeller

#endif  // FLUTTER_IMPELLER_RENDERER_BACKEND_GLES_RENDER_PASS_GLES3_H_

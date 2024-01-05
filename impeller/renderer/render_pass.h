// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_RENDERER_RENDER_PASS_H_
#define FLUTTER_IMPELLER_RENDERER_RENDER_PASS_H_

#include <string>

#include "impeller/core/formats.h"
#include "impeller/renderer/command.h"
#include "impeller/renderer/command_buffer.h"
#include "impeller/renderer/render_target.h"

namespace impeller {

class HostBuffer;
class Allocator;

/// @brief A Command object with the bindings recorded elsewhere.
struct BoundCommand {
  std::shared_ptr<Pipeline<PipelineDescriptor>> pipeline;

  /// Offset and length into a per-render pass binding vector.
  size_t buffer_offset;
  size_t buffer_length;
  size_t texture_offset;
  size_t texture_length;

#ifdef IMPELLER_DEBUG
  std::string label;
#endif  // IMPELLER_DEBUG
  uint32_t stencil_reference = 0u;
  uint64_t base_vertex = 0u;
  std::optional<Viewport> viewport;
  std::optional<IRect> scissor;
  size_t instance_count = 1u;
  VertexBuffer vertex_buffer;
};

//------------------------------------------------------------------------------
/// @brief      Render passes encode render commands directed as one specific
///             render target into an underlying command buffer.
///
///             Render passes can be obtained from the command buffer in which
///             the pass is meant to encode commands into.
///
/// @see        `CommandBuffer`
///
class RenderPass {
 public:
  virtual ~RenderPass();

  const std::weak_ptr<const Context>& GetContext() const;

  const RenderTarget& GetRenderTarget() const;

  ISize GetRenderTargetSize() const;

  const Matrix& GetOrthographicTransform() const;

  virtual bool IsValid() const = 0;

  void SetLabel(std::string label);

  /// @brief Reserve [command_count] commands in the HAL command buffer.
  ///
  /// Note: this is not the native command buffer.
  void ReserveCommands(size_t command_count) {
    commands_.reserve(command_count);
  }

  HostBuffer& GetTransientsBuffer();

  //----------------------------------------------------------------------------
  /// @brief      Record a command for subsequent encoding to the underlying
  ///             command buffer. No work is encoded into the command buffer at
  ///             this time.
  ///
  /// @param[in]  command  The command
  ///
  /// @return     If the command was valid for subsequent commitment.
  ///
  bool AddCommand(Command&& command);

  //----------------------------------------------------------------------------
  /// @brief      Encode the recorded commands to the underlying command buffer.
  ///
  /// @return     If the commands were encoded to the underlying command
  ///             buffer.
  ///
  bool EncodeCommands() const;

  //----------------------------------------------------------------------------
  /// @brief      Accessor for the current Commands.
  ///
  /// @details    Visible for testing.
  ///
  const std::vector<BoundCommand>& GetCommands() const { return commands_; }

  //----------------------------------------------------------------------------
  /// @brief      Accessor for the current bound buffers.
  ///
  /// @details    Visible for testing.
  ///
  const std::vector<BoundBuffer>& GetBoundBuffers() const {
    return bound_buffers_;
  }

  //----------------------------------------------------------------------------
  /// @brief      Accessor for the current bound textures.
  ///
  /// @details    Visible for testing.
  ///
  const std::vector<BoundTexture>& GetBoundTextures() const {
    return bound_textures_;
  }

  //----------------------------------------------------------------------------
  /// @brief      The sample count of the attached render target.
  SampleCount GetSampleCount() const;

  //----------------------------------------------------------------------------
  /// @brief      The pixel format of the attached render target.
  PixelFormat GetRenderTargetPixelFormat() const;

  //----------------------------------------------------------------------------
  /// @brief      Whether the render target has an stencil attachment.
  bool HasStencilAttachment() const;

 protected:
  const std::weak_ptr<const Context> context_;
  // The following properties: sample_count, pixel_format,
  // has_stencil_attachment, and render_target_size are cached on the
  // RenderTarget to speed up numerous lookups during rendering. This is safe as
  // the RenderTarget itself is copied into the RenderTarget and only exposed as
  // a const reference.
  const SampleCount sample_count_;
  const PixelFormat pixel_format_;
  const bool has_stencil_attachment_;
  const ISize render_target_size_;
  const RenderTarget render_target_;
  std::shared_ptr<HostBuffer> transients_buffer_;
  std::vector<BoundCommand> commands_;
  std::vector<BoundBuffer> bound_buffers_;
  std::vector<BoundTexture> bound_textures_;
  const Matrix orthographic_;

  RenderPass(std::weak_ptr<const Context> context, const RenderTarget& target);

  virtual void OnSetLabel(std::string label) = 0;

  virtual bool OnEncodeCommands(const Context& context) const = 0;

 private:
  RenderPass(const RenderPass&) = delete;

  RenderPass& operator=(const RenderPass&) = delete;
};

}  // namespace impeller

#endif  // FLUTTER_IMPELLER_RENDERER_RENDER_PASS_H_

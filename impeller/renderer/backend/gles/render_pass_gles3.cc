// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/gles/render_pass_gles3.h"

#include <cstdint>

#include "fml/closure.h"
#include "fml/logging.h"
#include "fml/status.h"
#include "impeller/base/validation.h"
#include "impeller/geometry/size.h"
#include "impeller/renderer/backend/gles/buffer_bindings_gles.h"
#include "impeller/renderer/backend/gles/context_gles.h"
#include "impeller/renderer/backend/gles/device_buffer_gles.h"
#include "impeller/renderer/backend/gles/formats_gles.h"
#include "impeller/renderer/backend/gles/pipeline_gles.h"
#include "impeller/renderer/backend/gles/texture_gles.h"
#include "impeller/renderer/command.h"

namespace impeller {

RenderPassGLES3::RenderPassGLES3(std::shared_ptr<const Context> context,
                                 const RenderTarget& target,
                                 std::shared_ptr<ReactorGLES> reactor)
    : RenderPass(std::move(context), target),
      reactor_(std::move(reactor)),
      is_valid_(reactor_ && reactor_->IsValid()) {
  const auto& render_target = GetRenderTarget();
  if (!render_target.HasColorAttachment(0u)) {
    return;
  }
  const auto& gl = reactor_->GetProcTable();
  const ContextGLES& context_gles = ContextGLES::Cast(*GetContext());

  const auto& color0 = render_target.GetColorAttachment(0);
  const auto& depth0 = render_target.GetDepthAttachment();
  const auto& stencil0 = render_target.GetStencilAttachment();

  render_pass_data_.label = label_;
  render_pass_data_.viewport.rect = Rect::MakeSize(GetRenderTargetSize());

  //----------------------------------------------------------------------------
  /// Setup color data.
  ///
  render_pass_data_.color_attachment = color0.texture;
  render_pass_data_.clear_color = color0.clear_color;
  render_pass_data_.clear_color_attachment =
      CanClearAttachment(color0.load_action);
  render_pass_data_.discard_color_attachment =
      CanDiscardAttachmentWhenDone(color0.store_action);

  // When we are using EXT_multisampled_render_to_texture, it is implicitly
  // resolved when we bind the texture to the framebuffer. We don't need to
  // discard the attachment when we are done.
  if (color0.resolve_texture) {
    FML_DCHECK(context->GetCapabilities()->SupportsImplicitResolvingMSAA());
    render_pass_data_.discard_color_attachment = false;
  }

  //----------------------------------------------------------------------------
  /// Setup depth data.
  ///
  if (depth0.has_value()) {
    render_pass_data_.depth_attachment = depth0->texture;
    render_pass_data_.clear_depth = depth0->clear_depth;
    render_pass_data_.clear_depth_attachment =
        CanClearAttachment(depth0->load_action);
    render_pass_data_.discard_depth_attachment =
        CanDiscardAttachmentWhenDone(depth0->store_action);
  }

  //----------------------------------------------------------------------------
  /// Setup stencil data.
  ///
  if (stencil0.has_value()) {
    render_pass_data_.stencil_attachment = stencil0->texture;
    render_pass_data_.clear_stencil = stencil0->clear_stencil;
    render_pass_data_.clear_stencil_attachment =
        CanClearAttachment(stencil0->load_action);
    render_pass_data_.discard_stencil_attachment =
        CanDiscardAttachmentWhenDone(stencil0->store_action);
  }

  GLuint fbo = GL_NONE;
  TextureGLES& color_gles =
      TextureGLES::Cast(*render_pass_data_.color_attachment);
  const bool is_default_fbo = color_gles.IsWrapped();

  if (is_default_fbo) {
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    gl.BindFramebuffer(GL_FRAMEBUFFER, *color_gles.GetFBO());
  } else {
    // Create and bind an offscreen FBO.
    auto cached_fbo = color_gles.GetCachedFBO();
    if (cached_fbo != GL_NONE) {
      fbo = cached_fbo;
      gl.BindFramebuffer(GL_FRAMEBUFFER, fbo);
    } else {
      gl.GenFramebuffers(1u, &fbo);
      color_gles.SetCachedFBO(fbo);
      gl.BindFramebuffer(GL_FRAMEBUFFER, fbo);

      if (!color_gles.SetAsFramebufferAttachment(
              GL_FRAMEBUFFER, TextureGLES::AttachmentType::kColor0)) {
        return;
      }

      if (auto depth =
              TextureGLES::Cast(render_pass_data_.depth_attachment.get())) {
        if (!depth->SetAsFramebufferAttachment(
                GL_FRAMEBUFFER, TextureGLES::AttachmentType::kDepth)) {
          return;
        }
      }
      if (auto stencil =
              TextureGLES::Cast(render_pass_data_.stencil_attachment.get())) {
        if (!stencil->SetAsFramebufferAttachment(
                GL_FRAMEBUFFER, TextureGLES::AttachmentType::kStencil)) {
          return;
        }
      }

      auto status = gl.CheckFramebufferStatus(GL_FRAMEBUFFER);
      if (status != GL_FRAMEBUFFER_COMPLETE) {
        VALIDATION_LOG << "Could not create a complete frambuffer: "
                       << DebugToFramebufferError(status);
        return;
      }
    }
  }
  current_fbo_ = fbo;

  gl.ClearColor(render_pass_data_.clear_color.red,    // red
                render_pass_data_.clear_color.green,  // green
                render_pass_data_.clear_color.blue,   // blue
                render_pass_data_.clear_color.alpha   // alpha
  );
  if (render_pass_data_.depth_attachment) {
    if (gl.DepthRangef.IsAvailable()) {
      gl.ClearDepthf(render_pass_data_.clear_depth);
    } else {
      gl.ClearDepth(render_pass_data_.clear_depth);
    }
  }
  if (render_pass_data_.stencil_attachment) {
    gl.ClearStencil(render_pass_data_.clear_stencil);
  }

  GLenum clear_bits = 0u;
  if (render_pass_data_.clear_color_attachment) {
    clear_bits |= GL_COLOR_BUFFER_BIT;
  }
  if (render_pass_data_.clear_depth_attachment) {
    clear_bits |= GL_DEPTH_BUFFER_BIT;
  }
  if (render_pass_data_.clear_stencil_attachment) {
    clear_bits |= GL_STENCIL_BUFFER_BIT;
  }

  RenderPassGLES3::ResetGLState(gl);

  gl.Clear(clear_bits);

  // Both the viewport and scissor are specified in framebuffer coordinates.
  // Impeller's framebuffer coordinate system is top left origin, but OpenGL's
  // is bottom left origin, so we convert the coordinates here.
  auto target_size = render_pass_data_.color_attachment->GetSize();

  //--------------------------------------------------------------------------
  /// Setup the viewport.
  ///
  const Viewport& viewport = render_pass_data_.viewport;
  gl.Viewport(viewport.rect.GetX(),  // x
              target_size.height - viewport.rect.GetY() -
                  viewport.rect.GetHeight(),  // y
              viewport.rect.GetWidth(),       // width
              viewport.rect.GetHeight()       // height
  );
  if (render_pass_data_.depth_attachment) {
    if (gl.DepthRangef.IsAvailable()) {
      gl.DepthRangef(viewport.depth_range.z_near, viewport.depth_range.z_far);
    } else {
      gl.DepthRange(viewport.depth_range.z_near, viewport.depth_range.z_far);
    }
  }

  // Cache prior global rendering state.
  GlobalStateGLES* global_state = context_gles.GetGlobalState();
  prev_framebuffer_ = global_state->fbo_id;
  prev_size_ = global_state->target_size;
  prev_viewport_ = global_state->viewport;

  global_state->fbo_id = fbo;
  global_state->target_size = target_size;
  global_state->viewport = viewport;
}

// |RenderPass|
RenderPassGLES3::~RenderPassGLES3() = default;

// |RenderPass|
bool RenderPassGLES3::IsValid() const {
  return is_valid_;
}

// |RenderPass|
void RenderPassGLES3::OnSetLabel(std::string_view label) {
  label_ = label;
}

void RenderPassGLES3::ResetGLState(const ProcTableGLES& gl) {
  gl.Disable(GL_SCISSOR_TEST);
  gl.Disable(GL_DEPTH_TEST);
  gl.Disable(GL_STENCIL_TEST);
  gl.Disable(GL_CULL_FACE);
  gl.Disable(GL_BLEND);
  gl.Disable(GL_DITHER);
  gl.ColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  gl.DepthMask(GL_TRUE);
  gl.StencilMaskSeparate(GL_FRONT, 0xFFFFFFFF);
  gl.StencilMaskSeparate(GL_BACK, 0xFFFFFFFF);
}

// |RenderPass|
void RenderPassGLES3::SetPipeline(PipelineRef pipeline) {
  pipeline_ = pipeline;
}

// |RenderPass|
void RenderPassGLES3::SetCommandLabel(std::string_view label) {}

// |RenderPass|
void RenderPassGLES3::SetStencilReference(uint32_t value) {
  if (stencil_reference_ == value) {
    return;
  }
  stencil_reference_ = value;
}

// |RenderPass|
void RenderPassGLES3::SetBaseVertex(uint64_t value) {
  base_vertex_ = value;
}

// |RenderPass|
void RenderPassGLES3::SetViewport(Viewport viewport) {
  const auto& gl = reactor_->GetProcTable();
  ISize target_size = render_pass_data_.color_attachment->GetSize();
  gl.Viewport(viewport.rect.GetX(),  // x
              target_size.height - viewport.rect.GetY() -
                  viewport.rect.GetHeight(),  // y
              viewport.rect.GetWidth(),       // width
              viewport.rect.GetHeight()       // height
  );
}

// |RenderPass|
void RenderPassGLES3::SetScissor(IRect scissor) {
  const auto& gl = reactor_->GetProcTable();
  ISize target_size = render_pass_data_.color_attachment->GetSize();
  gl.Enable(GL_SCISSOR_TEST);
  gl.Scissor(scissor.GetX(),                                             // x
             target_size.height - scissor.GetY() - scissor.GetHeight(),  // y
             scissor.GetWidth(),  // width
             scissor.GetHeight()  // height
  );
}

// |RenderPass|
void RenderPassGLES3::SetElementCount(size_t count) {
  element_count_ = count;
}

// |RenderPass|
void RenderPassGLES3::SetInstanceCount(size_t count) {
  instance_count_ = count;
}

// |RenderPass|
bool RenderPassGLES3::SetVertexBuffer(BufferView vertex_buffers[],
                                      size_t vertex_buffer_count) {
  vertex_buffer_count_ = vertex_buffer_count;
  for (size_t i = 0; i < vertex_buffer_count; i++) {
    vertex_buffers_[i] = vertex_buffers[i];
  }
  return true;
}

// |RenderPass|
bool RenderPassGLES3::SetIndexBuffer(BufferView index_buffer,
                                     IndexType index_type) {
  index_type_ = index_type;
  index_buffer_ = std::move(index_buffer);
  return true;
}

// |RenderPass|
fml::Status RenderPassGLES3::Draw() {
  const auto& gl = reactor_->GetProcTable();

  fml::ScopedCleanupClosure cleanup([&] {
    bound_texture_index_ = 0;
    bound_buffers_index_ = 0;
    base_vertex_ = 0;
    element_count_ = 0;
    instance_count_ = 0;
    index_buffer_ = {};
    index_type_ = IndexType::kNone;
    vertex_buffer_count_ = 0;
  });

  if (!pipeline_) {
    return fml::Status(fml::StatusCode::kInternal, "");
  }

  const PipelineGLES& pipeline = PipelineGLES::Cast(*pipeline_);
  const auto* color_attachment =
      pipeline.GetDescriptor().GetLegacyCompatibleColorAttachment();
  if (!color_attachment) {
    VALIDATION_LOG
        << "Color attachment is too complicated for a legacy renderer.";
    return fml::Status(fml::StatusCode::kInternal, "");
  }

  //--------------------------------------------------------------------------
  /// Configure blending.
  ///
  ConfigureBlending(gl, color_attachment);

  //--------------------------------------------------------------------------
  /// Setup stencil.
  ///
  ConfigureStencil(gl, pipeline.GetDescriptor(), stencil_reference_);

  //--------------------------------------------------------------------------
  /// Configure depth.
  ///
  if (auto depth =
          pipeline.GetDescriptor().GetDepthStencilAttachmentDescriptor();
      depth.has_value()) {
    gl.Enable(GL_DEPTH_TEST);
    gl.DepthFunc(ToCompareFunction(depth->depth_compare));
    gl.DepthMask(depth->depth_write_enabled ? GL_TRUE : GL_FALSE);
  } else {
    gl.Disable(GL_DEPTH_TEST);
  }

  //--------------------------------------------------------------------------
  /// Setup culling.
  ///
  switch (pipeline.GetDescriptor().GetCullMode()) {
    case CullMode::kNone:
      gl.Disable(GL_CULL_FACE);
      break;
    case CullMode::kFrontFace:
      gl.Enable(GL_CULL_FACE);
      gl.CullFace(GL_FRONT);
      break;
    case CullMode::kBackFace:
      gl.Enable(GL_CULL_FACE);
      gl.CullFace(GL_BACK);
      break;
  }
  //--------------------------------------------------------------------------
  /// Setup winding order.
  ///
  switch (pipeline.GetDescriptor().GetWindingOrder()) {
    case WindingOrder::kClockwise:
      gl.FrontFace(GL_CW);
      break;
    case WindingOrder::kCounterClockwise:
      gl.FrontFace(GL_CCW);
      break;
  }

  //--------------------------------------------------------------------------
  /// Bind the pipeline program.
  ///
  if (!pipeline.BindProgram()) {
    VALIDATION_LOG << "Failed to bind pipeline program";
  }

  BufferBindingsGLES* vertex_desc_gles = pipeline.GetBufferBindings();

  //--------------------------------------------------------------------------
  /// Bind uniform data.
  ///
  if (!vertex_desc_gles->BindUniformData(
          gl,  //
          /*bound_textures=*/bound_textures_.data(),
          /*bound_buffers=*/bound_buffers_.data(),
          /*texture_range=*/Range{0, bound_texture_index_},
          /*buffer_range=*/Range{0, bound_buffers_index_})) {
    return fml::Status(fml::StatusCode::kInternal, "");
  }

  // Bind Vertex Data
  for (auto i = 0u; i < vertex_buffer_count_; i++) {
    if (!BindVertexBuffer(gl, vertex_desc_gles, vertex_buffers_[i], i)) {
      return fml::Status(fml::StatusCode::kInternal, "");
    }
  }

  //--------------------------------------------------------------------------
  /// Determine the primitive type.
  ///
  // GLES doesn't support setting the fill mode, so override the primitive
  // with GL_LINE_STRIP to somewhat emulate PolygonMode::kLine. This isn't
  // correct; full triangle outlines won't be drawn and disconnected
  // geometry may appear connected. However this can still be useful for
  // wireframe debug views.
  auto mode = pipeline.GetDescriptor().GetPolygonMode() == PolygonMode::kLine
                  ? GL_LINE_STRIP
                  : ToMode(pipeline.GetDescriptor().GetPrimitiveType());

  //--------------------------------------------------------------------------
  /// Finally! Invoke the draw call.
  ///
  if (index_type_ == IndexType::kNone) {
    gl.DrawArrays(mode, base_vertex_, element_count_);
  } else {
    // Bind the index buffer if necessary.
    const DeviceBuffer* index_buffer = index_buffer_.GetBuffer();
    const DeviceBufferGLES& index_buffer_gles =
        DeviceBufferGLES::Cast(*index_buffer);
    if (!index_buffer_gles.BindAndUploadDataIfNecessary(
            DeviceBufferGLES::BindingType::kElementArrayBuffer)) {
      fml::Status(fml::StatusCode::kInternal, "");
    }
    gl.DrawElements(mode,                      // mode
                    element_count_,            // count
                    ToIndexType(index_type_),  // type
                    reinterpret_cast<const GLvoid*>(static_cast<GLsizei>(
                        index_buffer_.GetRange().offset))  // indices
    );
  }

  //--------------------------------------------------------------------------
  /// Unbind vertex attribs.
  ///
  if (!vertex_desc_gles->UnbindVertexAttributes(gl)) {
    fml::Status(fml::StatusCode::kInternal, "");
  }

  // OK!
  return {};
}

// |ResourceBinder|
bool RenderPassGLES3::BindResource(ShaderStage stage,
                                   DescriptorType type,
                                   const ShaderUniformSlot& slot,
                                   const ShaderMetadata* metadata,
                                   BufferView view) {
  if (bound_buffers_index_ > 16) {
    return false;
  }
  bound_buffers_[bound_buffers_index_++] =
      BufferResource(metadata, std::move(view));
  return true;
}

// |ResourceBinder|
bool RenderPassGLES3::BindResource(
    ShaderStage stage,
    DescriptorType type,
    const SampledImageSlot& slot,
    const ShaderMetadata* metadata,
    std::shared_ptr<const Texture> texture,
    const std::unique_ptr<const Sampler>& sampler) {
  if (!sampler || !texture || !texture->IsValid() ||
      bound_texture_index_ > 16) {
    return false;
  }

  TextureResource resource = TextureResource(metadata, std::move(texture));

  bound_textures_[bound_texture_index_++] = TextureAndSampler{
      .slot = slot,
      .stage = stage,
      .texture = std::move(resource),
      .sampler = &sampler,
  };
  return true;
}

// |RenderPass|
bool RenderPassGLES3::BindDynamicResource(
    ShaderStage stage,
    DescriptorType type,
    const SampledImageSlot& slot,
    std::unique_ptr<ShaderMetadata> metadata,
    std::shared_ptr<const Texture> texture,
    const std::unique_ptr<const Sampler>& sampler) {
  if (!sampler || !texture || !texture->IsValid() ||
      bound_texture_index_ > 16) {
    return false;
  }

  TextureResource resource =
      TextureResource::MakeDynamic(std::move(metadata), std::move(texture));

  bound_textures_[bound_texture_index_++] = TextureAndSampler{
      .slot = slot,
      .stage = stage,
      .texture = std::move(resource),
      .sampler = &sampler,
  };
  return true;
}

// |RenderPass|
bool RenderPassGLES3::BindDynamicResource(
    ShaderStage stage,
    DescriptorType type,
    const ShaderUniformSlot& slot,
    std::unique_ptr<ShaderMetadata> metadata,
    BufferView view) {
  if (bound_buffers_index_ > 16) {
    return false;
  }
  bound_buffers_[bound_buffers_index_++] =
      BufferResource::MakeDynamic(std::move(metadata), std::move(view));
  return true;
}

// |RenderPass|
bool RenderPassGLES3::OnEncodeCommands(const Context& context) const {
  if (!IsValid()) {
    return false;
  }

  const auto& gl = reactor_->GetProcTable();
  TextureGLES& color_gles =
      TextureGLES::Cast(*render_pass_data_.color_attachment);
  const bool is_default_fbo = color_gles.IsWrapped();

  if (gl.DiscardFramebufferEXT.IsAvailable()) {
    std::array<GLenum, 3> attachments;
    size_t attachment_count = 0;

    // TODO(130048): discarding stencil or depth on the default fbo causes Angle
    // to discard the entire render target. Until we know the reason, default to
    // storing.
    bool angle_safe = gl.GetCapabilities()->IsANGLE() ? !is_default_fbo : true;

    if (render_pass_data_.discard_color_attachment) {
      attachments[attachment_count++] =
          (is_default_fbo ? GL_COLOR_EXT : GL_COLOR_ATTACHMENT0);
    }
    if (render_pass_data_.discard_depth_attachment && angle_safe) {
      attachments[attachment_count++] =
          (is_default_fbo ? GL_DEPTH_EXT : GL_DEPTH_ATTACHMENT);
    }

    if (render_pass_data_.discard_stencil_attachment && angle_safe) {
      attachments[attachment_count++] =
          (is_default_fbo ? GL_STENCIL_EXT : GL_STENCIL_ATTACHMENT);
    }
    gl.DiscardFramebufferEXT(GL_FRAMEBUFFER,     // target
                             attachment_count,   // attachments to discard
                             attachments.data()  // size
    );
  }
  ResetGLState(gl);

  GlobalStateGLES* global_state =
      ContextGLES::Cast(*GetContext()).GetGlobalState();
  if (prev_framebuffer_ != current_fbo_) {
    gl.BindFramebuffer(GL_FRAMEBUFFER, prev_framebuffer_);

    if (prev_viewport_.has_value() && prev_size_.has_value()) {
      const auto& viewport = prev_viewport_.value();
      const auto& target_size = prev_size_.value();
      gl.Viewport(viewport.rect.GetX(),  // x
                  target_size.height - viewport.rect.GetY() -
                      viewport.rect.GetHeight(),  // y
                  viewport.rect.GetWidth(),       // width
                  viewport.rect.GetHeight()       // height
      );
      if (gl.DepthRangef.IsAvailable()) {
        gl.DepthRangef(viewport.depth_range.z_near, viewport.depth_range.z_far);
      } else {
        gl.DepthRange(viewport.depth_range.z_near, viewport.depth_range.z_far);
      }

      gl.Enable(GL_SCISSOR_TEST);
      gl.Scissor(0,                  // x
                 0,                  // y
                 target_size.width,  // width
                 target_size.height  // height
      );
    }

    global_state->fbo_id = prev_framebuffer_;
    global_state->target_size = prev_size_;
    global_state->viewport = prev_viewport_;
  }
  if (current_fbo_ == GL_NONE) {
    global_state->fbo_id = GL_NONE;
    global_state->target_size = std::nullopt;
    global_state->viewport = std::nullopt;
  }

  return true;
}

}  // namespace impeller
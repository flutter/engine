// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/gles/render_pass_gles.h"

#include <algorithm>
#include <unordered_map>

#include "flutter/fml/trace_event.h"
#include "impeller/base/config.h"
#include "impeller/base/validation.h"
#include "impeller/renderer/backend/gles/device_buffer_gles.h"
#include "impeller/renderer/backend/gles/formats_gles.h"
#include "impeller/renderer/backend/gles/pipeline_gles.h"
#include "impeller/renderer/backend/gles/texture_gles.h"

namespace impeller {

static void ConfigureBlending(const ProcTableGLES& gl,
                              const ColorAttachmentDescriptor* color) {
  if (color->blending_enabled) {
    gl.Enable(GL_BLEND);
    gl.BlendFuncSeparate(
        ToBlendFactor(color->src_color_blend_factor),  // src color
        ToBlendFactor(color->dst_color_blend_factor),  // dst color
        ToBlendFactor(color->src_alpha_blend_factor),  // src alpha
        ToBlendFactor(color->dst_alpha_blend_factor)   // dst alpha
    );
    gl.BlendEquationSeparate(
        ToBlendOperation(color->color_blend_op),  // mode color
        ToBlendOperation(color->alpha_blend_op)   // mode alpha
    );
  } else {
    gl.Disable(GL_BLEND);
  }

  {
    const auto is_set = [](std::underlying_type_t<ColorWriteMask> mask,
                           ColorWriteMask check) -> GLboolean {
      using RawType = decltype(mask);
      return (static_cast<RawType>(mask) & static_cast<RawType>(check))
                 ? GL_TRUE
                 : GL_FALSE;
    };

    gl.ColorMask(is_set(color->write_mask, ColorWriteMask::kRed),    // red
                 is_set(color->write_mask, ColorWriteMask::kGreen),  // green
                 is_set(color->write_mask, ColorWriteMask::kBlue),   // blue
                 is_set(color->write_mask, ColorWriteMask::kAlpha)   // alpha
    );
  }
}

static void ConfigureStencil(GLenum face,
                             const ProcTableGLES& gl,
                             const StencilAttachmentDescriptor& stencil,
                             uint32_t stencil_reference) {
  gl.StencilOpSeparate(
      face,                                    // face
      ToStencilOp(stencil.stencil_failure),    // stencil fail
      ToStencilOp(stencil.depth_failure),      // depth fail
      ToStencilOp(stencil.depth_stencil_pass)  // depth stencil pass
  );
  gl.StencilFuncSeparate(face,                                        // face
                         ToCompareFunction(stencil.stencil_compare),  // func
                         stencil_reference,                           // ref
                         stencil.read_mask                            // mask
  );
  gl.StencilMaskSeparate(face, stencil.write_mask);
}

static void ConfigureStencil(const ProcTableGLES& gl,
                             const PipelineDescriptor& pipeline,
                             uint32_t stencil_reference) {
  if (!pipeline.HasStencilAttachmentDescriptors()) {
    gl.Disable(GL_STENCIL_TEST);
    return;
  }

  gl.Enable(GL_STENCIL_TEST);
  const auto& front = pipeline.GetFrontStencilAttachmentDescriptor();
  const auto& back = pipeline.GetBackStencilAttachmentDescriptor();
  if (front.has_value() && front == back) {
    ConfigureStencil(GL_FRONT_AND_BACK, gl, *front, stencil_reference);
  } else if (front.has_value()) {
    ConfigureStencil(GL_FRONT, gl, *front, stencil_reference);
  } else if (back.has_value()) {
    ConfigureStencil(GL_BACK, gl, *back, stencil_reference);
  } else {
    FML_UNREACHABLE();
  }
}

PassBindingsCache::PassBindingsCache(const ProcTableGLES& gl) : gl_(gl) {}

bool PassBindingsCache::Reset() {
  if (last_pipeline_) {
    return PipelineGLES::Cast(*last_pipeline_).UnbindProgram();
  }
  return true;
}

bool PassBindingsCache::ConfigurePipeline(
    std::shared_ptr<Pipeline<PipelineDescriptor>> pipeline) {
  if (last_pipeline_ != nullptr &&
      pipeline->GetDescriptor().IsEqual(last_pipeline_->GetDescriptor())) {
    return true;
  }
  last_pipeline_ = pipeline;

  const auto& gles_pipeline = PipelineGLES::Cast(*pipeline);

  const auto* color_attachment =
      gles_pipeline.GetDescriptor().GetLegacyCompatibleColorAttachment();
  if (!color_attachment) {
    VALIDATION_LOG
        << "Color attachment is too complicated for a legacy renderer.";
    return false;
  }

  //--------------------------------------------------------------------------
  /// Configure blending.
  ///
  ConfigureBlending(gl_, color_attachment);

  if (last_pipeline_ != nullptr) {
    if (!PipelineGLES::Cast(*last_pipeline_).UnbindProgram()) {
      return false;
    }
    if (!gles_pipeline.BindProgram()) {
      return false;
    }
  }
  return true;
}

void PassBindingsCache::ConfigureDepth(
    std::optional<DepthAttachmentDescriptor> depth) {
  if (!depth.has_value()) {
    if (had_depth_) {
      gl_.Disable(GL_DEPTH_TEST);
    }
    had_depth_ = false;
    return;
  }
  had_depth_ = true;

  gl_.Enable(GL_DEPTH_TEST);
  gl_.DepthFunc(ToCompareFunction(depth->depth_compare));
  gl_.DepthMask(depth->depth_write_enabled ? GL_TRUE : GL_FALSE);
}

bool PassBindingsCache::BindVertexBuffer(
    BufferView vertex_buffer_view,
    const std::shared_ptr<Allocator>& transients_allocator) {
  return BindBuffer(vertex_buffer_view,
                    DeviceBufferGLES::BindingType::kArrayBuffer,
                    transients_allocator);
}

bool PassBindingsCache::BindIndexBuffer(
    BufferView index_buffer_view,
    const std::shared_ptr<Allocator>& transients_allocator) {
  return BindBuffer(index_buffer_view,
                    DeviceBufferGLES::BindingType::kElementArrayBuffer,
                    transients_allocator);
}

bool PassBindingsCache::BindBuffer(
    BufferView buffer_view,
    DeviceBufferGLES::BindingType binding_type,
    const std::shared_ptr<Allocator>& transients_allocator) {
  auto existing = bound_buffers_.find(static_cast<int>(binding_type));
  if (existing->second == buffer_view.buffer) {
    return true;
  }

  auto buffer = buffer_view.buffer->GetDeviceBuffer(*transients_allocator);

  if (!buffer) {
    return false;
  }

  const auto& buffer_gles = DeviceBufferGLES::Cast(*buffer);
  if (!buffer_gles.BindAndUploadDataIfNecessary(binding_type)) {
    return false;
  }
  bound_buffers_[static_cast<int>(binding_type)] = buffer_view.buffer;
  return true;
}

void PassBindingsCache::ConfigureScissor(std::optional<IRect> maybe_scissor,
                                         ISize target_size) {
  if (!maybe_scissor.has_value()) {
    if (had_scissor_) {
      gl_.Disable(GL_SCISSOR_TEST);
    }
    return;
  }
  had_scissor_ = true;
  const auto& scissor = maybe_scissor.value();
  gl_.Enable(GL_SCISSOR_TEST);
  gl_.Scissor(scissor.origin.x,                                             // x
              target_size.height - scissor.origin.y - scissor.size.height,  // y
              scissor.size.width,  // width
              scissor.size.height  // height
  );
}

void PassBindingsCache::ConfigureViewport(Viewport viewport,
                                          ISize target_size,
                                          bool has_depth) {
  if (viewport == last_viewport_ && target_size == last_target_size_ &&
      has_depth == viewport_had_depth_) {
    return;
  }
  last_viewport_ = viewport;
  last_target_size_ = target_size;
  viewport_had_depth_ = has_depth;

  gl_.Viewport(viewport.rect.origin.x,  // x
               target_size.height - viewport.rect.origin.y -
                   viewport.rect.size.height,  // y
               viewport.rect.size.width,       // width
               viewport.rect.size.height       // height
  );

  if (has_depth) {
    gl_.DepthRangef(viewport.depth_range.z_near, viewport.depth_range.z_far);
  }
}

void PassBindingsCache::SetCullMode(CullMode cull_mode) {
  if (last_cull_mode_ == cull_mode) {
    return;
  }
  last_cull_mode_ = cull_mode;
  switch (cull_mode) {
    case CullMode::kNone:
      gl_.Disable(GL_CULL_FACE);
      break;
    case CullMode::kFrontFace:
      gl_.Enable(GL_CULL_FACE);
      gl_.CullFace(GL_FRONT);
      break;
    case CullMode::kBackFace:
      gl_.Enable(GL_CULL_FACE);
      gl_.CullFace(GL_BACK);
      break;
  }
}

void PassBindingsCache::SetWindingOrder(WindingOrder winding_order) {
  if (last_winding_order_ == winding_order) {
    return;
  }
  last_winding_order_ = winding_order;
  switch (winding_order) {
    case WindingOrder::kClockwise:
      gl_.FrontFace(GL_CW);
      break;
    case WindingOrder::kCounterClockwise:
      gl_.FrontFace(GL_CCW);
      break;
  }
}

RenderPassGLES::RenderPassGLES(std::weak_ptr<const Context> context,
                               const RenderTarget& target,
                               ReactorGLES::Ref reactor)
    : RenderPass(std::move(context), target),
      reactor_(std::move(reactor)),
      is_valid_(reactor_ && reactor_->IsValid()) {}

// |RenderPass|
RenderPassGLES::~RenderPassGLES() = default;

// |RenderPass|
bool RenderPassGLES::IsValid() const {
  return is_valid_;
}

// |RenderPass|
void RenderPassGLES::OnSetLabel(std::string label) {
  label_ = std::move(label);
}

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

[[nodiscard]] bool EncodeCommandsInReactor(
    const RenderPassData& pass_data,
    const std::shared_ptr<Allocator>& transients_allocator,
    const ReactorGLES& reactor,
    const std::vector<Command>& commands) {
  TRACE_EVENT0("impeller", "RenderPassGLES::EncodeCommandsInReactor");

  if (commands.empty()) {
    return true;
  }

  const auto& gl = reactor.GetProcTable();

  fml::ScopedCleanupClosure pop_pass_debug_marker(
      [&gl]() { gl.PopDebugGroup(); });
  if (!pass_data.label.empty()) {
    gl.PushDebugGroup(pass_data.label);
  } else {
    pop_pass_debug_marker.Release();
  }

  GLuint fbo = GL_NONE;
  fml::ScopedCleanupClosure delete_fbo([&gl, &fbo]() {
    if (fbo != GL_NONE) {
      gl.BindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
      gl.DeleteFramebuffers(1u, &fbo);
    }
  });

  const auto is_default_fbo =
      TextureGLES::Cast(*pass_data.color_attachment).IsWrapped();

  if (!is_default_fbo) {
    // Create and bind an offscreen FBO.
    gl.GenFramebuffers(1u, &fbo);
    gl.BindFramebuffer(GL_FRAMEBUFFER, fbo);

    if (auto color = TextureGLES::Cast(pass_data.color_attachment.get())) {
      if (!color->SetAsFramebufferAttachment(
              GL_FRAMEBUFFER, fbo, TextureGLES::AttachmentPoint::kColor0)) {
        return false;
      }
    }
    if (auto depth = TextureGLES::Cast(pass_data.depth_attachment.get())) {
      if (!depth->SetAsFramebufferAttachment(
              GL_FRAMEBUFFER, fbo, TextureGLES::AttachmentPoint::kDepth)) {
        return false;
      }
    }
    if (auto stencil = TextureGLES::Cast(pass_data.stencil_attachment.get())) {
      if (!stencil->SetAsFramebufferAttachment(
              GL_FRAMEBUFFER, fbo, TextureGLES::AttachmentPoint::kStencil)) {
        return false;
      }
    }

    if (gl.CheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      VALIDATION_LOG << "Could not create a complete frambuffer.";
      return false;
    }
  }

  gl.ClearColor(pass_data.clear_color.red,    // red
                pass_data.clear_color.green,  // green
                pass_data.clear_color.blue,   // blue
                pass_data.clear_color.alpha   // alpha
  );
  if (pass_data.depth_attachment) {
    gl.ClearDepthf(pass_data.clear_depth);
  }
  if (pass_data.stencil_attachment) {
    gl.ClearStencil(pass_data.clear_stencil);
  }

  GLenum clear_bits = 0u;
  if (pass_data.clear_color_attachment) {
    clear_bits |= GL_COLOR_BUFFER_BIT;
  }
  if (pass_data.clear_depth_attachment) {
    clear_bits |= GL_DEPTH_BUFFER_BIT;
  }
  if (pass_data.clear_stencil_attachment) {
    clear_bits |= GL_STENCIL_BUFFER_BIT;
  }

  gl.Disable(GL_SCISSOR_TEST);
  gl.Disable(GL_DEPTH_TEST);
  gl.Disable(GL_STENCIL_TEST);
  gl.Disable(GL_CULL_FACE);
  gl.Disable(GL_BLEND);
  gl.ColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  gl.FrontFace(GL_CW);
  gl.Clear(clear_bits);

  PassBindingsCache pass_bindings(gl);
  for (const auto& command : commands) {
    if (command.instance_count != 1u) {
      VALIDATION_LOG << "GLES backend does not support instanced rendering.";
      return false;
    }

    if (!command.pipeline) {
      VALIDATION_LOG << "Command has no pipeline specified.";
      return false;
    }

    // This can be re-enabled locally, but in production builds it adds a
    // substantial amount of overhead per render pass.
#if 0
    fml::ScopedCleanupClosure pop_cmd_debug_marker(
        [&gl]() { gl.PopDebugGroup(); });
    if (!command.label.empty()) {
      gl.PushDebugGroup(command.label);
    } else {
      pop_cmd_debug_marker.Release();
    }
#endif

    const auto& pipeline = PipelineGLES::Cast(*command.pipeline);

    if (!pass_bindings.ConfigurePipeline(command.pipeline)) {
      return false;
    }

    //--------------------------------------------------------------------------
    /// Setup stencil.
    ///
    ConfigureStencil(gl, pipeline.GetDescriptor(), command.stencil_reference);

    //--------------------------------------------------------------------------
    /// Configure depth.
    ///
    pass_bindings.ConfigureDepth(
        pipeline.GetDescriptor().GetDepthStencilAttachmentDescriptor());

    // Both the viewport and scissor are specified in framebuffer coordinates.
    // Impeller's framebuffer coordinate system is top left origin, but OpenGL's
    // is bottom left origin, so we convert the coordinates here.
    auto target_size = pass_data.color_attachment->GetSize();

    //--------------------------------------------------------------------------
    /// Setup the viewport.
    ///
    pass_bindings.ConfigureViewport(
        command.viewport.value_or(pass_data.viewport), target_size,
        pipeline.GetDescriptor()
            .GetDepthStencilAttachmentDescriptor()
            .has_value());

    //--------------------------------------------------------------------------
    /// Setup the scissor rect.
    ///
    pass_bindings.ConfigureScissor(command.scissor, target_size);

    //--------------------------------------------------------------------------
    /// Setup culling and winding order.
    ///
    pass_bindings.SetCullMode(pipeline.GetDescriptor().GetCullMode());
    pass_bindings.SetWindingOrder(pipeline.GetDescriptor().GetWindingOrder());

    if (command.index_type == IndexType::kUnknown) {
      return false;
    }

    const auto& vertex_desc_gles = pipeline.GetBufferBindings();

    //--------------------------------------------------------------------------
    /// Bind vertex and index buffers.
    ///
    auto vertex_buffer_view = command.GetVertexBuffer();

    if (!vertex_buffer_view) {
      return false;
    }
    if (!pass_bindings.BindVertexBuffer(vertex_buffer_view,
                                        transients_allocator)) {
      return false;
    }

    //--------------------------------------------------------------------------
    /// Bind vertex attribs.
    ///
    if (!vertex_desc_gles->BindVertexAttributes(
            gl, vertex_buffer_view.range.offset)) {
      return false;
    }

    //--------------------------------------------------------------------------
    /// Bind uniform data.
    ///
    if (!vertex_desc_gles->BindUniformData(gl,                        //
                                           *transients_allocator,     //
                                           command.vertex_bindings,   //
                                           command.fragment_bindings  //
                                           )) {
      return false;
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
    if (command.index_type == IndexType::kNone) {
      gl.DrawArrays(mode, command.base_vertex, command.vertex_count);
    } else {
      // Bind the index buffer if necessary.
      if (!pass_bindings.BindIndexBuffer(command.index_buffer,
                                         transients_allocator)) {
        return false;
      }
      gl.DrawElements(mode,                             // mode
                      command.vertex_count,             // count
                      ToIndexType(command.index_type),  // type
                      reinterpret_cast<const GLvoid*>(static_cast<GLsizei>(
                          command.index_buffer.range.offset))  // indices
      );
    }

    //--------------------------------------------------------------------------
    /// Unbind vertex attribs.
    ///
    if (!vertex_desc_gles->UnbindVertexAttributes(gl)) {
      return false;
    }
  }

  //--------------------------------------------------------------------------
  /// Unbind the last program pipeline.
  ///
  if (!pass_bindings.Reset()) {
    return false;
  }

  if (gl.DiscardFramebufferEXT.IsAvailable()) {
    std::vector<GLenum> attachments;
    if (pass_data.discard_color_attachment) {
      attachments.push_back(is_default_fbo ? GL_COLOR_EXT
                                           : GL_COLOR_ATTACHMENT0);
    }
    if (pass_data.discard_depth_attachment) {
      attachments.push_back(is_default_fbo ? GL_DEPTH_EXT
                                           : GL_DEPTH_ATTACHMENT);
    }
    if (pass_data.discard_stencil_attachment) {
      attachments.push_back(is_default_fbo ? GL_STENCIL_EXT
                                           : GL_STENCIL_ATTACHMENT);
    }
    gl.DiscardFramebufferEXT(GL_FRAMEBUFFER,      // target
                             attachments.size(),  // attachments to discard
                             attachments.data()   // size
    );
  }

  return true;
}

// |RenderPass|
bool RenderPassGLES::OnEncodeCommands(const Context& context) const {
  if (!IsValid()) {
    return false;
  }
  if (commands_.empty()) {
    return true;
  }
  const auto& render_target = GetRenderTarget();
  if (!render_target.HasColorAttachment(0u)) {
    return false;
  }
  const auto& color0 = render_target.GetColorAttachments().at(0u);
  const auto& depth0 = render_target.GetDepthAttachment();
  const auto& stencil0 = render_target.GetStencilAttachment();

  auto pass_data = std::make_shared<RenderPassData>();
  pass_data->label = label_;
  pass_data->viewport.rect = Rect::MakeSize(GetRenderTargetSize());

  //----------------------------------------------------------------------------
  /// Setup color data.
  ///
  pass_data->color_attachment = color0.texture;
  pass_data->clear_color = color0.clear_color;
  pass_data->clear_color_attachment = CanClearAttachment(color0.load_action);
  pass_data->discard_color_attachment =
      CanDiscardAttachmentWhenDone(color0.store_action);

  //----------------------------------------------------------------------------
  /// Setup depth data.
  ///
  if (depth0.has_value()) {
    pass_data->depth_attachment = depth0->texture;
    pass_data->clear_depth = depth0->clear_depth;
    pass_data->clear_depth_attachment = CanClearAttachment(depth0->load_action);
    pass_data->discard_depth_attachment =
        CanDiscardAttachmentWhenDone(depth0->store_action);
  }

  //----------------------------------------------------------------------------
  /// Setup depth data.
  ///
  if (stencil0.has_value()) {
    pass_data->stencil_attachment = stencil0->texture;
    pass_data->clear_stencil = stencil0->clear_stencil;
    pass_data->clear_stencil_attachment =
        CanClearAttachment(stencil0->load_action);
    pass_data->discard_stencil_attachment =
        CanDiscardAttachmentWhenDone(stencil0->store_action);
  }

  std::shared_ptr<const RenderPassGLES> shared_this = shared_from_this();
  return reactor_->AddOperation([pass_data,
                                 allocator = context.GetResourceAllocator(),
                                 render_pass = std::move(shared_this)](
                                    const auto& reactor) {
    auto result = EncodeCommandsInReactor(*pass_data, allocator, reactor,
                                          render_pass->commands_);
    FML_CHECK(result) << "Must be able to encode GL commands without error.";
  });
}

}  // namespace impeller

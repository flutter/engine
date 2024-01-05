// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/render_pass.h"

namespace impeller {

RenderPass::RenderPass(std::weak_ptr<const Context> context,
                       const RenderTarget& target)
    : context_(std::move(context)),
      sample_count_(target.GetSampleCount()),
      pixel_format_(target.GetRenderTargetPixelFormat()),
      has_stencil_attachment_(target.GetStencilAttachment().has_value()),
      render_target_size_(target.GetRenderTargetSize()),
      render_target_(target),
      transients_buffer_(),
      orthographic_(Matrix::MakeOrthographic(render_target_size_)) {
  auto strong_context = context_.lock();
  FML_DCHECK(strong_context);
  transients_buffer_ = strong_context->GetHostBufferPool().Grab();
}

RenderPass::~RenderPass() {
  auto strong_context = context_.lock();
  if (strong_context) {
    strong_context->GetHostBufferPool().Recycle(transients_buffer_);
  }
}

SampleCount RenderPass::GetSampleCount() const {
  return sample_count_;
}

PixelFormat RenderPass::GetRenderTargetPixelFormat() const {
  return pixel_format_;
}

bool RenderPass::HasStencilAttachment() const {
  return has_stencil_attachment_;
}

const RenderTarget& RenderPass::GetRenderTarget() const {
  return render_target_;
}

ISize RenderPass::GetRenderTargetSize() const {
  return render_target_size_;
}

const Matrix& RenderPass::GetOrthographicTransform() const {
  return orthographic_;
}

HostBuffer& RenderPass::GetTransientsBuffer() {
  return *transients_buffer_;
}

void RenderPass::SetLabel(std::string label) {
  if (label.empty()) {
    return;
  }
  transients_buffer_->SetLabel(SPrintF("%s Transients", label.c_str()));
  OnSetLabel(std::move(label));
}

bool RenderPass::AddCommand(Command&& command) {
  if (!command.IsValid()) {
    VALIDATION_LOG << "Attempted to add an invalid command to the render pass.";
    return false;
  }

  if (command.scissor.has_value()) {
    auto target_rect = IRect::MakeSize(render_target_.GetRenderTargetSize());
    if (!target_rect.Contains(command.scissor.value())) {
      VALIDATION_LOG << "Cannot apply a scissor that lies outside the bounds "
                        "of the render target.";
      return false;
    }
  }

  if (command.vertex_buffer.vertex_count == 0u ||
      command.instance_count == 0u) {
    // Essentially a no-op. Don't record the command but this is not necessary
    // an error either.
    return true;
  }
  auto buffer_offset = bound_buffers_.size();
  auto texture_offset = bound_textures_.size();
  auto buffer_length = command.bindings.buffer_offset;
  auto texture_length = command.bindings.texture_offset;
  for (auto i = 0u; i < command.bindings.texture_offset; i++) {
    bound_textures_.push_back(std::move(command.bindings.bound_textures[i]));
  }
  for (auto i = 0u; i < command.bindings.buffer_offset; i++) {
    bound_buffers_.push_back(std::move(command.bindings.bound_buffers[i]));
  }

  commands_.emplace_back(BoundCommand{
      .pipeline = std::move(command.pipeline),
      .buffer_offset = buffer_offset,
      .buffer_length = buffer_length,
      .texture_offset = texture_offset,
      .texture_length = texture_length,
#ifdef IMPELLER_DEBUG
      .label = std::move(command.label),
#endif  // IMPELLER_DEBUG
      .stencil_reference = command.stencil_reference,
      .base_vertex = command.base_vertex,
      .viewport = command.viewport,
      .scissor = command.scissor,
      .instance_count = command.instance_count,
      .vertex_buffer = std::move(command.vertex_buffer),
  });
  return true;
}

bool RenderPass::EncodeCommands() const {
  auto context = context_.lock();
  // The context could have been collected in the meantime.
  if (!context) {
    return false;
  }
  return OnEncodeCommands(*context);
}

const std::weak_ptr<const Context>& RenderPass::GetContext() const {
  return context_;
}

}  // namespace impeller

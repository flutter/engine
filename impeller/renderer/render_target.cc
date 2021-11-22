// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/render_target.h"

#include "impeller/renderer/allocator.h"
#include "impeller/renderer/context.h"
#include "impeller/renderer/texture.h"

namespace impeller {

RenderTarget::RenderTarget() = default;

RenderTarget::~RenderTarget() = default;

bool RenderTarget::HasColorAttachment(size_t index) const {
  if (auto found = colors_.find(index); found != colors_.end()) {
    return true;
  }
  return false;
}

std::optional<ISize> RenderTarget::GetColorAttachmentSize(size_t index) const {
  auto found = colors_.find(index);

  if (found == colors_.end()) {
    return std::nullopt;
  }

  return found->second.texture->GetSize();
}

ISize RenderTarget::GetRenderTargetSize() const {
  auto size = GetColorAttachmentSize(0u);
  return size.has_value() ? size.value() : ISize{};
}

RenderTarget& RenderTarget::SetColorAttachment(ColorAttachment attachment,
                                               size_t index) {
  if (attachment) {
    colors_[index] = attachment;
  }
  return *this;
}

RenderTarget& RenderTarget::SetDepthAttachment(DepthAttachment attachment) {
  if (attachment) {
    depth_ = std::move(attachment);
  }
  return *this;
}

RenderTarget& RenderTarget::SetStencilAttachment(StencilAttachment attachment) {
  if (attachment) {
    stencil_ = std::move(attachment);
  }
  return *this;
}

const std::map<size_t, ColorAttachment>& RenderTarget::GetColorAttachments()
    const {
  return colors_;
}

const std::optional<DepthAttachment>& RenderTarget::GetDepthAttachment() const {
  return depth_;
}

const std::optional<StencilAttachment>& RenderTarget::GetStencilAttachment()
    const {
  return stencil_;
}

RenderTarget RenderTarget::CreateOffscreen(const Context& context,
                                           ISize size,
                                           std::string label) {
  TextureDescriptor color_tex0;
  color_tex0.format = PixelFormat::kB8G8R8A8UNormInt;
  color_tex0.size = size;
  color_tex0.usage = static_cast<uint64_t>(TextureUsage::kRenderTarget);

  TextureDescriptor stencil_tex0;
  stencil_tex0.format = PixelFormat::kD32FloatS8UNormInt;
  stencil_tex0.size = size;
  stencil_tex0.usage =
      static_cast<TextureUsageMask>(TextureUsage::kShaderRead) |
      static_cast<TextureUsageMask>(TextureUsage::kShaderWrite);

  ColorAttachment color0;
  color0.clear_color = Color::BlackTransparent();
  color0.load_action = LoadAction::kClear;
  color0.store_action = StoreAction::kStore;
  color0.texture = context.GetPermanentsAllocator()->CreateTexture(
      StorageMode::kDevicePrivate, color_tex0);

  if (!color0.texture) {
    return {};
  }

  color0.texture->SetLabel(label);

  StencilAttachment stencil0;
  stencil0.load_action = LoadAction::kClear;
  stencil0.store_action = StoreAction::kDontCare;
  stencil0.clear_stencil = 0u;
  stencil0.texture = context.GetPermanentsAllocator()->CreateTexture(
      StorageMode::kDeviceTransient, stencil_tex0);

  if (!stencil0.texture) {
    return {};
  }

  stencil0.texture->SetLabel(label);

  RenderTarget target;
  target.SetColorAttachment(std::move(color0), 0u);
  target.SetStencilAttachment(std::move(stencil0));

  return target;
}

}  // namespace impeller

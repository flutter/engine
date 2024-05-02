// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/vulkan/texture_vk.h"

#include "impeller/renderer/backend/vulkan/command_buffer_vk.h"
#include "impeller/renderer/backend/vulkan/command_encoder_vk.h"
#include "impeller/renderer/backend/vulkan/formats_vk.h"
#include "impeller/renderer/backend/vulkan/sampler_vk.h"

namespace impeller {

TextureVK::TextureVK(std::weak_ptr<Context> context,
                     std::shared_ptr<TextureSourceVK> source)
    : Texture(source->GetTextureDescriptor()),
      context_(std::move(context)),
      source_(std::move(source)) {}

TextureVK::~TextureVK() = default;

void TextureVK::SetLabel(std::string_view label) {
  auto context = context_.lock();
  if (!context) {
    // The context may have died.
    return;
  }
  ContextVK::Cast(*context).SetDebugName(GetImage(), label);
  ContextVK::Cast(*context).SetDebugName(GetImageView(), label);
}

bool TextureVK::IsValid() const {
  return !!source_;
}

ISize TextureVK::GetSize() const {
  return GetTextureDescriptor().size;
}

vk::Image TextureVK::GetImage() const {
  return source_->GetImage();
}

vk::ImageView TextureVK::GetImageView() const {
  return source_->GetImageView();
}

std::shared_ptr<const TextureSourceVK> TextureVK::GetTextureSource() const {
  return source_;
}

bool TextureVK::SetLayout(const BarrierVK& barrier) const {
  return source_ ? source_->SetLayout(barrier).ok() : false;
}

vk::ImageLayout TextureVK::SetLayoutWithoutEncoding(
    vk::ImageLayout layout) const {
  return source_ ? source_->SetLayoutWithoutEncoding(layout)
                 : vk::ImageLayout::eUndefined;
}

vk::ImageLayout TextureVK::GetLayout() const {
  return source_ ? source_->GetLayout() : vk::ImageLayout::eUndefined;
}

vk::ImageView TextureVK::GetRenderTargetView() const {
  return source_->GetRenderTargetView();
}

void TextureVK::SetCachedFramebuffer(
    const SharedHandleVK<vk::Framebuffer>& framebuffer) {
  source_->SetCachedFramebuffer(framebuffer);
}

void TextureVK::SetCachedRenderPass(
    const SharedHandleVK<vk::RenderPass>& render_pass) {
  source_->SetCachedRenderPass(render_pass);
}

SharedHandleVK<vk::Framebuffer> TextureVK::GetCachedFramebuffer() const {
  return source_->GetCachedFramebuffer();
}

SharedHandleVK<vk::RenderPass> TextureVK::GetCachedRenderPass() const {
  return source_->GetCachedRenderPass();
}

void TextureVK::SetMipMapGenerated() {
  mipmap_generated_ = true;
}

bool TextureVK::IsSwapchainImage() const {
  return source_->IsSwapchainImage();
}

std::shared_ptr<SamplerVK> TextureVK::GetImmutableSamplerVariant(
    const SamplerVK& sampler) const {
  if (!source_) {
    return nullptr;
  }
  auto conversion = source_->GetYUVConversion();
  if (!conversion) {
    // Most textures don't need a sampler conversion and will go down this path.
    // Only needed for YUV sampling from external textures.
    return nullptr;
  }
  return sampler.CreateVariantForConversion(std::move(conversion));
}

}  // namespace impeller

// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/entity/render_target_cache.h"
#include "impeller/renderer/render_target.h"

namespace impeller {

RenderTargetCache::RenderTargetCache(std::shared_ptr<Allocator> allocator)
    : RenderTargetAllocator(std::move(allocator)) {}

void RenderTargetCache::Start() {
  for (auto& td : render_target_data_) {
    td.used_this_frame = false;
    td.safe_for_use = true;
  }
}

void RenderTargetCache::End() {
  std::vector<RenderTargetData> retain;
  for (auto& td : render_target_data_) {
    if (td.used_this_frame) {
      td.render_target.SetFrameKey(std::nullopt);
      retain.push_back(td);
    }
  }
  render_target_data_.swap(retain);
}

RenderTarget RenderTargetCache::CreateOffscreen(
    const Context& context,
    ISize size,
    int mip_count,
    const std::string& label,
    RenderTarget::AttachmentConfig color_attachment_config,
    std::optional<RenderTarget::AttachmentConfig> stencil_attachment_config,
    const std::shared_ptr<Texture>& existing_color_texture,
    const std::shared_ptr<Texture>& existing_depth_stencil_texture) {
  if (size.IsEmpty()) {
    return {};
  }

  FML_DCHECK(existing_color_texture == nullptr &&
             existing_depth_stencil_texture == nullptr);
  auto config = RenderTargetConfig{
      .size = size,
      .mip_count = static_cast<size_t>(mip_count),
      .has_msaa = false,
      .has_depth_stencil = stencil_attachment_config.has_value(),
  };
  size_t offset = 0;
  for (auto& render_target_data : render_target_data_) {
    const auto other_config = render_target_data.config;
    if (render_target_data.safe_for_use && other_config == config) {
      render_target_data.used_this_frame = true;
      render_target_data.safe_for_use = false;
      auto color0 = render_target_data.render_target.GetColorAttachments()
                        .find(0u)
                        ->second;
      auto depth = render_target_data.render_target.GetDepthAttachment();
      std::shared_ptr<Texture> depth_tex = depth ? depth->texture : nullptr;
      auto result = RenderTargetAllocator::CreateOffscreen(
          context, size, mip_count, label, color_attachment_config,
          stencil_attachment_config, color0.texture, depth_tex);
      result.SetFrameKey(offset);
      return result;
    }
    offset++;
  }
  RenderTarget created_target = RenderTargetAllocator::CreateOffscreen(
      context, size, mip_count, label, color_attachment_config,
      stencil_attachment_config);
  if (!created_target.IsValid()) {
    return created_target;
  }
  render_target_data_.push_back(RenderTargetData{
      .used_this_frame = true,         //
      .safe_for_use = false,           //
      .config = config,                //
      .render_target = created_target  //
  });
  created_target.SetFrameKey(render_target_data_.size() - 1);
  return created_target;
}

RenderTarget RenderTargetCache::CreateOffscreenMSAA(
    const Context& context,
    ISize size,
    int mip_count,
    const std::string& label,
    RenderTarget::AttachmentConfigMSAA color_attachment_config,
    std::optional<RenderTarget::AttachmentConfig> stencil_attachment_config,
    const std::shared_ptr<Texture>& existing_color_msaa_texture,
    const std::shared_ptr<Texture>& existing_color_resolve_texture,
    const std::shared_ptr<Texture>& existing_depth_stencil_texture) {
  if (size.IsEmpty()) {
    return {};
  }

  FML_DCHECK(existing_color_msaa_texture == nullptr &&
             existing_color_resolve_texture == nullptr &&
             existing_depth_stencil_texture == nullptr);
  auto config = RenderTargetConfig{
      .size = size,
      .mip_count = static_cast<size_t>(mip_count),
      .has_msaa = true,
      .has_depth_stencil = stencil_attachment_config.has_value(),
  };
  size_t offset = 0;
  for (auto& render_target_data : render_target_data_) {
    const auto other_config = render_target_data.config;
    if (render_target_data.safe_for_use && other_config == config) {
      render_target_data.used_this_frame = true;
      render_target_data.safe_for_use = false;
      auto color0 = render_target_data.render_target.GetColorAttachments()
                        .find(0u)
                        ->second;
      auto depth = render_target_data.render_target.GetDepthAttachment();
      std::shared_ptr<Texture> depth_tex = depth ? depth->texture : nullptr;
      auto result = RenderTargetAllocator::CreateOffscreenMSAA(
          context, size, mip_count, label, color_attachment_config,
          stencil_attachment_config, color0.texture, color0.resolve_texture,
          depth_tex);
      result.SetFrameKey(offset);
      return result;
    }
    offset++;
  }
  RenderTarget created_target = RenderTargetAllocator::CreateOffscreenMSAA(
      context, size, mip_count, label, color_attachment_config,
      stencil_attachment_config);
  if (!created_target.IsValid()) {
    return created_target;
  }
  render_target_data_.push_back(RenderTargetData{
      .used_this_frame = true,         //
      .safe_for_use = false,           //
      .config = config,                //
      .render_target = created_target  //
  });
  created_target.SetFrameKey(render_target_data_.size() - 1);
  return created_target;
}

void RenderTargetCache::Reclaim(const RenderTarget& render_target) {
  auto key = render_target.GetFrameKey();
  if (key.has_value() && key.value() < render_target_data_.size()) {
    render_target_data_[key.value()].safe_for_use = true;
  }
}

size_t RenderTargetCache::CachedTextureCount() const {
  return render_target_data_.size();
}

}  // namespace impeller

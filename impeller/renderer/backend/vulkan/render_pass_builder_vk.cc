// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/vulkan/render_pass_builder_vk.h"

#include <algorithm>
#include <vector>

#include "impeller/renderer/backend/vulkan/formats_vk.h"

namespace impeller {

RenderPassBuilderVK::RenderPassBuilderVK() = default;

RenderPassBuilderVK::~RenderPassBuilderVK() = default;

RenderPassBuilderVK& RenderPassBuilderVK::SetColorAttachment(
    size_t index,
    PixelFormat format,
    SampleCount sample_count,
    LoadAction load_action,
    StoreAction store_action) {
  vk::AttachmentDescription desc;
  desc.format = ToVKImageFormat(format);
  desc.samples = ToVKSampleCount(sample_count);
  desc.loadOp = ToVKAttachmentLoadOp(load_action);
  desc.storeOp = ToVKAttachmentStoreOp(store_action);
  desc.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
  desc.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
  desc.initialLayout = vk::ImageLayout::eGeneral;
  desc.finalLayout = vk::ImageLayout::eGeneral;
  colors_[index] = desc;

  desc.samples = vk::SampleCountFlagBits::e1;
  resolves_[index] = desc;

  return *this;
}

RenderPassBuilderVK& RenderPassBuilderVK::SetDepthStencilAttachment(
    PixelFormat format,
    SampleCount sample_count,
    LoadAction load_action,
    StoreAction store_action) {
  vk::AttachmentDescription desc;
  desc.format = ToVKImageFormat(format);
  desc.samples = ToVKSampleCount(sample_count);
  desc.loadOp = ToVKAttachmentLoadOp(load_action);
  desc.storeOp = ToVKAttachmentStoreOp(store_action);
  desc.stencilLoadOp = desc.loadOp;    // Not separable in Impeller.
  desc.stencilStoreOp = desc.storeOp;  // Not separable in Impeller.
  desc.initialLayout = vk::ImageLayout::eGeneral;
  desc.finalLayout = vk::ImageLayout::eGeneral;
  depth_stencil_ = desc;
  return *this;
}

RenderPassBuilderVK& RenderPassBuilderVK::SetStencilAttachment(
    PixelFormat format,
    SampleCount sample_count,
    LoadAction load_action,
    StoreAction store_action) {
  vk::AttachmentDescription desc;
  desc.format = ToVKImageFormat(format);
  desc.samples = ToVKSampleCount(sample_count);
  desc.loadOp = vk::AttachmentLoadOp::eDontCare;
  desc.storeOp = vk::AttachmentStoreOp::eDontCare;
  desc.stencilLoadOp = ToVKAttachmentLoadOp(load_action);
  desc.stencilStoreOp = ToVKAttachmentStoreOp(store_action);
  desc.initialLayout = vk::ImageLayout::eGeneral;
  desc.finalLayout = vk::ImageLayout::eGeneral;
  depth_stencil_ = desc;
  return *this;
}

vk::UniqueRenderPass RenderPassBuilderVK::Build(
    const vk::Device& device) const {
  if (colors_.empty()) {
    return {};
  }

  FML_DCHECK(colors_.size() == resolves_.size());

  // This must be less than `VkPhysicalDeviceLimits::maxColorAttachments` but we
  // are not checking.
  const auto color_attachments_count = colors_.rbegin()->first + 1u;

  std::vector<vk::AttachmentDescription> attachments;

  std::vector<vk::AttachmentReference> color_refs(color_attachments_count,
                                                  kUnusedAttachmentReference);
  std::vector<vk::AttachmentReference> resolve_refs(color_attachments_count,
                                                    kUnusedAttachmentReference);
  vk::AttachmentReference depth_stencil_ref = kUnusedAttachmentReference;

  for (const auto& color : colors_) {
    vk::AttachmentReference color_ref;
    color_ref.attachment = attachments.size();
    color_ref.layout = vk::ImageLayout::eGeneral;
    color_refs[color.first] = color_ref;
    attachments.push_back(color.second);

    if (color.second.samples != vk::SampleCountFlagBits::e1) {
      vk::AttachmentReference resolve_ref;
      resolve_ref.attachment = attachments.size();
      resolve_ref.layout = vk::ImageLayout::eGeneral;
      resolve_refs[color.first] = resolve_ref;
      attachments.push_back(resolves_.at(color.first));
    }
  }

  if (depth_stencil_.has_value()) {
    vk::AttachmentReference depth_stencil_ref;
    depth_stencil_ref.attachment = attachments.size();
    depth_stencil_ref.layout = vk::ImageLayout::eGeneral;
    attachments.push_back(depth_stencil_.value());
  }

  std::vector<vk::SubpassDescription> subpasses;
  subpasses.reserve(subpass_count_);

  std::vector<vk::SubpassDependency> subpass_dependencies;
  subpass_dependencies.reserve(subpass_count_);

  for (size_t i = 0; i < subpass_count_; i++) {
    // Setup the subpass.
    {
      vk::SubpassDescription subpass;
      subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
      subpass.setInputAttachments(color_refs);
      subpass.setColorAttachments(color_refs);
      subpass.setResolveAttachments(resolve_refs);
      subpass.setPDepthStencilAttachment(&depth_stencil_ref);
      subpasses.emplace_back(std::move(subpass));
    }

    // Wire up the dependencies from the subpass before and after this subpass.
    {
      const bool is_first_subpass = i == 0u;
      const bool is_last_subpass = i == subpass_count_ - 1u;
      const bool is_sole_subpass = is_first_subpass && is_last_subpass;

      if (is_sole_subpass) {
        // Don't bother adding dependencies from the external passes if this is
        // the only subpass. The defaults are fine.
        break;
      }

      {
        vk::SubpassDependency dep;

        dep.srcSubpass = is_first_subpass ? VK_SUBPASS_EXTERNAL : i - 1;
        dep.dstSubpass = i;

        // All dependencies are framebuffer local.
        dep.dependencyFlags = vk::DependencyFlagBits::eByRegion;

        // All commands before this barrier must proceed till at least the
        // color-attachment writes in the color-attachment output pipeline
        // stage are executed. And, all commands after this barrier may
        // continue till they encounter an input-attachment read in the
        // fragment shader pipeline stage.
        dep.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        dep.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        dep.dstStageMask = vk::PipelineStageFlagBits::eFragmentShader;
        dep.dstAccessMask = vk::AccessFlagBits::eInputAttachmentRead;

        subpass_dependencies.emplace_back(std::move(dep));
      }
    }
  }

  vk::RenderPassCreateInfo render_pass_desc;
  render_pass_desc.setAttachments(attachments);
  render_pass_desc.setSubpasses(subpasses);
  render_pass_desc.setDependencies(subpass_dependencies);

  auto [result, pass] = device.createRenderPassUnique(render_pass_desc);
  if (result != vk::Result::eSuccess) {
    VALIDATION_LOG << "Failed to create render pass: " << vk::to_string(result);
    return {};
  }
  return std::move(pass);
}

RenderPassBuilderVK& RenderPassBuilderVK::SetSubpassCount(size_t subpasses) {
  subpass_count_ = std::max<size_t>(1u, subpasses);
  return *this;
}

}  // namespace impeller

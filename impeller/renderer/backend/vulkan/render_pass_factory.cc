// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/vulkan/render_pass_factory.h"

#include <map>
#include <optional>
#include <set>
#include <string>

#include "flutter/fml/build_config.h"
#include "flutter/fml/trace_event.h"
#include "impeller/base/validation.h"
#include "impeller/renderer/backend/vulkan/allocator_vk.h"
#include "impeller/renderer/backend/vulkan/capabilities_vk.h"
#include "impeller/renderer/backend/vulkan/formats_vk.h"
#include "impeller/renderer/backend/vulkan/swapchain_vk.h"
#include "impeller/renderer/backend/vulkan/vk.h"

namespace impeller::renderpass {

static vk::AttachmentDescription CreatePlaceholderAttachmentDescription(
    vk::Format format,
    SampleCount sample_count) {
  vk::AttachmentDescription desc;

  // See
  // https://www.khronos.org/registry/vulkan/specs/1.3-extensions/html/chap8.html#renderpass-compatibility

  // Format and samples must match for sub-pass compatibility
  desc.setFormat(format);
  desc.setSamples(ToVKSampleCountFlagBits(sample_count));

  // The rest of these are placeholders and the right values will be set when
  // the render-pass to be used with the framebuffer is created.
  desc.setLoadOp(vk::AttachmentLoadOp::eDontCare);
  desc.setStoreOp(vk::AttachmentStoreOp::eDontCare);
  desc.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
  desc.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
  desc.setInitialLayout(vk::ImageLayout::eUndefined);
  desc.setFinalLayout(vk::ImageLayout::eGeneral);

  return desc;
}

//----------------------------------------------------------------------------
/// Render Pass
/// We are NOT going to use the same render pass with the framebuffer (later)
/// and the graphics pipeline (here). Instead, we are going to ensure that the
/// sub-passes are compatible. To see the compatibility rules, see the Vulkan
/// spec:
/// https://www.khronos.org/registry/vulkan/specs/1.3-extensions/html/chap8.html#renderpass-compatibility
/// TODO(106378): Add a format specifier to the ColorAttachmentDescriptor,
///               StencilAttachmentDescriptor, and, DepthAttachmentDescriptor.
///               Right now, these are placeholders.
///
std::optional<vk::UniqueRenderPass> CreateRenderPass(vk::Device device,
                                                     SampleCount sample_count,
                                                     std::string_view label) {
  std::vector<vk::AttachmentDescription> render_pass_attachments;

  std::vector<vk::AttachmentReference> color_attachment_references;
  std::vector<vk::AttachmentReference> resolve_attachment_references;
  std::optional<vk::AttachmentReference> depth_stencil_attachment_reference;

  // Set the color attachment.
  render_pass_attachments.push_back(CreatePlaceholderAttachmentDescription(
      vk::Format::eR8G8B8A8Unorm, sample_count));
  // TODO (kaushikiska): consider changing the image layout to
  // eColorAttachmentOptimal.
  color_attachment_references.push_back(vk::AttachmentReference(
      render_pass_attachments.size() - 1u, vk::ImageLayout::eGeneral));

  // Set the resolve attachment if MSAA is enabled.
  //   if (sample_count != SampleCount::kCount1) {
  //     render_pass_attachments.push_back(CreatePlaceholderAttachmentDescription(
  //         vk::Format::eR8G8B8A8Unorm, SampleCount::kCount1));
  //     resolve_attachment_references.push_back(vk::AttachmentReference(
  //         render_pass_attachments.size() - 1u, vk::ImageLayout::eGeneral));
  //   }

  //   if (desc.HasStencilAttachmentDescriptors()) {
  //     render_pass_attachments.push_back(CreatePlaceholderAttachmentDescription(
  //         vk::Format::eS8Uint, sample_count));
  //     depth_stencil_attachment_reference = vk::AttachmentReference(
  //         render_pass_attachments.size() - 1u, vk::ImageLayout::eGeneral);
  //   }

  vk::SubpassDescription subpass_info;
  subpass_info.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
  subpass_info.setColorAttachments(color_attachment_references);
  //   subpass_info.setResolveAttachments(resolve_attachment_references);
  //   if (depth_stencil_attachment_reference.has_value()) {
  //     subpass_info.setPDepthStencilAttachment(
  //         &depth_stencil_attachment_reference.value());
  //   }

  vk::RenderPassCreateInfo render_pass_info;
  render_pass_info.setSubpasses(subpass_info);
  render_pass_info.setSubpassCount(1);
  render_pass_info.setAttachments(render_pass_attachments);
  auto render_pass = device.createRenderPassUnique(render_pass_info);
  if (render_pass.result != vk::Result::eSuccess) {
    VALIDATION_LOG << "Could not create render pass for pipeline " << label
                   << ": " << vk::to_string(render_pass.result);
    return std::nullopt;
  }

  return std::move(render_pass.value);
}

}  // namespace impeller::renderpass

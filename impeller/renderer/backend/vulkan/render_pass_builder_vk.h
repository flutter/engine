// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_RENDERER_BACKEND_VULKAN_RENDER_PASS_BUILDER_VK_H_
#define FLUTTER_IMPELLER_RENDERER_BACKEND_VULKAN_RENDER_PASS_BUILDER_VK_H_

#include <map>
#include <optional>

#include "impeller/core/formats.h"
#include "impeller/renderer/backend/vulkan/context_vk.h"
#include "impeller/renderer/backend/vulkan/vk.h"

namespace impeller {

class RenderPassBuilderVK {
 public:
  RenderPassBuilderVK();

  ~RenderPassBuilderVK();

  RenderPassBuilderVK(const RenderPassBuilderVK&) = delete;

  RenderPassBuilderVK& operator=(const RenderPassBuilderVK&) = delete;

  RenderPassBuilderVK& SetSubpassCount(size_t subpasses);

  RenderPassBuilderVK& SetColorAttachment(size_t index,
                                          PixelFormat format,
                                          SampleCount sample_count,
                                          LoadAction load_action,
                                          StoreAction store_action);

  RenderPassBuilderVK& SetDepthStencilAttachment(PixelFormat format,
                                                 SampleCount sample_count,
                                                 LoadAction load_action,
                                                 StoreAction store_action);

  RenderPassBuilderVK& SetStencilAttachment(PixelFormat format,
                                            SampleCount sample_count,
                                            LoadAction load_action,
                                            StoreAction store_action);

  vk::UniqueRenderPass Build(const vk::Device& device) const;

 private:
  std::map<size_t, vk::AttachmentDescription> colors_;
  std::map<size_t, vk::AttachmentDescription> resolves_;
  std::optional<vk::AttachmentDescription> depth_stencil_;
  size_t subpass_count_ = 1u;
};

}  // namespace impeller

#endif  // FLUTTER_IMPELLER_RENDERER_BACKEND_VULKAN_RENDER_PASS_BUILDER_VK_H_

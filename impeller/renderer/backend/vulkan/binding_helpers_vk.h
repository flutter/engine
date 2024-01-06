// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_RENDERER_BACKEND_VULKAN_BINDING_HELPERS_VK_H_
#define FLUTTER_IMPELLER_RENDERER_BACKEND_VULKAN_BINDING_HELPERS_VK_H_

#include <vector>

#include "fml/status_or.h"
#include "impeller/renderer/backend/vulkan/context_vk.h"
#include "impeller/renderer/backend/vulkan/texture_vk.h"
#include "impeller/renderer/command.h"
#include "impeller/renderer/compute_pass.h"
#include "impeller/renderer/render_pass.h"

namespace impeller {

fml::StatusOr<std::vector<vk::DescriptorSet>> AllocateAndBindDescriptorSets(
    const ContextVK& context,
    const std::shared_ptr<CommandEncoderVK>& encoder,
    const std::vector<BoundCommand>& commands,
    const std::vector<BoundBuffer>& bound_buffers,
    const std::vector<BoundTexture>& bound_textures,
    const TextureVK& input_attachment);

fml::StatusOr<std::vector<vk::DescriptorSet>> AllocateAndBindDescriptorSets(
    const ContextVK& context,
    const std::shared_ptr<CommandEncoderVK>& encoder,
    const std::vector<BoundComputeCommand>& commands,
    const std::vector<BoundBuffer>& bound_buffers,
    const std::vector<BoundTexture>& bound_textures);

}  // namespace impeller

#endif  // FLUTTER_IMPELLER_RENDERER_BACKEND_VULKAN_BINDING_HELPERS_VK_H_

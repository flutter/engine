// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/impeller/golden_tests/vulkan_screenshotter.h"

#include "impeller/renderer/backend/vulkan/surface_context_vk.h"
#include "impeller/renderer/backend/vulkan/texture_vk.h"
#define GLFW_INCLUDE_NONE
#include "third_party/glfw/include/GLFW/glfw3.h"

namespace impeller {
namespace testing {

namespace {
std::unique_ptr<Screenshot> ReadTexture(
    const std::shared_ptr<SurfaceContextVK>& surface_context,
    const std::shared_ptr<TextureVK>& texture) {
  vk::BufferCreateInfo buffer_create_info;
  const int bpp = 4;
  buffer_create_info.size =
      texture->GetSize().width * texture->GetSize().height * bpp;
  buffer_create_info.usage = vk::BufferUsageFlagBits::eTransferDst;
  buffer_create_info.sharingMode = vk::SharingMode::eExclusive;

  auto buffer_result = surface_context->GetDevice().createBufferUnique(
      buffer_create_info, nullptr);
  FML_CHECK(buffer_result.result == vk::Result::eSuccess);

  vk::MemoryRequirements memory_requirements;
  surface_context->GetDevice().getBufferMemoryRequirements(
      buffer_result.value.get(), &memory_requirements);

  vk::MemoryAllocateInfo alloc_info;
  alloc_info.allocationSize = memory_requirements.size;
  // alloc_info.memoryTypeIndex =
  auto memory_result =
      surface_context->GetDevice().allocateMemoryUnique(alloc_info, nullptr);
  FML_CHECK(memory_result.result == vk::Result::eSuccess);

  return {};
}
}  // namespace

VulkanScreenshotter::VulkanScreenshotter() {
  FML_CHECK(::glfwInit() == GLFW_TRUE);
  playground_ =
      PlaygroundImpl::Create(PlaygroundBackend::kVulkan, PlaygroundSwitches{});
}

std::unique_ptr<Screenshot> VulkanScreenshotter::MakeScreenshot(
    AiksContext& aiks_context,
    const Picture& picture,
    const ISize& size,
    bool scale_content) {
  Vector2 content_scale =
      scale_content ? playground_->GetContentScale() : Vector2{1, 1};
  std::shared_ptr<Image> image = picture.ToImage(
      aiks_context,
      ISize(size.width * content_scale.x, size.height * content_scale.y));
  std::shared_ptr<Texture> texture = image->GetTexture();
  FML_CHECK(aiks_context.GetContext()->GetBackendType() ==
            Context::BackendType::kVulkan);
  std::shared_ptr<SurfaceContextVK> surface_context =
      std::static_pointer_cast<SurfaceContextVK>(aiks_context.GetContext());
  std::shared_ptr<TextureVK> vulkan_texture =
      std::reinterpret_pointer_cast<TextureVK>(texture);
  // TODO(gaaclarke)
  std::unique_ptr<Screenshot> result =
      ReadTexture(surface_context, vulkan_texture);
  FML_CHECK(result);
  return result;
}

}  // namespace testing
}  // namespace impeller

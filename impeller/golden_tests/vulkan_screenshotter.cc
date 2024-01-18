// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/impeller/golden_tests/vulkan_screenshotter.h"

#define GLFW_INCLUDE_NONE
#include "third_party/glfw/include/GLFW/glfw3.h"

namespace impeller {
namespace testing {

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
  // TODO(gaaclarke)
  FML_CHECK(false);
  return {};
}

}  // namespace testing
}  // namespace impeller

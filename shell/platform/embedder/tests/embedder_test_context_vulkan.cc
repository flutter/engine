// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/embedder/tests/embedder_test_context_vulkan.h"

#include <memory>

#include "embedder.h"
#include "flutter/fml/logging.h"
// #include
// "flutter/shell/platform/embedder/tests/embedder_test_compositor_vulkan.h"
#include "flutter/vulkan/vulkan_device.h"
#include "flutter/vulkan/vulkan_proc_table.h"
#include "testing/test_vulkan_context.h"
#include "testing/test_vulkan_surface.h"
#include "third_party/skia/include/core/SkSurface.h"

namespace flutter {
namespace testing {

EmbedderTestContextVulkan::EmbedderTestContextVulkan(std::string assets_path)
    : EmbedderTestContext(assets_path),
      context_(std::make_unique<TestVulkanContext>()),
      surface_() {}

EmbedderTestContextVulkan::~EmbedderTestContextVulkan() {}

void EmbedderTestContextVulkan::SetupSurface(SkISize surface_size) {
  FML_CHECK(surface_size_.isEmpty());
  surface_size_ = surface_size;
  surface_ = TestVulkanSurface::Create(*context_, surface_size_);
}

size_t EmbedderTestContextVulkan::GetSurfacePresentCount() const {
  return present_count_;
}

VkImage EmbedderTestContextVulkan::GetNextImage(const SkISize& size) {
  assert(false);  // TODO(bdero)
  return nullptr;
}

bool EmbedderTestContextVulkan::PresentImage(VkImage image) {
  assert(false);  // TODO(bdero)
  return false;
}

EmbedderTestContextType EmbedderTestContextVulkan::GetContextType() const {
  return EmbedderTestContextType::kVulkanContext;
}

void EmbedderTestContextVulkan::SetupCompositor() {
  FML_CHECK(!compositor_) << "Already set up a compositor in this context.";

  assert(false);  // TODO(bdero)
  // FML_CHECK(vulkan_surface_)
  //     << "Set up the Vulkan surface before setting up a compositor.";
  // compositor_ = std::make_unique<EmbedderTestCompositorVulkan>(
  //     surface_size_, vulkan_surface_->GetGrContext());
}

}  // namespace testing
}  // namespace flutter

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

#ifdef OS_MACOSX
#define VULKAN_SO_PATH "libvk_swiftshader.dylib"
#elif OS_WIN
#define VULKAN_SO_PATH "vk_swiftshader.dll"
#else
#define VULKAN_SO_PATH "libvk_swiftshader.so"
#endif

namespace flutter {
namespace testing {

EmbedderTestContextVulkan::EmbedderTestContextVulkan(std::string assets_path)
    : EmbedderTestContext(assets_path) {
  vk_ = fml::MakeRefCounted<vulkan::VulkanProcTable>(VULKAN_SO_PATH);
  if (!vk_ || !vk_->HasAcquiredMandatoryProcAddresses()) {
    FML_DLOG(ERROR) << "Proc table has not acquired mandatory proc addresses.";
    return;
  }

  application_ = std::unique_ptr<vulkan::VulkanApplication>(
      new vulkan::VulkanApplication(*vk_, "Flutter Unittests", {}));
  if (!application_->IsValid()) {
    FML_DLOG(ERROR) << "Failed to initialize basic Vulkan state.";
    return;
  }
  if (!vk_->AreInstanceProcsSetup()) {
    FML_DLOG(ERROR) << "Failed to acquire full proc table.";
    return;
  }

  logical_device_ = application_->AcquireFirstCompatibleLogicalDevice();
  if (!logical_device_ || !logical_device_->IsValid()) {
    FML_DLOG(ERROR) << "Failed to create compatible logical device.";
    return;
  }
}

EmbedderTestContextVulkan::~EmbedderTestContextVulkan() {}

void EmbedderTestContextVulkan::SetupSurface(SkISize surface_size) {
  FML_CHECK(surface_size_.isEmpty());
  surface_size_ = surface_size;

  assert(false);  // TODO(bdero)
  // vulkan_surface_ = TestVulkanSurface::Create(*vulkan_context_,
  // surface_size_);
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

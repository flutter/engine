// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cassert>

#include "flutter/fml/logging.h"
#include "flutter/shell/common/context_options.h"
#include "flutter/testing/test_vulkan_context.h"

#include "include/gpu/GrDirectContext.h"
#include "include/gpu/vk/GrVkExtensions.h"
#include "third_party/skia/include/core/SkSurface.h"

#ifdef OS_MACOSX
#define VULKAN_SO_PATH "libvk_swiftshader.dylib"
#elif OS_WIN
#define VULKAN_SO_PATH "vk_swiftshader.dll"
#else
#define VULKAN_SO_PATH "libvk_swiftshader.so"
#endif

namespace flutter {
namespace testing {

TestVulkanContext::TestVulkanContext() {
  // ---------------------------------------------------------------------------
  // Initialize basic Vulkan state using the Swiftshader ICD.
  // ---------------------------------------------------------------------------

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

  device_ = application_->AcquireFirstCompatibleLogicalDevice();
  if (!device_ || !device_->IsValid()) {
    FML_DLOG(ERROR) << "Failed to create compatible logical device.";
    return;
  }

  // ---------------------------------------------------------------------------
  // Create a Skia context.
  // For creating SkSurfaces from VkImages and snapshotting them, etc.
  // ---------------------------------------------------------------------------

  uint32_t skia_features = 0;
  if (!device_->GetPhysicalDeviceFeaturesSkia(&skia_features)) {
    FML_LOG(ERROR) << "Failed to get physical device features.";

    return;
  }

  auto get_proc = vk_->CreateSkiaGetProc();
  if (get_proc == nullptr) {
    FML_LOG(ERROR) << "Failed to create Vulkan getProc for Skia.";
    return;
  }

  GrVkExtensions extensions;

  GrVkBackendContext backend_context = {};
  backend_context.fInstance = application_->GetInstance();
  backend_context.fPhysicalDevice = device_->GetPhysicalDeviceHandle();
  backend_context.fDevice = device_->GetHandle();
  backend_context.fQueue = device_->GetQueueHandle();
  backend_context.fGraphicsQueueIndex = device_->GetGraphicsQueueIndex();
  backend_context.fMinAPIVersion = VK_MAKE_VERSION(1, 0, 0);
  backend_context.fMaxAPIVersion = VK_MAKE_VERSION(1, 0, 0);
  backend_context.fFeatures = skia_features;
  backend_context.fVkExtensions = &extensions;
  backend_context.fGetProc = get_proc;
  backend_context.fOwnsInstanceAndDevice = false;

  GrContextOptions options =
      MakeDefaultContextOptions(ContextType::kRender, GrBackendApi::kVulkan);
  options.fReduceOpsTaskSplitting = GrContextOptions::Enable::kNo;
  context_ = GrDirectContext::MakeVulkan(backend_context, options);
}

VkImage TestVulkanContext::CreateImage(const SkISize& size) const {
  assert(false);  // TODO(bdero)
  return nullptr;
}

sk_sp<GrDirectContext> TestVulkanContext::GetGrDirectContext() const {
  return context_;
}

}  // namespace testing
}  // namespace flutter

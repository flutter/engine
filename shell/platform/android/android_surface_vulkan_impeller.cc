// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/android/android_surface_vulkan_impeller.h"

#include <memory>
#include <utility>

#include "flutter/fml/logging.h"
#include "flutter/impeller/renderer/backend/vulkan/context_vk.h"
#include "flutter/shell/gpu/gpu_surface_vulkan.h"
#include "flutter/vulkan/vulkan_native_surface_android.h"
#include "fml/memory/ref_ptr.h"
#include "impeller/entity/vk/entity_shaders_vk.h"

namespace flutter {

std::shared_ptr<impeller::Context> CreateImpellerContext(
    fml::RefPtr<vulkan::VulkanProcTable> proc_table) {
  if (!proc_table->IsValid()) {
    FML_LOG(ERROR) << "Invalid Vulkan Proc Table.";
    return nullptr;
  }

  std::vector<std::shared_ptr<fml::Mapping>> shader_mappings = {
      std::make_shared<fml::NonOwnedMapping>(impeller_entity_shaders_vk_data,
                                             impeller_entity_shaders_vk_length),
  };

  // auto context = ContextVK::Create(reinterpret_cast<PFN_vkGetInstanceProcAddr>(
  //                                      &::glfwGetInstanceProcAddress),  //
  //                                  shader_mappings,                     //
  //                                  nullptr,                             //
  //                                  concurrent_loop_->GetTaskRunner(),   //
  //                                  "Android Impeller Vulkan Lib"        //
  // );

  return nullptr;
}

AndroidSurfaceVulkanImpeller::AndroidSurfaceVulkanImpeller(
    const std::shared_ptr<AndroidContext>& android_context,
    std::shared_ptr<PlatformViewAndroidJNI> jni_facade)
    : AndroidSurface(android_context),
      proc_table_(fml::MakeRefCounted<vulkan::VulkanProcTable>()) {}

AndroidSurfaceVulkanImpeller::~AndroidSurfaceVulkanImpeller() = default;

bool AndroidSurfaceVulkanImpeller::IsValid() const {
  return proc_table_->HasAcquiredMandatoryProcAddresses();
}

void AndroidSurfaceVulkanImpeller::TeardownOnScreenContext() {
  // Nothing to do.
}

std::unique_ptr<Surface> AndroidSurfaceVulkanImpeller::CreateGPUSurface(
    GrDirectContext* gr_context) {
  if (!IsValid()) {
    return nullptr;
  }

  if (!native_window_ || !native_window_->IsValid()) {
    return nullptr;
  }

  auto vulkan_surface_android =
      std::make_unique<vulkan::VulkanNativeSurfaceAndroid>(
          native_window_->handle());

  if (!vulkan_surface_android->IsValid()) {
    return nullptr;
  }

  sk_sp<GrDirectContext> provided_gr_context;
  if (gr_context) {
    provided_gr_context = sk_ref_sp(gr_context);
  } else if (android_context_->GetMainSkiaContext()) {
    provided_gr_context = android_context_->GetMainSkiaContext();
  }

  std::unique_ptr<GPUSurfaceVulkan> gpu_surface;
  // if (provided_gr_context) {
  //   gpu_surface = std::make_unique<GPUSurfaceVulkan>(
  //       provided_gr_context, this, std::move(vulkan_surface_android), true);
  // } else {
  //   gpu_surface = std::make_unique<GPUSurfaceVulkan>(
  //       this, std::move(vulkan_surface_android), true);
  //   android_context_->SetMainSkiaContext(sk_ref_sp(gpu_surface->GetContext()));
  // }

  if (!gpu_surface->IsValid()) {
    return nullptr;
  }

  return gpu_surface;
}

bool AndroidSurfaceVulkanImpeller::OnScreenSurfaceResize(const SkISize& size) {
  return true;
}

bool AndroidSurfaceVulkanImpeller::ResourceContextMakeCurrent() {
  FML_DLOG(ERROR) << "The vulkan backend does not support resource contexts.";
  return false;
}

bool AndroidSurfaceVulkanImpeller::ResourceContextClearCurrent() {
  FML_DLOG(ERROR) << "The vulkan backend does not support resource contexts.";
  return false;
}

bool AndroidSurfaceVulkanImpeller::SetNativeWindow(
    fml::RefPtr<AndroidNativeWindow> window) {
  native_window_ = std::move(window);
  return native_window_ && native_window_->IsValid();
}

const vulkan::VulkanProcTable& AndroidSurfaceVulkanImpeller::vk() {
  return *proc_table_;
}

}  // namespace flutter

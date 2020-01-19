// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/vulkan/vulkan_native_surface_embedder.h"

namespace vulkan {

VulkanNativeSurfaceEmbedder::VulkanNativeSurfaceEmbedder(
    VulkanDispatchTable dispatch_table)
    : dispatch_table_(dispatch_table) {
  if (!dispatch_table_.get_size) {
    return;
  }
  if (!dispatch_table_.get_extension_name) {
    return;
  }
  if (!dispatch_table_.get_skia_extension_name) {
    return;
  }
  if (!dispatch_table_.create_surface_handle) {
    return;
  }
  valid_ = true;
}

VulkanNativeSurfaceEmbedder::~VulkanNativeSurfaceEmbedder() = default;

const char* VulkanNativeSurfaceEmbedder::GetExtensionName() const {
  return dispatch_table_.get_extension_name();
}

uint32_t VulkanNativeSurfaceEmbedder::GetSkiaExtensionName() const {
  return dispatch_table_.get_skia_extension_name();
}

VkSurfaceKHR VulkanNativeSurfaceEmbedder::CreateSurfaceHandle(
    VulkanProcTable& vk,
    const VulkanHandle<VkInstance>& instance) const {
  return dispatch_table_.create_surface_handle(vk, instance);
}

bool VulkanNativeSurfaceEmbedder::IsValid() const {
  return valid_;
}

SkISize VulkanNativeSurfaceEmbedder::GetSize() const {
  return dispatch_table_.get_size();
}

}  // namespace vulkan

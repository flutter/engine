// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "flutter/fml/macros.h"
#include "flutter/fml/memory/ref_ptr.h"
#include "flutter/vulkan/procs/vulkan_proc_table.h"
#include "impeller/core/allocator.h"
#include "impeller/renderer/backend/vulkan/context_vk.h"
#include "impeller/renderer/backend/vulkan/device_buffer_vk.h"
#include "impeller/renderer/backend/vulkan/device_holder.h"
#include "impeller/renderer/backend/vulkan/vk.h"

#include <memory>

namespace impeller {

class AllocatorVK final : public Allocator {
 public:
  // |Allocator|
  ~AllocatorVK() override;

 private:
  friend class ContextVK;

  fml::RefPtr<vulkan::VulkanProcTable> vk_;
  VmaAllocator allocator_ = {};

  // Staging buffer pools.
  VmaPool staging_buffer_pools_[3] = {};
  // Render target buffer pools.
  VmaPool render_target_buffer_pools_[3] = {};

  std::weak_ptr<Context> context_;
  std::weak_ptr<DeviceHolder> device_holder_;
  ISize max_texture_size_;
  bool is_valid_ = false;
  bool supports_memoryless_textures_ = false;
  uint32_t frame_index_ = 0u;

  AllocatorVK(std::weak_ptr<Context> context,
              uint32_t vulkan_api_version,
              const vk::PhysicalDevice& physical_device,
              const std::shared_ptr<DeviceHolder>& device_holder,
              const vk::Instance& instance,
              PFN_vkGetInstanceProcAddr get_instance_proc_address,
              PFN_vkGetDeviceProcAddr get_device_proc_address,
              const CapabilitiesVK& capabilities);

  // |Allocator|
  bool IsValid() const;

  // |Allocator|
  std::shared_ptr<DeviceBuffer> OnCreateBuffer(
      const DeviceBufferDescriptor& desc) override;

  // |Allocator|
  std::shared_ptr<Texture> OnCreateTexture(
      const TextureDescriptor& desc) override;

  // |Allocator|
  ISize GetMaxTextureSizeSupported() const override;

  // |Allocator|
  void IncrementFrameIndex() override {
    frame_index_ += 1;
  }

  static bool CreateBufferPool(VmaAllocator allocator, VmaPool* pool);

  static bool CreateRenderTargetPool(VmaAllocator allocator, VmaPool* pool);

  FML_DISALLOW_COPY_AND_ASSIGN(AllocatorVK);
};

}  // namespace impeller

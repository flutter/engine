// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_RENDERER_BACKEND_VULKAN_ANDROID_HARDWARE_BUFFER_SWAPCHAIN_IMAGE_VK_H_
#define FLUTTER_IMPELLER_RENDERER_BACKEND_VULKAN_ANDROID_HARDWARE_BUFFER_SWAPCHAIN_IMAGE_VK_H_

#include <android/hardware_buffer.h>
#include <memory>

#include "impeller/core/texture_descriptor.h"
#include "impeller/renderer/backend/vulkan/android_hardware_buffer_texture_source_vk.h"
#include "impeller/renderer/backend/vulkan/swapchain_image_vk.h"

namespace impeller {

class AndroidHardwareBufferSwapchainImageVK : public SwapchainImageVK {
 public:
  static std::shared_ptr<AndroidHardwareBufferSwapchainImageVK> Create(
      TextureDescriptor desc,
      const vk::Device& device);

  ~AndroidHardwareBufferSwapchainImageVK() override;

  bool IsValid() const override;

 private:
  AndroidHardwareBufferSwapchainImageVK(
      AHardwareBuffer* hardware_buffer,
      TextureDescriptor desc,
      const vk::Device& device,
      std::unique_ptr<AndroidHardwareBufferTextureSourceVK> texture_source);

  AHardwareBuffer* hardware_buffer_;
  std::unique_ptr<AndroidHardwareBufferTextureSourceVK> texture_source_;

  AndroidHardwareBufferSwapchainImageVK(
      const AndroidHardwareBufferSwapchainImageVK&) = delete;

  AndroidHardwareBufferSwapchainImageVK& operator=(
      const AndroidHardwareBufferSwapchainImageVK&) = delete;
};

}  // namespace impeller

#endif  // FLUTTER_IMPELLER_RENDERER_BACKEND_VULKAN_ANDROID_HARDWARE_BUFFER_SWAPCHAIN_IMAGE_VK_H_

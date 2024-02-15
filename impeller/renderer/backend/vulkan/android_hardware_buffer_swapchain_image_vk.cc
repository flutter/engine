// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/vulkan/android_hardware_buffer_swapchain_image_vk.h"

#include <optional>

#include "fml/platform/android/ndk_helpers.h"
#include "impeller/base/validation.h"
#include "impeller/core/texture_descriptor.h"
#include "impeller/renderer/backend/vulkan/swapchain_image_vk.h"

namespace impeller {

static std::optional<AHardwareBuffer_Format> ToAHardwareBufferFormat(
    const PixelFormat format) {
  switch (format) {
    case PixelFormat::kR8UNormInt:
      return AHardwareBuffer_Format::AHARDWAREBUFFER_FORMAT_R8_UNORM;
    case PixelFormat::kR8G8B8A8UNormInt:
      return AHardwareBuffer_Format::AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM;
    case PixelFormat::kR16G16B16A16Float:
      return AHardwareBuffer_Format::AHARDWAREBUFFER_FORMAT_R16G16B16A16_FLOAT;

    case PixelFormat::kUnknown:
    case PixelFormat::kA8UNormInt:
    case PixelFormat::kR8G8UNormInt:
    case PixelFormat::kR8G8B8A8UNormIntSRGB:
    case PixelFormat::kB8G8R8A8UNormInt:
    case PixelFormat::kB8G8R8A8UNormIntSRGB:
    case PixelFormat::kR32G32B32A32Float:
    case PixelFormat::kB10G10R10XR:
    case PixelFormat::kB10G10R10XRSRGB:
    case PixelFormat::kB10G10R10A10XR:
    case PixelFormat::kS8UInt:
    case PixelFormat::kD24UnormS8Uint:
    case PixelFormat::kD32FloatS8UInt:
      return std::nullopt;
  }
}

std::shared_ptr<AndroidHardwareBufferSwapchainImageVK>
AndroidHardwareBufferSwapchainImageVK::Create(TextureDescriptor desc,
                                              const vk::Device& device) {
  std::optional<AHardwareBuffer_Format> format =
      ToAHardwareBufferFormat(desc.format);
  if (!format.has_value()) {
    VALIDATION_LOG << "Unsupported format " << PixelFormatToString(desc.format);
    return nullptr;
  }
  AHardwareBuffer_Desc buffer_desc{
      .width = static_cast<uint32_t>(desc.size.width),
      .height = static_cast<uint32_t>(desc.size.height),
      .layers = 1u,
      .format = format.value(),
      .usage =
          AHardwareBuffer_UsageFlags::AHARDWAREBUFFER_USAGE_CPU_READ_NEVER |
          AHardwareBuffer_UsageFlags::AHARDWAREBUFFER_USAGE_GPU_SAMPLED |
          AHardwareBuffer_UsageFlags::AHARDWAREBUFFER_USAGE_GPU_FRAMEBUFFER,
  };
  AHardwareBuffer* buffer;
  int allocate_result =
      flutter::NDKHelpers::AHardwareBuffer_allocate(&buffer_desc, &buffer);
  if (!allocate_result) {
    VALIDATION_LOG << "Failed to allocate AHardwareBuffer (" << desc.size
                   << ", " << PixelFormatToString(desc.format) << ").";
    return nullptr;
  }

  return std::shared_ptr<AndroidHardwareBufferSwapchainImageVK>(
      new AndroidHardwareBufferSwapchainImageVK(
          buffer, desc, device,
          std::make_unique<AndroidHardwareBufferTextureSourceVK>(
              desc, device, buffer, buffer_desc)));
}

AndroidHardwareBufferSwapchainImageVK::AndroidHardwareBufferSwapchainImageVK(
    AHardwareBuffer* hardware_buffer,
    TextureDescriptor desc,
    const vk::Device& device,
    std::unique_ptr<AndroidHardwareBufferTextureSourceVK> texture_source)
    : SwapchainImageVK(desc, device, texture_source->GetImage()),
      hardware_buffer_(hardware_buffer),
      texture_source_(std::move(texture_source)) {}

AndroidHardwareBufferSwapchainImageVK::
    ~AndroidHardwareBufferSwapchainImageVK() {
  texture_source_.reset();
  flutter::NDKHelpers::AHardwareBuffer_release(hardware_buffer_);
}

bool AndroidHardwareBufferSwapchainImageVK::IsValid() const {
  return texture_source_->IsValid() && SwapchainImageVK::IsValid();
}

}  // namespace impeller

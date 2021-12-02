// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/fml/macros.h"
#include "flutter/vulkan/vulkan_handle.h"

#include "third_party/skia/include/core/SkSize.h"

namespace flutter {
namespace testing {

class TestVulkanContext;

/// Captures the lifetime of a test VkImage along with its bound memory.
class TestVulkanImage {
 public:
  TestVulkanImage(TestVulkanImage&& other);
  TestVulkanImage& operator=(TestVulkanImage&& other);

  ~TestVulkanImage();

  VkImage GetImage();

 private:
  TestVulkanImage();

  vulkan::VulkanHandle<VkImage> image_;
  vulkan::VulkanHandle<VkDeviceMemory> memory_;

  FML_DISALLOW_COPY_AND_ASSIGN(TestVulkanImage);

  friend TestVulkanContext;
};

}  // namespace testing
}  // namespace flutter

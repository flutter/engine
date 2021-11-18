// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_EMBEDDER_TESTS_EMBEDDER_CONTEXT_VULKAN_H_
#define FLUTTER_SHELL_PLATFORM_EMBEDDER_TESTS_EMBEDDER_CONTEXT_VULKAN_H_

#include "tests/embedder_test_context.h"
#include "vulkan/vulkan_application.h"

namespace flutter {
namespace testing {

class EmbedderTestContextVulkan : public EmbedderTestContext {
 public:
  explicit EmbedderTestContextVulkan(std::string assets_path = "");

  ~EmbedderTestContextVulkan() override;

  // |EmbedderTestContext|
  EmbedderTestContextType GetContextType() const override;

  // |EmbedderTestContext|
  size_t GetSurfacePresentCount() const override;

  // |EmbedderTestContext|
  void SetupCompositor() override;

  VkImage GetNextImage(const SkISize& size);

  bool PresentImage(VkImage image);

 private:
  // This allows the builder to access the hooks.
  friend class EmbedderConfigBuilder;

  fml::RefPtr<vulkan::VulkanProcTable> vk_;
  std::unique_ptr<vulkan::VulkanApplication> application_;
  std::unique_ptr<vulkan::VulkanDevice> logical_device_;

  SkISize surface_size_ = SkISize::MakeEmpty();
  //std::unique_ptr<TestVulkanContext> vulkan_context_;
  size_t present_count_ = 0;

  void SetupSurface(SkISize surface_size) override;

  FML_DISALLOW_COPY_AND_ASSIGN(EmbedderTestContextVulkan);
};

}  // namespace testing
}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_EMBEDDER_TESTS_EMBEDDER_CONTEXT_VULKAN_H_

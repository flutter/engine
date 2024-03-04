// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/testing/testing.h"  // IWYU pragma: keep

#include "gtest/gtest.h"
#include "impeller/core/formats.h"
#include "impeller/renderer//backend/vulkan/command_encoder_vk.h"
#include "impeller/renderer/backend/vulkan/context_vk.h"
#include "impeller/renderer/backend/vulkan/device_buffer_vk.h"
#include "impeller/renderer/backend/vulkan/test/mock_vulkan.h"
#include "impeller/renderer/backend/vulkan/vma.h"

namespace impeller {
namespace testing {

TEST(AllocatorVK, CanCreatebuffer) {
  auto const context =
      MockVulkanContextBuilder()
          .SetSettingsCallback([](ContextVK::Settings& settings) {})
          .Build();
  auto allocator = context->GetResourceAllocator();
  auto buffer = allocator->CreateBuffer(
      {.storage_mode = StorageMode::kHostVisible, .size = 1});
  ASSERT_TRUE(buffer);

  buffer->Flush();
  buffer->Invalidate();

  auto const called = GetMockVulkanFunctions(context->GetDevice());
  EXPECT_EQ(
      std::find(called->begin(), called->end(), "vkFlushMappedMemoryRanges"),
      called->end());

  context->Shutdown();
}

}  // namespace testing
}  // namespace impeller

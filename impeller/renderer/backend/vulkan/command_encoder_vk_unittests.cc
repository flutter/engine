// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <thread>

#include "flutter/testing/testing.h"
#include "impeller/renderer/backend/vulkan/command_encoder_vk.h"
#include "impeller/renderer/backend/vulkan/test/mock_vulkan.h"

namespace impeller {
namespace testing {

TEST(CommandEncoderVKTest, DeleteEncoderAfterThreadDies) {
  std::shared_ptr<std::vector<std::string>> called_functions;
  {
    auto context = CreateMockVulkanContext();
    called_functions = GetMockVulkanFunctions(context->GetDevice());
    std::shared_ptr<CommandEncoderVK> encoder;
    std::thread thread([&] {
      CommandEncoderFactoryVK factory(context);
      encoder = factory.Create();
    });
    thread.join();
  }
  auto destroy_pool =
      std::find(called_functions->begin(), called_functions->end(),
                "vkDestroyCommandPool");
  auto free_buffers =
      std::find(called_functions->begin(), called_functions->end(),
                "vkFreeCommandBuffers");
  EXPECT_TRUE(destroy_pool != called_functions->end());
  EXPECT_TRUE(free_buffers != called_functions->end());
  EXPECT_TRUE(free_buffers < destroy_pool);
}

}  // namespace testing
}  // namespace impeller
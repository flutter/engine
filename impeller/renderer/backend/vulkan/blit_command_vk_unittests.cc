// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/testing/testing.h"
#include "impeller/renderer/backend/vulkan/blit_command_vk.h"
#include "impeller/renderer/backend/vulkan/command_encoder_vk.h"
#include "impeller/renderer/backend/vulkan/test/mock_vulkan.h"

namespace impeller {
namespace testing {

TEST(BlitCommandVkTest, BlitCopyTextureToTextureCommandVK) {
  ContextVK::Settings settings;
  auto message_loop = fml::ConcurrentMessageLoop::Create();
  settings.worker_task_runner =
      std::make_shared<fml::ConcurrentTaskRunner>(message_loop);
  settings.proc_address_callback = GetMockVulkanProcAddress;
  auto context = ContextVK::Create(std::move(settings));
  auto pool = CommandPoolVK::GetThreadLocal(context.get());
  CommandEncoderVK encoder(context->GetDevice(), context->GetGraphicsQueue(),
                           pool, context->GetFenceWaiter());
  BlitCopyTextureToTextureCommandVK cmd;
  cmd.source = context->GetResourceAllocator()->CreateTexture({
      .size = ISize(100, 100),
  });
  cmd.destination = context->GetResourceAllocator()->CreateTexture({
      .size = ISize(100, 100),
  });
  bool result = cmd.Encode(encoder);
  EXPECT_TRUE(result);
  EXPECT_TRUE(encoder.IsTracking(cmd.source));
  EXPECT_TRUE(encoder.IsTracking(cmd.destination));
}

// TEST(BlitCommandVkTest, BlitCopyTextureToBufferCommandVK) {
//   ContextVK::Settings settings;
//   auto message_loop = fml::ConcurrentMessageLoop::Create();
//   settings.worker_task_runner =
//       std::make_shared<fml::ConcurrentTaskRunner>(message_loop);
//   settings.proc_address_callback = GetMockVulkanProcAddress;
//   auto context = ContextVK::Create(std::move(settings));
//   auto pool = CommandPoolVK::GetThreadLocal(context.get());
//   CommandEncoderVK encoder(context->GetDevice(), context->GetGraphicsQueue(),
//                            pool, context->GetFenceWaiter());
//   BlitCopyTextureToBufferCommandVK cmd;
//   cmd.source = context->GetResourceAllocator()->CreateTexture({
//       .size = ISize(100, 100),
//   });
//   cmd.destination = context->GetResourceAllocator()->CreateTexture({
//       .size = ISize(100, 100),
//   });
//   bool result = cmd.Encode(encoder);
//   EXPECT_TRUE(result);
//   EXPECT_TRUE(encoder.IsTracking(cmd.source));
//   EXPECT_TRUE(encoder.IsTracking(cmd.destination));
// }

TEST(BlitCommandVkTest, BlitGenerateMipmapCommandVK) {
  ContextVK::Settings settings;
  auto message_loop = fml::ConcurrentMessageLoop::Create();
  settings.worker_task_runner =
      std::make_shared<fml::ConcurrentTaskRunner>(message_loop);
  settings.proc_address_callback = GetMockVulkanProcAddress;
  auto context = ContextVK::Create(std::move(settings));
  auto pool = CommandPoolVK::GetThreadLocal(context.get());
  CommandEncoderVK encoder(context->GetDevice(), context->GetGraphicsQueue(),
                           pool, context->GetFenceWaiter());
  BlitGenerateMipmapCommandVK cmd;
  cmd.texture = context->GetResourceAllocator()->CreateTexture({
      .size = ISize(100, 100),
      .mip_count = 2,
  });
  bool result = cmd.Encode(encoder);
  EXPECT_TRUE(result);
  EXPECT_TRUE(encoder.IsTracking(cmd.texture));
}

}  // namespace testing
}  // namespace impeller

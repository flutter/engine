// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/testing/testing.h"  // IWYU pragma: keep
#include "gtest/gtest.h"
#include "impeller/renderer//backend/vulkan/command_encoder_vk.h"
#include "impeller/renderer/backend/vulkan/command_buffer_vk.h"
#include "impeller/renderer/backend/vulkan/context_vk.h"
#include "impeller/renderer/backend/vulkan/gpu_tracer_vk.h"
#include "impeller/renderer/backend/vulkan/test/mock_vulkan.h"

namespace impeller {
namespace testing {

#ifdef IMPELLER_DEBUG
TEST(GPUTrackerVK, CanTraceCmdBuffer) {
  auto const context = MockVulkanContextBuilder().Build();

  auto tracer = std::make_shared<GPUTracerVK>(context->GetDeviceHolder());

  ASSERT_TRUE(tracer->IsValid());

  auto cmd_buffer = context->CreateCommandBuffer();
  auto vk_cmd_buffer = CommandBufferVK::Cast(cmd_buffer.get());
  auto blit_ass = cmd_buffer->CreateBlitPass();

  tracer->MarkFrameStart();
  tracer->RecordCmdBufferStart(vk_cmd_buffer->GetEncoder()->GetCommandBuffer());
  auto frame_id = tracer->RecordCmdBufferEnd(
      vk_cmd_buffer->GetEncoder()->GetCommandBuffer());
  tracer->MarkFrameEnd();

  ASSERT_EQ(frame_id, 0u);

  tracer->OnFenceComplete(frame_id);
}
#endif  // IMPELLER_DEBUG

}  // namespace testing
}  // namespace impeller

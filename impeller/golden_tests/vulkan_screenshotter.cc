// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/impeller/golden_tests/vulkan_screenshotter.h"

#include "flutter/fml/synchronization/waitable_event.h"
#include "flutter/impeller/golden_tests/libpng_screenshot.h"
#define GLFW_INCLUDE_NONE
#include "third_party/glfw/include/GLFW/glfw3.h"

namespace impeller {
namespace testing {

namespace {

std::unique_ptr<Screenshot> ReadTexture(
    const std::shared_ptr<Context>& surface_context,
    const std::shared_ptr<Texture>& texture) {
  DeviceBufferDescriptor buffer_desc;
  buffer_desc.storage_mode = StorageMode::kHostVisible;
  buffer_desc.size =
      texture->GetTextureDescriptor().GetByteSizeOfBaseMipLevel();
  buffer_desc.readback = true;
  std::shared_ptr<DeviceBuffer> device_buffer =
      surface_context->GetResourceAllocator()->CreateBuffer(buffer_desc);
  FML_CHECK(device_buffer);

  auto command_buffer = surface_context->CreateCommandBuffer();
  auto blit_pass = command_buffer->CreateBlitPass();
  bool success = blit_pass->AddCopy(texture, device_buffer);
  FML_CHECK(success);

  success = blit_pass->EncodeCommands(surface_context->GetResourceAllocator());
  FML_CHECK(success);

  fml::AutoResetWaitableEvent latch;
  success =
      surface_context->GetCommandQueue()
          ->Submit({command_buffer},
                   [&latch](CommandBuffer::Status status) {
                     FML_CHECK(status == CommandBuffer::Status::kCompleted);
                     latch.Signal();
                   })
          .ok();
  FML_CHECK(success);
  latch.Wait();
  device_buffer->Invalidate();

  FML_DCHECK(texture->GetTextureDescriptor().format ==
             PixelFormat::kB8G8R8A8UNormInt);

  return std::make_unique<LibPNGScreenshot>(device_buffer->OnGetContents(),
                                            texture->GetSize().width,
                                            texture->GetSize().height);
}
}  // namespace

VulkanScreenshotter::VulkanScreenshotter(
    const std::unique_ptr<PlaygroundImpl>& playground)
    : playground_(playground) {
  FML_CHECK(playground_);
}

std::unique_ptr<Screenshot> VulkanScreenshotter::MakeScreenshot(
    AiksContext& aiks_context,
    const std::shared_ptr<Texture> texture) {
  return ReadTexture(aiks_context.GetContext(), texture);
}

}  // namespace testing
}  // namespace impeller

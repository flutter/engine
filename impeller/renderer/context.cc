// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/context.h"

#include "impeller/core/capture.h"
#include "impeller/renderer/command_buffer.h"

namespace impeller {

Context::~Context() = default;

Context::Context() : capture(CaptureContext::MakeInactive()) {}

bool Context::UpdateOffscreenLayerPixelFormat(PixelFormat format) {
  return false;
}

void Context::GetTextureContents(const std::shared_ptr<Texture>& texture,
                                 const ReadbackCallback& callback) const {
  auto cmd_buffer = CreateCommandBuffer();
  auto blit_pass = cmd_buffer->CreateBlitPass();
  auto buffer_size =
      texture->GetTextureDescriptor().GetByteSizeOfBaseMipLevel();

  DeviceBufferDescriptor desc;
  desc.size = buffer_size;
  desc.storage_mode = StorageMode::kHostVisible;
  auto device_buffer = GetResourceAllocator()->CreateBuffer(desc);
  if (!device_buffer) {
    callback(nullptr);
    return;
  }

  if (!blit_pass->AddCopy(texture, device_buffer)) {
    callback(nullptr);
    return;
  }
  if (!blit_pass->EncodeCommands(GetResourceAllocator()) ||
      !cmd_buffer->SubmitCommands(
          [callback, device_buffer](CommandBuffer::Status status) {
            if (status == CommandBuffer::Status::kCompleted) {
              callback(device_buffer->CopyToUnqiueBuffer());
            } else {
              callback(nullptr);
            }
          })) {
    callback(nullptr);
  }
}

}  // namespace impeller

// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/context.h"

#include <utility>

#include "impeller/base/validation.h"

namespace impeller {

Context::~Context() = default;

Context::Context() = default;

bool Context::UpdateOffscreenLayerPixelFormat(PixelFormat format) {
  return false;
}

void Context::EnqueueCommandBuffer(
    std::shared_ptr<CommandBuffer> command_buffer) {
  bool result = GetCommandQueue()->Submit({std::move(command_buffer)}).ok();
  if (!result) {
    VALIDATION_LOG << "Failed to submit command buffer";
  }
}

bool Context::FlushCommandBuffers() {
  return true;
}

}  // namespace impeller

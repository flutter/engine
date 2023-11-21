// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/gpu/command_buffer.h"

#include "flutter/lib/ui/painting/image.h"
#include "fml/mapping.h"
#include "impeller/core/allocator.h"
#include "impeller/core/formats.h"
#include "impeller/display_list/dl_image_impeller.h"
#include "impeller/renderer/command_buffer.h"
#include "third_party/tonic/typed_data/dart_byte_data.h"

namespace flutter {
namespace gpu {

IMPLEMENT_WRAPPERTYPEINFO(flutter_gpu, CommandBuffer);

CommandBuffer::CommandBuffer(
    std::shared_ptr<impeller::CommandBuffer> command_buffer)
    : command_buffer_(std::move(command_buffer)) {}

std::shared_ptr<impeller::CommandBuffer> CommandBuffer::GetCommandBuffer() {
  return command_buffer_;
}

CommandBuffer::~CommandBuffer() = default;

}  // namespace gpu
}  // namespace flutter

//----------------------------------------------------------------------------
/// Exports
///

bool InternalFlutterGpu_CommandBuffer_Initialize(
    Dart_Handle wrapper,
    flutter::gpu::Context* contextWrapper) {
  auto res = fml::MakeRefCounted<flutter::gpu::CommandBuffer>(
      contextWrapper->GetContext()->CreateCommandBuffer());
  res->AssociateWithDartWrapper(wrapper);

  return true;
}

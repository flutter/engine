// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "flutter/lib/gpu/context.h"
#include "flutter/lib/gpu/export.h"
#include "flutter/lib/ui/dart_wrapper.h"
#include "impeller/core/formats.h"
#include "impeller/renderer/command_buffer.h"
#include "third_party/tonic/typed_data/dart_byte_data.h"

namespace flutter {
namespace gpu {

class CommandBuffer : public RefCountedDartWrappable<CommandBuffer> {
  DEFINE_WRAPPERTYPEINFO();
  FML_FRIEND_MAKE_REF_COUNTED(CommandBuffer);

 public:
  explicit CommandBuffer(
      std::shared_ptr<impeller::CommandBuffer> command_buffer);

  std::shared_ptr<impeller::CommandBuffer> GetCommandBuffer();

  ~CommandBuffer() override;

 private:
  std::shared_ptr<impeller::CommandBuffer> command_buffer_;

  FML_DISALLOW_COPY_AND_ASSIGN(CommandBuffer);
};

}  // namespace gpu
}  // namespace flutter

//----------------------------------------------------------------------------
/// Exports
///

extern "C" {

FLUTTER_GPU_EXPORT
extern bool InternalFlutterGpu_CommandBuffer_Initialize(
    Dart_Handle wrapper,
    flutter::gpu::Context* contextWrapper);

}  // extern "C"

// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <memory>
#include "flutter/lib/gpu/command_buffer.h"
#include "flutter/lib/gpu/export.h"
#include "flutter/lib/ui/dart_wrapper.h"
#include "impeller/renderer/command.h"
#include "impeller/renderer/render_pass.h"
#include "impeller/renderer/render_target.h"
#include "lib/gpu/texture.h"

namespace flutter {
namespace gpu {

class RenderPass : public RefCountedDartWrappable<RenderPass> {
  DEFINE_WRAPPERTYPEINFO();
  FML_FRIEND_MAKE_REF_COUNTED(RenderPass);

 public:
  RenderPass();

  const std::weak_ptr<const impeller::Context>& GetContext() const;

  impeller::Command& GetCommand();

  impeller::RenderTarget& GetRenderTarget();

  bool Begin(flutter::gpu::CommandBuffer& command_buffer);

  ~RenderPass() override;

 private:
  impeller::RenderTarget render_target_;
  impeller::Command command_;
  std::shared_ptr<impeller::RenderPass> render_pass_;

  FML_DISALLOW_COPY_AND_ASSIGN(RenderPass);
};

}  // namespace gpu
}  // namespace flutter

//----------------------------------------------------------------------------
/// Exports
///

extern "C" {

FLUTTER_GPU_EXPORT
extern void InternalFlutterGpu_RenderPass_Initialize(Dart_Handle wrapper);

FLUTTER_GPU_EXPORT
extern Dart_Handle InternalFlutterGpu_RenderPass_SetColorAttachment(
    flutter::gpu::RenderPass* wrapper,
    int load_action,
    int store_action,
    int clear_color,
    flutter::gpu::Texture* texture,
    Dart_Handle resolve_texture_wrapper);

FLUTTER_GPU_EXPORT
extern Dart_Handle InternalFlutterGpu_RenderPass_SetStencilAttachment(
    flutter::gpu::RenderPass* wrapper,
    int load_action,
    int store_action,
    int clear_stencil,
    flutter::gpu::Texture* texture);

FLUTTER_GPU_EXPORT
extern Dart_Handle InternalFlutterGpu_RenderPass_Begin(
    flutter::gpu::RenderPass* wrapper,
    flutter::gpu::CommandBuffer* command_buffer);

}  // extern "C"

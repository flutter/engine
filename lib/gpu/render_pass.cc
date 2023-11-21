// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/gpu/render_pass.h"

#include "dart_api.h"
#include "impeller/core/formats.h"
#include "impeller/geometry/color.h"
#include "tonic/converter/dart_converter.h"

namespace flutter {
namespace gpu {

IMPLEMENT_WRAPPERTYPEINFO(flutter_gpu, RenderPass);

RenderPass::RenderPass() = default;

impeller::Command& RenderPass::GetCommand() {
  return command_;
}

impeller::RenderTarget& RenderPass::GetRenderTarget() {
  return render_target_;
}

bool RenderPass::Begin(flutter::gpu::CommandBuffer& command_buffer) {
  render_pass_ =
      command_buffer.GetCommandBuffer()->CreateRenderPass(render_target_);
  if (!render_pass_) {
    return false;
  }
  command_buffer.AddRenderPass(render_pass_);
  return true;
}

RenderPass::~RenderPass() = default;

}  // namespace gpu
}  // namespace flutter

static impeller::Color ToImpellerColor(uint32_t argb) {
  return impeller::Color::MakeRGBA8((argb >> 16) & 0xFF,  // R
                                    (argb >> 8) & 0xFF,   // G
                                    argb & 0xFF,          // B
                                    argb >> 24);          // A
}

//----------------------------------------------------------------------------
/// Exports
///

void InternalFlutterGpu_RenderPass_Initialize(Dart_Handle wrapper) {
  auto res = fml::MakeRefCounted<flutter::gpu::RenderPass>();
  res->AssociateWithDartWrapper(wrapper);
}

Dart_Handle InternalFlutterGpu_RenderPass_SetColorAttachment(
    flutter::gpu::RenderPass* wrapper,
    int load_action,
    int store_action,
    int clear_color,
    flutter::gpu::Texture* texture,
    Dart_Handle resolve_texture_wrapper) {
  impeller::ColorAttachment desc;
  desc.load_action = static_cast<impeller::LoadAction>(load_action);
  desc.store_action = static_cast<impeller::StoreAction>(store_action);
  desc.clear_color = ToImpellerColor(static_cast<uint32_t>(clear_color));
  desc.texture = texture->GetTexture();
  if (!Dart_IsNull(resolve_texture_wrapper)) {
    flutter::gpu::Texture* resolve_texture =
        tonic::DartConverter<flutter::gpu::Texture*>::FromDart(
            resolve_texture_wrapper);
    desc.resolve_texture = resolve_texture->GetTexture();
  }
  wrapper->GetRenderTarget().SetColorAttachment(desc, 0);
  return Dart_Null();
}

Dart_Handle InternalFlutterGpu_RenderPass_SetStencilAttachment(
    flutter::gpu::RenderPass* wrapper,
    int load_action,
    int store_action,
    int clear_stencil,
    flutter::gpu::Texture* texture) {
  impeller::StencilAttachment desc;
  desc.load_action = static_cast<impeller::LoadAction>(load_action);
  desc.store_action = static_cast<impeller::StoreAction>(store_action);
  desc.clear_stencil = clear_stencil;
  desc.texture = texture->GetTexture();
  wrapper->GetRenderTarget().SetStencilAttachment(desc);
  return Dart_Null();
}

Dart_Handle InternalFlutterGpu_RenderPass_Begin(
    flutter::gpu::RenderPass* wrapper,
    flutter::gpu::CommandBuffer* command_buffer) {
  if (!wrapper->Begin(*command_buffer)) {
    return tonic::ToDart("Failed to begin RenderPass");
  }
  return Dart_Null();
}

// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/metal/surface_mtl.h"

#include "flutter/fml/trace_event.h"
#include "flutter/impeller/renderer/command_buffer.h"
#include "impeller/base/validation.h"
#include "impeller/renderer/backend/metal/formats_mtl.h"
#include "impeller/renderer/backend/metal/texture_mtl.h"
#include "impeller/renderer/render_target.h"

namespace impeller {

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunguarded-availability-new"

std::unique_ptr<SurfaceMTL> SurfaceMTL::WrapCurrentMetalLayerDrawable(
    std::shared_ptr<Context> context,
    CAMetalLayer* layer,
    ISize texture_size) {
  TRACE_EVENT0("impeller", "SurfaceMTL::WrapCurrentMetalLayerDrawable");

  if (context == nullptr || !context->IsValid() || layer == nil) {
    return nullptr;
  }

  PixelFormat color_format = FromMTLPixelFormat(layer.pixelFormat);

  TextureDescriptor msaa_tex_desc;
  msaa_tex_desc.storage_mode = StorageMode::kDeviceTransient;
  msaa_tex_desc.type = TextureType::kTexture2DMultisample;
  msaa_tex_desc.sample_count = SampleCount::kCount4;
  msaa_tex_desc.format = color_format;
  msaa_tex_desc.size = texture_size;
  msaa_tex_desc.usage = static_cast<uint64_t>(TextureUsage::kRenderTarget);

  auto msaa_tex = context->GetResourceAllocator()->CreateTexture(msaa_tex_desc);
  if (!msaa_tex) {
    VALIDATION_LOG << "Could not allocate MSAA color texture.";
    return nullptr;
  }
  msaa_tex->SetLabel("ImpellerOnscreenColorMSAA");

  TextureDescriptor resolve_tex_desc;
  resolve_tex_desc.format = color_format;
  resolve_tex_desc.size = msaa_tex_desc.size;
  resolve_tex_desc.usage = static_cast<uint64_t>(TextureUsage::kRenderTarget);
  resolve_tex_desc.sample_count = SampleCount::kCount1;
  resolve_tex_desc.storage_mode = StorageMode::kDevicePrivate;

  // Create color resolve texture.
  std::shared_ptr<Texture> resolve_tex =
      context->GetResourceAllocator()->CreateTexture(resolve_tex_desc);

  if (!resolve_tex) {
    VALIDATION_LOG << "Could not wrap resolve texture.";
    return nullptr;
  }
  resolve_tex->SetLabel("ImpellerOnscreenResolve");

  ColorAttachment color0;
  color0.texture = msaa_tex;
  color0.clear_color = Color::DarkSlateGray();
  color0.load_action = LoadAction::kClear;
  color0.store_action = StoreAction::kMultisampleResolve;
  color0.resolve_texture = resolve_tex;

  TextureDescriptor stencil_tex_desc;
  stencil_tex_desc.storage_mode = StorageMode::kDeviceTransient;
  stencil_tex_desc.type = TextureType::kTexture2DMultisample;
  stencil_tex_desc.sample_count = SampleCount::kCount4;
  stencil_tex_desc.format =
      context->GetCapabilities()->GetDefaultStencilFormat();
  stencil_tex_desc.size = msaa_tex_desc.size;
  stencil_tex_desc.usage =
      static_cast<TextureUsageMask>(TextureUsage::kRenderTarget);
  auto stencil_tex =
      context->GetResourceAllocator()->CreateTexture(stencil_tex_desc);

  if (!stencil_tex) {
    VALIDATION_LOG << "Could not create stencil texture.";
    return nullptr;
  }
  stencil_tex->SetLabel("ImpellerOnscreenStencil");

  StencilAttachment stencil0;
  stencil0.texture = stencil_tex;
  stencil0.clear_stencil = 0;
  stencil0.load_action = LoadAction::kClear;
  stencil0.store_action = StoreAction::kDontCare;

  RenderTarget render_target_desc;
  render_target_desc.SetColorAttachment(color0, 0u);
  render_target_desc.SetStencilAttachment(stencil0);

  // The constructor is private. So make_unique may not be used.
  return std::unique_ptr<SurfaceMTL>(
      new SurfaceMTL(context, render_target_desc, resolve_tex, layer));
}

SurfaceMTL::SurfaceMTL(std::shared_ptr<Context> context,
                       const RenderTarget& target,
                       std::shared_ptr<Texture> root_resolve_texture,
                       CAMetalLayer* layer)
    : Surface(target),
      context_(context),
      root_resolve_texture_(root_resolve_texture),
      layer_(layer) {}

// |Surface|
SurfaceMTL::~SurfaceMTL() = default;

// |Surface|
bool SurfaceMTL::Present() const {
  if (!layer_) {
    return false;
  }

  TRACE_EVENT0("impeller", "WaitForNextDrawable");
  id<CAMetalDrawable> current_drawable = [layer_ nextDrawable];
  auto command_buffer = context_->CreateCommandBuffer();
  if (!command_buffer) {
    return false;
  }

  auto blit_pass = command_buffer->CreateBlitPass();
  auto current = TextureMTL::Wrapper({}, current_drawable.texture);
  blit_pass->AddCopy(root_resolve_texture_, current);
  blit_pass->EncodeCommands(context_->GetResourceAllocator());
  if (!command_buffer->SubmitCommands()) {
    return false;
  }

  if (current_drawable == nil) {
    return false;
  }

  [current_drawable present];
  return true;
}
#pragma GCC diagnostic pop

}  // namespace impeller

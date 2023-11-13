// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/metal/surface_mtl.h"

#include "flutter/fml/trace_event.h"
#include "flutter/impeller/renderer/command_buffer.h"
#include "impeller/base/validation.h"
#include "impeller/core/texture_descriptor.h"
#include "impeller/renderer/backend/metal/context_mtl.h"
#include "impeller/renderer/backend/metal/formats_mtl.h"
#include "impeller/renderer/backend/metal/texture_mtl.h"
#include "impeller/renderer/render_target.h"

namespace impeller {

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunguarded-availability-new"

static std::optional<RenderTarget> WrapTextureWithRenderTarget(
    Allocator& allocator,
    CAMetalLayer* layer) {
  // compositor_context.cc will offset the rendering by the clip origin. Here we
  // shrink to the size of the clip. This has the same effect as clipping the
  // rendering but also creates smaller intermediate passes.
  ISize root_size = {static_cast<ISize::Type>(layer.drawableSize.width),
                     static_cast<ISize::Type>(layer.drawableSize.height)};

  TextureDescriptor resolve_tex_desc;
  resolve_tex_desc.format = FromMTLPixelFormat(layer.pixelFormat);
  resolve_tex_desc.size = root_size;
  resolve_tex_desc.usage = static_cast<uint64_t>(TextureUsage::kRenderTarget) |
                           static_cast<uint64_t>(TextureUsage::kShaderRead);
  resolve_tex_desc.sample_count = SampleCount::kCount1;
  resolve_tex_desc.storage_mode = StorageMode::kDevicePrivate;

  if (resolve_tex_desc.format == PixelFormat::kUnknown) {
    VALIDATION_LOG << "Unknown drawable color format.";
    return std::nullopt;
  }

  // Create color resolve texture.
  std::shared_ptr<Texture> resolve_tex =
      TextureMTL::Wrapper(resolve_tex_desc, layer);

  if (!resolve_tex) {
    VALIDATION_LOG << "Could not wrap resolve texture.";
    return std::nullopt;
  }
  resolve_tex->SetLabel("ImpellerOnscreenResolve");

  TextureDescriptor msaa_tex_desc;
  msaa_tex_desc.storage_mode = StorageMode::kDeviceTransient;
  msaa_tex_desc.type = TextureType::kTexture2DMultisample;
  msaa_tex_desc.sample_count = SampleCount::kCount4;
  msaa_tex_desc.format = resolve_tex->GetTextureDescriptor().format;
  msaa_tex_desc.size = resolve_tex->GetSize();
  msaa_tex_desc.usage = static_cast<uint64_t>(TextureUsage::kRenderTarget);

  auto msaa_tex = allocator.CreateTexture(msaa_tex_desc);
  if (!msaa_tex) {
    VALIDATION_LOG << "Could not allocate MSAA color texture.";
    return std::nullopt;
  }
  msaa_tex->SetLabel("ImpellerOnscreenColorMSAA");

  ColorAttachment color0;
  color0.texture = msaa_tex;
  color0.clear_color = Color::DarkSlateGray();
  color0.load_action = LoadAction::kClear;
  color0.store_action = StoreAction::kMultisampleResolve;
  color0.resolve_texture = std::move(resolve_tex);

  auto render_target_desc = std::make_optional<RenderTarget>();
  render_target_desc->SetColorAttachment(color0, 0u);

  return render_target_desc;
}

std::unique_ptr<SurfaceMTL> SurfaceMTL::MakeFromMetalLayer(
    const std::shared_ptr<Context>& context,
    CAMetalLayer* layer) {
  // The returned render target is the texture that Impeller will render the
  // root pass to. If partial repaint is in use, this may be a new texture which
  // is smaller than the given MTLTexture.
  auto render_target =
      WrapTextureWithRenderTarget(*context->GetResourceAllocator(), layer);
  if (!render_target) {
    return nullptr;
  }

  return std::unique_ptr<SurfaceMTL>(new SurfaceMTL(
      context,                                 // context
      *render_target,                          // target
      render_target->GetRenderTargetTexture()  // resolve_texture
      ));
}

SurfaceMTL::SurfaceMTL(const std::weak_ptr<Context>& context,
                       const RenderTarget& target,
                       std::shared_ptr<Texture> resolve_texture)
    : Surface(target),
      context_(context),
      resolve_texture_(std::move(resolve_texture)) {}

// |Surface|
SurfaceMTL::~SurfaceMTL() = default;

// |Surface|
IRect SurfaceMTL::coverage() const {
  return IRect::MakeSize(resolve_texture_->GetSize());
}

// |Surface|
bool SurfaceMTL::Present() const {
  auto context = context_.lock();
  if (!context) {
    return false;
  }

#ifdef IMPELLER_DEBUG
  ContextMTL::Cast(context.get())->GetGPUTracer()->MarkFrameEnd();
#endif  // IMPELL

  id<MTLCommandBuffer> command_buffer =
      ContextMTL::Cast(context.get())
          ->CreateMTLCommandBuffer("Present Waiter Command Buffer");
  // If the threads have been merged, or there is a pending frame capture,
  // then block on cmd buffer scheduling to ensure that the
  // transaction/capture work correctly.
  if ([[NSThread currentThread] isMainThread] ||
      [[MTLCaptureManager sharedCaptureManager] isCapturing]) {
    TRACE_EVENT0("flutter", "waitUntilScheduled");
    [command_buffer commit];
    [command_buffer waitUntilScheduled];
    TextureMTL::Cast(resolve_texture_.get())->WaitForNextDrawable();
    auto drawable = TextureMTL::Cast(resolve_texture_.get())->GetDrawable();
    [drawable present];
  } else {
    auto resolve_texture = resolve_texture_;
    [command_buffer addScheduledHandler:^(id<MTLCommandBuffer> _Nonnull) {
      auto mtl_texture = TextureMTL::Cast(resolve_texture.get());
      mtl_texture->WaitForNextDrawable();
      auto drawable = mtl_texture->GetDrawable();
      if (drawable) {
        [drawable present];
      }
    }];
    [command_buffer commit];
  }

  return true;
}
#pragma GCC diagnostic pop

}  // namespace impeller

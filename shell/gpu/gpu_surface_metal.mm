// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// FLUTTER_NOLINT

#include "flutter/shell/gpu/gpu_surface_metal.h"

#include <QuartzCore/CAMetalLayer.h>

#include "flutter/fml/trace_event.h"
#include "third_party/skia/include/core/SkSurface.h"
#include "third_party/skia/include/gpu/GrBackendSurface.h"
#include "third_party/skia/include/gpu/mtl/GrMtlTypes.h"
#include "third_party/skia/include/ports/SkCFObject.h"

static_assert(!__has_feature(objc_arc), "ARC must be disabled.");

namespace flutter {

namespace {
sk_sp<SkSurface> CreateSurfaceFromMetalTexture(GrContext* context,
                                               id<MTLTexture> texture,
                                               GrSurfaceOrigin origin,
                                               int sample_cnt,
                                               SkColorType color_type,
                                               sk_sp<SkColorSpace> color_space,
                                               const SkSurfaceProps* props) {
  GrMtlTextureInfo info;
  info.fTexture.reset([texture retain]);
  GrBackendTexture backend_texture(texture.width, texture.height, GrMipmapped::kNo, info);
  return SkSurface::MakeFromBackendTexture(context, backend_texture, origin, sample_cnt, color_type,
                                           color_space, props);
}
}  // namespace

GPUSurfaceMetal::GPUSurfaceMetal(GPUSurfaceDelegate* delegate,
                                 fml::scoped_nsobject<CAMetalLayer> layer,
                                 sk_sp<GrDirectContext> context,
                                 fml::scoped_nsprotocol<id<MTLCommandQueue>> command_queue)
    : delegate_(delegate),
      layer_(std::move(layer)),
      context_(std::move(context)),
      command_queue_(std::move(command_queue)) {
  layer_.get().pixelFormat = MTLPixelFormatBGRA8Unorm;
  // Flutter needs to read from the color attachment in cases where there are effects such as
  // backdrop filters.
  layer_.get().framebufferOnly = NO;
}

GPUSurfaceMetal::~GPUSurfaceMetal() {}

// |Surface|
bool GPUSurfaceMetal::IsValid() {
  return layer_ && context_ && command_queue_;
}

// |Surface|
std::unique_ptr<SurfaceFrame> GPUSurfaceMetal::AcquireFrame(const SkISize& frame_size) {
  if (!IsValid()) {
    FML_LOG(ERROR) << "Metal surface was invalid.";
    return nullptr;
  }

  if (frame_size.isEmpty()) {
    FML_LOG(ERROR) << "Metal surface was asked for an empty frame.";
    return nullptr;
  }

  const auto drawable_size = CGSizeMake(frame_size.width(), frame_size.height());

  if (!CGSizeEqualToSize(drawable_size, layer_.get().drawableSize)) {
    layer_.get().drawableSize = drawable_size;
  }

  // When there are platform views in the scene, the drawable needs to be presented in the same
  // transaction as the one created for platform views. When the drawable are being presented from
  // the raster thread, there is no such transaction.
  layer_.get().presentsWithTransaction = [[NSThread currentThread] isMainThread];

  // Get the drawable eagerly, we will need texture object to identify target framebuffer
  fml::scoped_nsprotocol<id<CAMetalDrawable>> drawable(
      reinterpret_cast<id<CAMetalDrawable>>([[layer_.get() nextDrawable] retain]));

  if (!drawable.get()) {
    FML_LOG(ERROR) << "Could not obtain drawable from the metal layer.";
    return nullptr;
  }

  auto surface = CreateSurfaceFromMetalTexture(context_.get(), drawable.get().texture,
                                               kTopLeft_GrSurfaceOrigin,  // origin
                                               1,                         // sample count
                                               kBGRA_8888_SkColorType,    // color type
                                               nullptr,                   // colorspace
                                               nullptr                    // surface properties
  );

  if (!surface) {
    FML_LOG(ERROR) << "Could not create the SkSurface from the metal texture.";
    return nullptr;
  }

  auto submit_callback = [this, drawable](const SurfaceFrame& surface_frame,
                                          SkCanvas* canvas) -> bool {
    TRACE_EVENT0("flutter", "GPUSurfaceMetal::Submit");
    if (canvas == nullptr) {
      FML_DLOG(ERROR) << "Canvas not available.";
      return false;
    }

    canvas->flush();

    auto command_buffer =
        fml::scoped_nsprotocol<id<MTLCommandBuffer>>([[command_queue_.get() commandBuffer] retain]);

    uintptr_t texture = reinterpret_cast<uintptr_t>(drawable.get().texture);
    for (auto& entry : damage_) {
      if (entry.first != texture) {
        // Accumulate damage for other framebuffers
        for (const auto& rect : surface_frame.submit_info().surface_damage) {
          entry.second.join(rect);
        }
      }
    }
    // Reset accumulated damage for current framebuffer
    damage_[texture] = SkIRect::MakeEmpty();

    [command_buffer.get() commit];
    [command_buffer.get() waitUntilScheduled];
    [drawable.get() present];

    return true;
  };

  SurfaceFrame::FramebufferInfo framebuffer_info;
  framebuffer_info.supports_readback = true;

  // Provide accumulated damage to rasterizer (area in current framebuffer that lags behind
  // front buffer)
  uintptr_t texture = reinterpret_cast<uintptr_t>(drawable.get().texture);
  auto i = damage_.find(texture);
  if (i != damage_.end()) {
    framebuffer_info.existing_damage.push_back(i->second);
  }

  return std::make_unique<SurfaceFrame>(std::move(surface), std::move(framebuffer_info),
                                        submit_callback);
}

// |Surface|
SkMatrix GPUSurfaceMetal::GetRootTransformation() const {
  // This backend does not currently support root surface transformations. Just
  // return identity.
  return {};
}

// |Surface|
GrDirectContext* GPUSurfaceMetal::GetContext() {
  return context_.get();
}

// |Surface|
flutter::ExternalViewEmbedder* GPUSurfaceMetal::GetExternalViewEmbedder() {
  return delegate_->GetExternalViewEmbedder();
}

// |Surface|
std::unique_ptr<GLContextResult> GPUSurfaceMetal::MakeRenderContextCurrent() {
  // This backend has no such concept.
  return std::make_unique<GLContextDefaultResult>(true);
}

}  // namespace flutter

// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/gpu/gpu_surface_metal.h"

#import <Metal/Metal.h>

#include "flutter/fml/make_copyable.h"
#include "flutter/fml/trace_event.h"
#include "flutter/shell/gpu/gpu_surface_metal_delegate.h"
#include "third_party/skia/include/core/SkSurface.h"
#include "third_party/skia/include/gpu/GrBackendSurface.h"
#include "third_party/skia/include/ports/SkCFObject.h"

static_assert(!__has_feature(objc_arc), "ARC must be disabled.");

namespace flutter {

namespace {
struct MTLDrawableDeleter {
  void operator()(GrMTLHandle* drawable) {
    if (drawable && *drawable) {
      CFRelease(*drawable);
    }
  }
};
}

GPUSurfaceMetal::GPUSurfaceMetal(GPUSurfaceMetalDelegate* delegate, sk_sp<GrDirectContext> context)
    : delegate_(delegate),
      render_target_type_(delegate->GetRenderTargetType()),
      context_(std::move(context)) {}

GPUSurfaceMetal::~GPUSurfaceMetal() = default;

// |Surface|
bool GPUSurfaceMetal::IsValid() {
  return context_ != nullptr;
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

  MTLFrameInfo frame_info;
  frame_info.height = frame_size.height();
  frame_info.width = frame_size.width();

  switch (render_target_type_) {
    case MTLRenderTargetType::kCAMetalLayer:
      return AcquireFrameFromCAMetalLayer(frame_info);
    case MTLRenderTargetType::kMTLTexture:
      return AcquireFrameFromMTLTexture(frame_info);
    default:
      FML_CHECK(false) << "Unknown MTLRenderTargetType type.";
  }

  return nullptr;
}

std::unique_ptr<SurfaceFrame> GPUSurfaceMetal::AcquireFrameFromCAMetalLayer(
    const MTLFrameInfo& frame_info) {
  auto layer = delegate_->GetCAMetalLayer(frame_info);
  if (!layer) {
    FML_LOG(ERROR) << "Invalid CAMetalLayer given by the embedder.";
    return nullptr;
  }

  std::unique_ptr<GrMTLHandle, MTLDrawableDeleter> drawable;
  sk_sp<SkSurface> surface =
      SkSurface::MakeFromCAMetalLayer(context_.get(),            // context
                                      layer,                     // layer
                                      kTopLeft_GrSurfaceOrigin,  // origin
                                      1,                         // sample count
                                      kBGRA_8888_SkColorType,    // color type
                                      nullptr,                   // colorspace
                                      nullptr,                   // surface properties
                                      drawable.get()             // drawable (transfer out)
      );

  if (!surface) {
    FML_LOG(ERROR) << "Could not create the SkSurface from the CAMetalLayer.";
    return nullptr;
  }

  auto submit_callback =
      fml::MakeCopyable([drawable = std::move(drawable), delegate = delegate_](
                            const SurfaceFrame& surface_frame, SkCanvas* canvas) -> bool {
        TRACE_EVENT0("flutter", "GPUSurfaceMetal::Submit");
        if (canvas == nullptr) {
          FML_DLOG(ERROR) << "Canvas not available.";
          return false;
        }

        canvas->flush();

        return delegate->PresentDrawable(drawable.get());
      });

  return std::make_unique<SurfaceFrame>(std::move(surface), true, submit_callback);
}

std::unique_ptr<SurfaceFrame> GPUSurfaceMetal::AcquireFrameFromMTLTexture(
    const MTLFrameInfo& frame_info) {
  GPUMTLTextureInfo texture = delegate_->GetMTLTexture(frame_info);
  id<MTLTexture> mtl_texture = (id<MTLTexture>)(texture.texture);

  if (!mtl_texture) {
    FML_LOG(ERROR) << "Invalid MTLTexture given by the embedder.";
    return nullptr;
  }

  GrMtlTextureInfo info;
  info.fTexture.reset([mtl_texture retain]);
  GrBackendTexture backend_texture(frame_info.width, frame_info.height, GrMipmapped::kNo, info);

  sk_sp<SkSurface> surface =
      SkSurface::MakeFromBackendTexture(context_.get(), backend_texture, kTopLeft_GrSurfaceOrigin,
                                        1, kBGRA_8888_SkColorType, nullptr, nullptr);

  if (!surface) {
    FML_LOG(ERROR) << "Could not create the SkSurface from the metal texture.";
    return nullptr;
  }

  auto submit_callback = [texture_id = texture.texture_id, delegate = delegate_](
                             const SurfaceFrame& surface_frame, SkCanvas* canvas) -> bool {
    TRACE_EVENT0("flutter", "GPUSurfaceMetal::PresentTexture");
    if (canvas == nullptr) {
      FML_DLOG(ERROR) << "Canvas not available.";
      return false;
    }

    canvas->flush();

    return delegate->PresentTexture(texture_id);
  };

  return std::make_unique<SurfaceFrame>(std::move(surface), true, submit_callback);
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
std::unique_ptr<GLContextResult> GPUSurfaceMetal::MakeRenderContextCurrent() {
  // This backend has no such concept.
  return std::make_unique<GLContextDefaultResult>(true);
}

}  // namespace flutter

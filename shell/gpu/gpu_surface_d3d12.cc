// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/gpu/gpu_surface_d3d12.h"

#include "flutter/fml/logging.h"
#include "fml/trace_event.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkSize.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/d3d/GrD3DBackendContext.h"

namespace flutter {

GPUSurfaceD3D12::GPUSurfaceD3D12(GPUSurfaceD3D12Delegate* delegate,
                                   const sk_sp<GrDirectContext>& skia_context)
    : delegate_(delegate),
      skia_context_(skia_context),
      weak_factory_(this) {}

GPUSurfaceD3D12::~GPUSurfaceD3D12() = default;

bool GPUSurfaceD3D12::IsValid() {
  return skia_context_ != nullptr;
}

std::unique_ptr<SurfaceFrame> GPUSurfaceD3D12::AcquireFrame(
    const SkISize& frame_size) {
  if (!IsValid()) {
    FML_LOG(ERROR) << "D3D12 surface was invalid.";
    return nullptr;
  }

  if (frame_size.isEmpty()) {
    FML_LOG(ERROR) << "D3D12 surface was asked for an empty frame.";
    return nullptr;
  }

  //if (frame_size != last_frame_size_) {
    // Delegate may destroy the old buffer
    // Clean up any outstanding resources in command lists
    skia_context_->flush({});
    skia_context_->submit(true);
  //  last_frame_size_ = frame_size;
  //}

  ID3D12Resource* image = delegate_->AcquireBackBuffer(frame_size);
  if (!image) {
    FML_LOG(ERROR) << "Invalid ID3D12Resource given by the embedder.";
    return nullptr;
  }

  D3D12_RESOURCE_DESC image_desc = image->GetDesc();

  sk_sp<SkSurface> surface = CreateSurfaceFromResource(
      image,
      image_desc.Format, frame_size);
  if (!surface) {
    FML_LOG(ERROR) << "Could not create the SkSurface from the D3D12 resource.";
    return nullptr;
  }

  SurfaceFrame::SubmitCallback callback = [image = image, delegate = delegate_, skia_context = skia_context_](
                                              const SurfaceFrame& frame,
                                              SkCanvas* canvas) -> bool {
    TRACE_EVENT0("flutter", "GPUSurfaceD3D12::PresentBackBuffer");
    if (canvas == nullptr) {
      FML_DLOG(ERROR) << "Canvas not available.";
      return false;
    }

    auto surface = frame.SkiaSurface();

    GrFlushInfo info;
    surface->flush(SkSurface::BackendSurfaceAccess::kPresent, info);
    // surface->flushAndSubmit();
    // skia_context->submit();

    return delegate->PresentBackBuffer();
  };

  SurfaceFrame::FramebufferInfo framebuffer_info{.supports_readback = !(image_desc.Flags & D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE)};

  return std::make_unique<SurfaceFrame>(
      std::move(surface), std::move(framebuffer_info), std::move(callback));
}

SkMatrix GPUSurfaceD3D12::GetRootTransformation() const {
  // This backend does not support delegating to the underlying platform to
  // query for root surface transformations. Just return identity.
  SkMatrix matrix;
  matrix.reset();
  return matrix;
}

GrDirectContext* GPUSurfaceD3D12::GetContext() {
  return skia_context_.get();
}

sk_sp<SkSurface> GPUSurfaceD3D12::CreateSurfaceFromResource(
    ID3D12Resource* image,
    const DXGI_FORMAT format,
    const SkISize& size) {

  GrD3DTextureResourceInfo image_info(
    nullptr,
    nullptr,
    D3D12_RESOURCE_STATE_PRESENT,
    format,
    1,
    1,
    0
  );
  image_info.fResource.reset(image);

  // TODO: For MSAA, use GrBackendTexture instead.
  // See https://github.com/google/skia/blob/1f193df9b393d50da39570dab77a0bb5d28ec8ef/tools/sk_app/win/D3D12WindowContext_win.cpp#L154

  GrBackendRenderTarget backend_rt(size.width(),   //
                                   size.height(),  //
                                   image_info      //
  );

  SkSurfaceProps surface_properties(0, kUnknown_SkPixelGeometry);

  return SkSurface::MakeFromBackendRenderTarget(
      skia_context_.get(),          // context
      backend_rt,              // back-end texture
      kTopLeft_GrSurfaceOrigin,     // surface origin
      ColorTypeFromFormat(format),  // color type
      SkColorSpace::MakeSRGB(),     // color space
      &surface_properties           // surface properties
  );
}

SkColorType GPUSurfaceD3D12::ColorTypeFromFormat(const DXGI_FORMAT format) {
  switch (format) {
    case DXGI_FORMAT_R8G8B8A8_UNORM:
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
      return SkColorType::kRGBA_8888_SkColorType;
    case DXGI_FORMAT_B8G8R8A8_UNORM:
    case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
      return SkColorType::kBGRA_8888_SkColorType;
    default:
      return SkColorType::kUnknown_SkColorType;
  }
}

}  // namespace flutter

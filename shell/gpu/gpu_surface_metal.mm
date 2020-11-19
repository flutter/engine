// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/gpu/gpu_surface_metal.h"

#include "flutter/fml/trace_event.h"
#include "flutter/shell/gpu/gpu_surface_metal_delegate.h"
#include "third_party/skia/include/core/SkSurface.h"
#include "third_party/skia/include/gpu/GrBackendSurface.h"
#include "third_party/skia/include/ports/SkCFObject.h"

static_assert(!__has_feature(objc_arc), "ARC must be disabled.");

namespace flutter {

GPUSurfaceMetal::GPUSurfaceMetal(GPUSurfaceMetalDelegate* delegate, sk_sp<GrDirectContext> context)
    : delegate_(delegate),
      render_target_type_(delegate->GetRenderTargetType()),
      context_(std::move(context)) {}

GPUSurfaceMetal::~GPUSurfaceMetal() {
  ReleaseUnusedDrawableIfNecessary();
}

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

  ReleaseUnusedDrawableIfNecessary();

  MTLFrameInfo frame_info;
  frame_info.height = frame_size.height();
  frame_info.width = frame_size.width();

  auto layer = delegate_->GetCAMetalLayer(frame_info);
  if (!layer) {
    FML_LOG(ERROR) << "Invalid CALayer given by the embedder.";
    return nullptr;
  }

  auto surface = SkSurface::MakeFromCAMetalLayer(context_.get(),            // context
                                                 layer,                     // layer
                                                 kTopLeft_GrSurfaceOrigin,  // origin
                                                 1,                         // sample count
                                                 kBGRA_8888_SkColorType,    // color type
                                                 nullptr,                   // colorspace
                                                 nullptr,                   // surface properties
                                                 &next_drawable_  // drawable (transfer out)
  );

  if (!surface) {
    FML_LOG(ERROR) << "Could not create the SkSurface from the metal texture.";
    return nullptr;
  }

  auto submit_callback = [this](const SurfaceFrame& surface_frame, SkCanvas* canvas) -> bool {
    TRACE_EVENT0("flutter", "GPUSurfaceMetal::Submit");
    if (canvas == nullptr) {
      FML_DLOG(ERROR) << "Canvas not available.";
      return false;
    }

    canvas->flush();

    bool present_result = delegate_->PresentDrawable(next_drawable_);
    next_drawable_ = nullptr;
    return present_result;
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

void GPUSurfaceMetal::ReleaseUnusedDrawableIfNecessary() {
  // If the previous surface frame was not submitted before  a new one is acquired, the old drawable
  // needs to be released. An RAII wrapper may not be used because this needs to interoperate with
  // Skia APIs.
  if (next_drawable_ == nullptr) {
    return;
  }

  CFRelease(next_drawable_);
  next_drawable_ = nullptr;
}

}  // namespace flutter

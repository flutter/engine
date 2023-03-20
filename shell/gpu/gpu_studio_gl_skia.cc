// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/gpu/gpu_studio_gl_skia.h"

#include "flutter/common/graphics/persistent_cache.h"
#include "flutter/fml/base32.h"
#include "flutter/fml/logging.h"
#include "flutter/fml/size.h"
#include "flutter/fml/trace_event.h"
#include "flutter/shell/common/context_options.h"
#include "flutter/shell/gpu/gpu_surface_gl_delegate.h"
#include "third_party/skia/include/core/SkAlphaType.h"
#include "third_party/skia/include/core/SkColorFilter.h"
#include "third_party/skia/include/core/SkColorSpace.h"
#include "third_party/skia/include/core/SkColorType.h"
#include "third_party/skia/include/gpu/GrContextOptions.h"

// These are common defines present on all OpenGL headers. However, we don't
// want to perform GL header reasolution on each platform we support. So just
// define these upfront. It is unlikely we will need more. But, if we do, we can
// add the same here.
#define GPU_GL_RGBA8 0x8058
#define GPU_GL_RGBA4 0x8056
#define GPU_GL_RGB565 0x8D62

namespace flutter {

GPUStudioGLSkia::GPUStudioGLSkia(const sk_sp<GrDirectContext>& gr_context,
                                   GPUSurfaceGLDelegate* delegate)
    : delegate_(delegate),
      context_(gr_context) {
  auto context_switch = delegate_->GLContextMakeCurrent();
  if (!context_switch->GetResult()) {
    FML_LOG(ERROR)
        << "Could not make the context current to set up the Gr context.";
    return;
  }

  delegate_->GLContextClearCurrent();

  valid_ = gr_context != nullptr;
}

GPUStudioGLSkia::~GPUStudioGLSkia() {
  if (!valid_) {
    return;
  }
  auto context_switch = delegate_->GLContextMakeCurrent();
  if (!context_switch->GetResult()) {
    FML_LOG(ERROR) << "Could not make the context current to destroy the "
                      "GrDirectContext resources.";
    return;
  }

  context_ = nullptr;

  delegate_->GLContextClearCurrent();
}

// |Studio|
bool GPUStudioGLSkia::IsValid() {
  return valid_;
}

// |Studio|
GrDirectContext* GPUStudioGLSkia::GetContext() {
  return context_.get();
}

// |Studio|
std::unique_ptr<GLContextResult> GPUStudioGLSkia::MakeRenderContextCurrent() {
  return delegate_->GLContextMakeCurrent();
}

// |Studio|
bool GPUStudioGLSkia::ClearRenderContext() {
  return delegate_->GLContextClearCurrent();
}

// |Studio|
bool GPUStudioGLSkia::AllowsDrawingWhenGpuDisabled() const {
  return delegate_->AllowsDrawingWhenGpuDisabled();
}

}  // namespace flutter

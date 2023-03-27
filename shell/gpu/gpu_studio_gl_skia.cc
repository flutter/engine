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

namespace flutter {

// Default maximum number of bytes of GPU memory of budgeted resources in the
// cache.
// The shell will dynamically increase or decrease this cache based on the
// viewport size, unless a user has specifically requested a size on the Skia
// system channel.
static const size_t kGrCacheMaxByteSize = 24 * (1 << 20);

sk_sp<GrDirectContext> GPUStudioGLSkia::MakeGLContext(
    GPUSurfaceGLDelegate* delegate) {
  auto context_switch = delegate->GLContextMakeCurrent();
  if (!context_switch->GetResult()) {
    FML_LOG(ERROR)
        << "Could not make the context current to set up the Gr context.";
    return nullptr;
  }

  const auto options =
      MakeDefaultContextOptions(ContextType::kRender, GrBackendApi::kOpenGL);

  auto context = GrDirectContext::MakeGL(delegate->GetGLInterface(), options);

  if (!context) {
    FML_LOG(ERROR) << "Failed to set up Skia Gr context.";
    return nullptr;
  }

  context->setResourceCacheLimit(kGrCacheMaxByteSize);

  PersistentCache::GetCacheForProcess()->PrecompileKnownSkSLs(context.get());

  return context;
}

GPUStudioGLSkia::GPUStudioGLSkia(const sk_sp<GrDirectContext>& gr_context,
                                 GPUSurfaceGLDelegate* delegate)
    : delegate_(delegate), context_(gr_context) {
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

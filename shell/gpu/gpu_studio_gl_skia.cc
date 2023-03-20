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
#include "third_party/skia/include/core/SkStudio.h"
#include "third_party/skia/include/gpu/GrBackendStudio.h"
#include "third_party/skia/include/gpu/GrContextOptions.h"

// These are common defines present on all OpenGL headers. However, we don't
// want to perform GL header reasolution on each platform we support. So just
// define these upfront. It is unlikely we will need more. But, if we do, we can
// add the same here.
#define GPU_GL_RGBA8 0x8058
#define GPU_GL_RGBA4 0x8056
#define GPU_GL_RGB565 0x8D62

namespace flutter {

// Default maximum number of bytes of GPU memory of budgeted resources in the
// cache.
// The shell will dynamically increase or decrease this cache based on the
// viewport size, unless a user has specifically requested a size on the Skia
// system channel.
static const size_t kGrCacheMaxByteSize = 24 * (1 << 20);

sk_sp<GrDirectContext> GPUStudioGLSkia::MakeGLContext(
    GPUStudioGLDelegate* delegate) {
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
                                   GPUStudioGLDelegate* delegate,
                                   bool render_to_surface)
    : delegate_(delegate),
      context_(gr_context),

      render_to_surface_(render_to_surface),
      weak_factory_(this) {
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

  onscreen_surface_ = nullptr;
  fbo_id_ = 0;
  context_ = nullptr;

  delegate_->GLContextClearCurrent();
}

// |Studio|
bool GPUStudioGLSkia::IsValid() {
  return valid_;
}

static SkColorType FirstSupportedColorType(GrDirectContext* context,
                                           GrGLenum* format) {
#define RETURN_IF_RENDERABLE(x, y)                 \
  if (context->colorTypeSupportedAsStudio((x))) { \
    *format = (y);                                 \
    return (x);                                    \
  }
  RETURN_IF_RENDERABLE(kRGBA_8888_SkColorType, GPU_GL_RGBA8);
  RETURN_IF_RENDERABLE(kARGB_4444_SkColorType, GPU_GL_RGBA4);
  RETURN_IF_RENDERABLE(kRGB_565_SkColorType, GPU_GL_RGB565);
  return kUnknown_SkColorType;
}

static sk_sp<SkStudio> WrapOnscreenStudio(GrDirectContext* context,
                                            const SkISize& size,
                                            intptr_t fbo) {
  GrGLenum format = kUnknown_SkColorType;
  const SkColorType color_type = FirstSupportedColorType(context, &format);

  GrGLFramebufferInfo framebuffer_info = {};
  framebuffer_info.fFBOID = static_cast<GrGLuint>(fbo);
  framebuffer_info.fFormat = format;

  GrBackendRenderTarget render_target(size.width(),     // width
                                      size.height(),    // height
                                      0,                // sample count
                                      0,                // stencil bits
                                      framebuffer_info  // framebuffer info
  );

  sk_sp<SkColorSpace> colorspace = SkColorSpace::MakeSRGB();
  SkStudioProps surface_props(0, kUnknown_SkPixelGeometry);

  return SkStudio::MakeFromBackendRenderTarget(
      context,                                       // Gr context
      render_target,                                 // render target
      GrStudioOrigin::kBottomLeft_GrStudioOrigin,  // origin
      color_type,                                    // color type
      colorspace,                                    // colorspace
      &surface_props                                 // surface properties
  );
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

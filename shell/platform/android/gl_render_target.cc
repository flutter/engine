// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/android/gl_render_target.h"
#include "third_party/skia/include/core/SkSurface.h"
#include "third_party/skia/include/gpu/gl/GrGLTypes.h"

namespace shell {

static sk_sp<SkSurface> CreateSurface(GrContext* context, const SkISize& size) {
  SkImageInfo image_info =
      SkImageInfo::MakeS32(size.fWidth, size.fHeight, kOpaque_SkAlphaType);
  SkSurfaceProps surface_props(
      SkSurfaceProps::InitType::kLegacyFontHost_InitType);

  auto surface = SkSurface::MakeRenderTarget(
      context,                      // context
      SkBudgeted::kNo,              // budgeted
      image_info,                   // image info
      0,                            // sample count
      kBottomLeft_GrSurfaceOrigin,  // surface origin
      &surface_props                // surface props
      );

  return surface;
}

static sk_sp<SkSurface> WrapFBO0(GrContext* context, const SkISize& size) {
  GrBackendRenderTargetDesc desc;

  desc.fConfig = kRGBA_8888_GrPixelConfig;
  desc.fWidth = size.width();
  desc.fHeight = size.height();
  desc.fStencilBits = 8;
  desc.fOrigin = kBottomLeft_GrSurfaceOrigin;
  desc.fRenderTargetHandle = 0 /* FBO0 */;

  return SkSurface::MakeFromBackendRenderTarget(
      context,  // GR Context
      desc,     // backend render target description
      nullptr,  // colorspace
      nullptr   // surface props
      );
}

GLRenderTarget::GLRenderTarget(GrContext* context, const SkISize& size)
    : context_(context) {
  auto onscreen_buffer = WrapFBO0(context, size);
  if (!onscreen_buffer) {
    return;
  }
  onscreen_buffer_ = std::move(onscreen_buffer);
  Resize(size);
}

GLRenderTarget::~GLRenderTarget() = default;

bool GLRenderTarget::IsValid() const {
  return valid_;
}

bool GLRenderTarget::Resize(const SkISize& size) {
  valid_ = false;
  offscreen_buffer_ = nullptr;

  auto offscreen_buffer = CreateSurface(context_, size);

  if (offscreen_buffer == nullptr) {
    return false;
  }

  offscreen_buffer_ = std::move(offscreen_buffer);
  valid_ = true;
  return true;
}

intptr_t GLRenderTarget::GetFBO() const {
  if (offscreen_buffer_ == nullptr) {
    FTL_DLOG(ERROR) << "Offscreen buffer was not setup.";
    return -1;
  }

  GrBackendObject object;
  if (!offscreen_buffer_->getRenderTargetHandle(
          &object,
          SkSurface::BackendHandleAccess::kDiscardWrite_BackendHandleAccess)) {
    FTL_DLOG(ERROR) << "Could not access the FBO of the offscreen buffer.";
    return -1;
  }

  FTL_DLOG(INFO) << "Render target is: " << object;

  return object;
}

bool GLRenderTarget::RenderToWindowFBO() {
  if (!valid_) {
    return false;
  }

  auto canvas = onscreen_buffer_->getCanvas();

  canvas->drawImage(offscreen_buffer_->makeImageSnapshot(), 0, 0, nullptr);
  canvas->flush();

  return true;
}

}  // namespace shell

// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/android/android_surface_gl.h"

#include <utility>

#include "flutter/common/threads.h"
#include "lib/ftl/logging.h"
#include "lib/ftl/memory/ref_ptr.h"
#include "third_party/skia/include/gpu/GrContextOptions.h"
#include "third_party/skia/include/gpu/gl/GrGLInterface.h"

namespace shell {

static ftl::RefPtr<AndroidContextGL> GlobalResourceLoadingContext(
    PlatformView::SurfaceConfig offscreen_config) {
  // AndroidSurfaceGL instances are only ever created on the platform thread. So
  // there is no need to lock here.

  static ftl::RefPtr<AndroidContextGL> global_context;

  if (global_context) {
    return global_context;
  }

  auto environment = ftl::MakeRefCounted<AndroidEnvironmentGL>();

  if (!environment->IsValid()) {
    return nullptr;
  }

  // TODO(chinmaygarde): We should check that the configurations are stable
  // across multiple invocations.

  auto context = ftl::MakeRefCounted<AndroidContextGL>(
      environment,       // GL environment
      offscreen_config,  // platform config
      nullptr,           // native window
      nullptr            // sharegroup
      );

  if (!context->IsValid()) {
    return nullptr;
  }

  global_context = context;
  return global_context;
}

AndroidSurfaceGL::AndroidSurfaceGL(
    PlatformView::SurfaceConfig offscreen_config) {
  // Acquire the offscreen context.
  auto offscreen = GlobalResourceLoadingContext(offscreen_config);

  if (!offscreen || !offscreen->IsValid()) {
    return;
  }

  // Make the offscreen context current so we can setup the GrContext.
  if (!offscreen->MakeCurrent()) {
    FTL_LOG(ERROR) << "Could not make the offscreen context current to setup "
                      "the GL render target.";
    return;
  }

  // Create the native interface.
  auto backend_context =
      reinterpret_cast<GrBackendContext>(GrGLCreateNativeInterface());

  // Setup context options.
  GrContextOptions options;
  options.fRequireDecodeDisableForSRGB = false;

  auto context = sk_sp<GrContext>(
      GrContext::Create(kOpenGL_GrBackend, backend_context, options));
  if (context == nullptr) {
    FTL_LOG(INFO) << "Failed to setup Skia Gr context.";
    return;
  }

  offscreen->ClearCurrent();

  context_ = std::move(context);
  offscreen_context_ = std::move(offscreen);
}

AndroidSurfaceGL::~AndroidSurfaceGL() = default;

bool AndroidSurfaceGL::IsOffscreenContextValid() const {
  return offscreen_context_ && offscreen_context_->IsValid();
}

void AndroidSurfaceGL::TeardownOnScreenContext() {
  ftl::AutoResetWaitableEvent latch;
  blink::Threads::Gpu()->PostTask([this, &latch]() {
    if (IsValid()) {
      GLContextClearCurrent();
    }
    latch.Signal();
  });
  latch.Wait();
  onscreen_context_ = nullptr;
}

bool AndroidSurfaceGL::IsValid() const {
  if (!onscreen_context_ || !offscreen_context_) {
    return false;
  }

  return onscreen_context_->IsValid() && offscreen_context_->IsValid();
}

std::unique_ptr<Surface> AndroidSurfaceGL::CreateGPUSurface() {
  auto surface = std::make_unique<GPUSurfaceGL>(context_.get(), this);

  if (!surface->Setup()) {
    FTL_LOG(ERROR) << "Surface could not be setup.";
    return nullptr;
  }

  return surface;
}

SkISize AndroidSurfaceGL::OnScreenSurfaceSize() const {
  FTL_DCHECK(onscreen_context_ && onscreen_context_->IsValid());
  return onscreen_context_->GetSize();
}

bool AndroidSurfaceGL::OnScreenSurfaceResize(const SkISize& size) const {
  FTL_DCHECK(onscreen_context_ && onscreen_context_->IsValid() &&
             render_target_ && render_target_->IsValid());
  return onscreen_context_->Resize(size) && render_target_->Resize(size);
}

bool AndroidSurfaceGL::ResourceContextMakeCurrent() {
  FTL_DCHECK(offscreen_context_ && offscreen_context_->IsValid());
  return offscreen_context_->MakeCurrent();
}

bool AndroidSurfaceGL::SetNativeWindow(ftl::RefPtr<AndroidNativeWindow> window,
                                       PlatformView::SurfaceConfig config) {
  // In any case, we want to get rid of our current onscreen context.
  onscreen_context_ = nullptr;
  render_target_ = nullptr;

  // If the offscreen context has not been setup, we dont have the sharegroup.
  // So bail.
  if (!offscreen_context_ || !offscreen_context_->IsValid()) {
    return false;
  }

  // Create the onscreen context.
  auto onscreen = ftl::MakeRefCounted<AndroidContextGL>(
      offscreen_context_->Environment(),  // GL environment
      config,                             // surface config
      std::move(window),                  // native window
      offscreen_context_.get()            // sharegroup
      );

  if (!onscreen->IsValid()) {
    FTL_LOG(ERROR) << "Could not setup onscreen context.";
    return false;
  }

  if (!onscreen->MakeCurrent()) {
    FTL_LOG(ERROR)
        << "Could not make the context current for render target setup.";
    return false;
  }

  auto target =
      std::make_unique<GLRenderTarget>(context_.get(), onscreen->GetSize());
  if (!target->IsValid()) {
    FTL_LOG(ERROR) << "Could not setup render target for onscreen context.";
    return false;
  }

  onscreen->ClearCurrent();

  onscreen_context_ = std::move(onscreen);
  render_target_ = std::move(target);
  return true;
}

bool AndroidSurfaceGL::GLContextMakeCurrent() {
  FTL_DCHECK(onscreen_context_ && onscreen_context_->IsValid());
  return onscreen_context_->MakeCurrent();
}

bool AndroidSurfaceGL::GLContextClearCurrent() {
  FTL_DCHECK(onscreen_context_ && onscreen_context_->IsValid());
  return onscreen_context_->ClearCurrent();
}

bool AndroidSurfaceGL::GLContextPresent() {
  FTL_DCHECK(onscreen_context_ && onscreen_context_->IsValid() &&
             render_target_ && render_target_->IsValid());
  if (!render_target_->RenderToWindowFBO()) {
    FTL_LOG(ERROR) << "Could draw to the onscreen surface.";
    return false;
  }
  return onscreen_context_->SwapBuffers();
}

intptr_t AndroidSurfaceGL::GLContextFBO() const {
  FTL_DCHECK(onscreen_context_ && onscreen_context_->IsValid() &&
             render_target_ && render_target_->IsValid());
  return render_target_->GetFBO();
}

void AndroidSurfaceGL::SetFlutterView(
    const fml::jni::JavaObjectWeakGlobalRef& flutter_view) {}

}  // namespace shell

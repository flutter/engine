// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/android/android_surface_gl_impeller.h"

#include "flutter/fml/logging.h"
#include "flutter/impeller/toolkit/egl/surface.h"
#include "flutter/shell/gpu/gpu_surface_gl_impeller.h"

namespace flutter {

AndroidSurfaceGLImpeller::AndroidSurfaceGLImpeller(
<<<<<<< HEAD
    const std::shared_ptr<AndroidContextGLImpeller>& android_context)
    : android_context_(android_context) {
  offscreen_surface_ = android_context_->CreateOffscreenSurface();

  if (!offscreen_surface_) {
=======
    const std::shared_ptr<AndroidContext>& android_context,
    const std::shared_ptr<PlatformViewAndroidJNI>& jni_facade,
    std::unique_ptr<impeller::egl::Display> display)
    : AndroidSurface(android_context),
      reactor_worker_(std::shared_ptr<ReactorWorker>(new ReactorWorker())),
      display_(std::move(display)) {
  if (!display_->IsValid()) {
    FML_DLOG(ERROR) << "Could not create surface with invalid Display.";
    return;
  }

  impeller::egl::ConfigDescriptor desc;
  desc.api = impeller::egl::API::kOpenGLES2;
  desc.color_format = impeller::egl::ColorFormat::kRGBA8888;
  desc.depth_bits = impeller::egl::DepthBits::kZero;
  desc.stencil_bits = impeller::egl::StencilBits::kEight;
  desc.samples = impeller::egl::Samples::kFour;

  desc.surface_type = impeller::egl::SurfaceType::kWindow;
  std::unique_ptr<impeller::egl::Config> onscreen_config =
      display_->ChooseConfig(desc);
  if (!onscreen_config) {
    // Fallback for Android emulator.
    desc.samples = impeller::egl::Samples::kOne;
    onscreen_config = display_->ChooseConfig(desc);
    if (onscreen_config) {
      FML_LOG(INFO) << "Warning: This device doesn't support MSAA for onscreen "
                       "framebuffers. Falling back to a single sample.";
    } else {
      FML_DLOG(ERROR) << "Could not choose onscreen config.";
      return;
    }
  }

  desc.surface_type = impeller::egl::SurfaceType::kPBuffer;
  auto offscreen_config = display_->ChooseConfig(desc);
  if (!offscreen_config) {
    FML_DLOG(ERROR) << "Could not choose offscreen config.";
    return;
  }

  auto onscreen_context = display_->CreateContext(*onscreen_config, nullptr);
  if (!onscreen_context) {
    FML_DLOG(ERROR) << "Could not create onscreen context.";
    return;
  }

  auto offscreen_context =
      display_->CreateContext(*offscreen_config, onscreen_context.get());
  if (!offscreen_context) {
    FML_DLOG(ERROR) << "Could not create offscreen context.";
    return;
  }

  auto offscreen_surface =
      display_->CreatePixelBufferSurface(*offscreen_config, 1u, 1u);
  if (!offscreen_surface) {
>>>>>>> upstream/main
    FML_DLOG(ERROR) << "Could not create offscreen surface.";
    return;
  }

<<<<<<< HEAD
=======
  if (!offscreen_context->MakeCurrent(*offscreen_surface)) {
    FML_DLOG(ERROR) << "Could not make offscreen context current.";
    return;
  }

  auto impeller_context = CreateImpellerContext(reactor_worker_);

  if (!impeller_context) {
    FML_DLOG(ERROR) << "Could not create Impeller context.";
    return;
  }

  if (!offscreen_context->ClearCurrent()) {
    FML_DLOG(ERROR) << "Could not clear offscreen context.";
    return;
  }

  // Setup context listeners.
  impeller::egl::Context::LifecycleListener listener =
      [worker =
           reactor_worker_](impeller::egl ::Context::LifecycleEvent event) {
        switch (event) {
          case impeller::egl::Context::LifecycleEvent::kDidMakeCurrent:
            worker->SetReactionsAllowedOnCurrentThread(true);
            break;
          case impeller::egl::Context::LifecycleEvent::kWillClearCurrent:
            worker->SetReactionsAllowedOnCurrentThread(false);
            break;
        }
      };
  if (!onscreen_context->AddLifecycleListener(listener).has_value() ||
      !offscreen_context->AddLifecycleListener(listener).has_value()) {
    FML_DLOG(ERROR) << "Could not add lifecycle listeners";
  }

  onscreen_config_ = std::move(onscreen_config);
  offscreen_config_ = std::move(offscreen_config);
  offscreen_surface_ = std::move(offscreen_surface);
  onscreen_context_ = std::move(onscreen_context);
  offscreen_context_ = std::move(offscreen_context);
  impeller_context_ = std::move(impeller_context);

>>>>>>> upstream/main
  // The onscreen surface will be acquired once the native window is set.

  is_valid_ = true;
}

AndroidSurfaceGLImpeller::~AndroidSurfaceGLImpeller() = default;

// |AndroidSurface|
bool AndroidSurfaceGLImpeller::IsValid() const {
  return is_valid_;
}

// |AndroidSurface|
std::unique_ptr<Surface> AndroidSurfaceGLImpeller::CreateGPUSurface(
    GrDirectContext* gr_context) {
  auto surface = std::make_unique<GPUSurfaceGLImpeller>(
      this,                                   // delegate
      android_context_->GetImpellerContext()  // context
  );
  if (!surface->IsValid()) {
    return nullptr;
  }
  return surface;
}

// |AndroidSurface|
void AndroidSurfaceGLImpeller::TeardownOnScreenContext() {
  GLContextClearCurrent();
  onscreen_surface_.reset();
}

// |AndroidSurface|
bool AndroidSurfaceGLImpeller::OnScreenSurfaceResize(const SkISize& size) {
  // The size is unused. It was added only for iOS where the sizes were
  // necessary to re-create auxiliary buffers (stencil, depth, etc.).
  return RecreateOnscreenSurfaceAndMakeOnscreenContextCurrent();
}

// |AndroidSurface|
bool AndroidSurfaceGLImpeller::ResourceContextMakeCurrent() {
  if (!offscreen_surface_) {
    return false;
  }
  return android_context_->ResourceContextMakeCurrent(offscreen_surface_.get());
}

// |AndroidSurface|
bool AndroidSurfaceGLImpeller::ResourceContextClearCurrent() {
  return android_context_->ResourceContextClearCurrent();
}

// |AndroidSurface|
bool AndroidSurfaceGLImpeller::SetNativeWindow(
    fml::RefPtr<AndroidNativeWindow> window) {
  native_window_ = std::move(window);
  return RecreateOnscreenSurfaceAndMakeOnscreenContextCurrent();
}

// |AndroidSurface|
std::unique_ptr<Surface> AndroidSurfaceGLImpeller::CreateSnapshotSurface() {
  FML_UNREACHABLE();
}

// |AndroidSurface|
std::shared_ptr<impeller::Context>
AndroidSurfaceGLImpeller::GetImpellerContext() {
  return android_context_->GetImpellerContext();
}

// |GPUSurfaceGLDelegate|
std::unique_ptr<GLContextResult>
AndroidSurfaceGLImpeller::GLContextMakeCurrent() {
  return std::make_unique<GLContextDefaultResult>(OnGLContextMakeCurrent());
}

bool AndroidSurfaceGLImpeller::OnGLContextMakeCurrent() {
  if (!onscreen_surface_) {
    return false;
  }

  return android_context_->OnscreenContextMakeCurrent(onscreen_surface_.get());
}

// |GPUSurfaceGLDelegate|
bool AndroidSurfaceGLImpeller::GLContextClearCurrent() {
  if (!onscreen_surface_) {
    return false;
  }

  return android_context_->OnscreenContextClearCurrent();
}

// |GPUSurfaceGLDelegate|
SurfaceFrame::FramebufferInfo
AndroidSurfaceGLImpeller::GLContextFramebufferInfo() const {
  auto info = SurfaceFrame::FramebufferInfo{};
  info.supports_readback = true;
  info.supports_partial_repaint = false;
  return info;
}

// |GPUSurfaceGLDelegate|
void AndroidSurfaceGLImpeller::GLContextSetDamageRegion(
    const std::optional<SkIRect>& region) {
  // Not supported.
}

// |GPUSurfaceGLDelegate|
bool AndroidSurfaceGLImpeller::GLContextPresent(
    const GLPresentInfo& present_info) {
  // The FBO ID is superfluous and was introduced for iOS where the default
  // framebuffer was not FBO0.
  if (!onscreen_surface_) {
    return false;
  }
  return onscreen_surface_->Present();
}

// |GPUSurfaceGLDelegate|
GLFBOInfo AndroidSurfaceGLImpeller::GLContextFBO(GLFrameInfo frame_info) const {
  // FBO0 is the default window bound framebuffer in EGL environments.
  return GLFBOInfo{
      .fbo_id = 0,
  };
}

// |GPUSurfaceGLDelegate|
sk_sp<const GrGLInterface> AndroidSurfaceGLImpeller::GetGLInterface() const {
  return nullptr;
}

bool AndroidSurfaceGLImpeller::
    RecreateOnscreenSurfaceAndMakeOnscreenContextCurrent() {
  GLContextClearCurrent();
  if (!native_window_) {
    return false;
  }
  onscreen_surface_.reset();
  auto onscreen_surface =
      android_context_->CreateOnscreenSurface(native_window_->handle());
  if (!onscreen_surface) {
    FML_DLOG(ERROR) << "Could not create onscreen surface.";
    return false;
  }
  onscreen_surface_ = std::move(onscreen_surface);
  return OnGLContextMakeCurrent();
}

}  // namespace flutter

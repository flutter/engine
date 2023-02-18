// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/common/shell_test_platform_view_gl.h"

#include <utility>

#include "flutter/common/graphics/persistent_cache.h"
#include "flutter/shell/common/context_options.h"
#include "flutter/shell/gpu/gpu_surface_gl_skia.h"

namespace flutter {
namespace testing {

ShellTestPlatformViewGL::ShellTestPlatformViewGL(
    PlatformView::Delegate& delegate,
    const TaskRunners& task_runners,
    std::shared_ptr<ShellTestVsyncClock> vsync_clock,
    CreateVsyncWaiter create_vsync_waiter,
    std::shared_ptr<ShellTestExternalViewEmbedder>
        shell_test_external_view_embedder)
    : ShellTestPlatformView(delegate, task_runners),
      gl_surface_(SkISize::Make(800, 600)),
      create_vsync_waiter_(std::move(create_vsync_waiter)),
      vsync_clock_(std::move(vsync_clock)),
      shell_test_external_view_embedder_(
          std::move(shell_test_external_view_embedder)) {}

ShellTestPlatformViewGL::~ShellTestPlatformViewGL() = default;

std::unique_ptr<VsyncWaiter> ShellTestPlatformViewGL::CreateVSyncWaiter() {
  return create_vsync_waiter_();
}

// |ShellTestPlatformView|
void ShellTestPlatformViewGL::SimulateVSync() {
  vsync_clock_->SimulateVSync();
}

// Default maximum number of bytes of GPU memory of budgeted resources in the
// cache.
// The shell will dynamically increase or decrease this cache based on the
// viewport size, unless a user has specifically requested a size on the Skia
// system channel.
static const size_t kGrCacheMaxByteSize = 24 * (1 << 20);

sk_sp<GrDirectContext> ShellTestPlatformViewGL::CreateMainContext() {
  auto context_switch = GLContextMakeCurrent();
  if (!context_switch->GetResult()) {
    FML_LOG(ERROR)
        << "Could not make the context current to set up the Gr context.";
    return nullptr;
  }

  const auto options =
      MakeDefaultContextOptions(ContextType::kRender, GrBackendApi::kOpenGL);

  auto context = GrDirectContext::MakeGL(GetGLInterface(), options);

  if (!context) {
    FML_LOG(ERROR) << "Failed to set up Skia Gr context.";
    return nullptr;
  }

  context->setResourceCacheLimit(kGrCacheMaxByteSize);

  PersistentCache::GetCacheForProcess()->PrecompileKnownSkSLs(context.get());

  return context;
}

// |PlatformView|
std::unique_ptr<Surface> ShellTestPlatformViewGL::CreateRenderingSurface() {
  return std::make_unique<GPUSurfaceGLSkia>(CreateMainContext(), this, true);
}

// |PlatformView|
std::shared_ptr<ExternalViewEmbedder>
ShellTestPlatformViewGL::CreateExternalViewEmbedder() {
  return shell_test_external_view_embedder_;
}

// |PlatformView|
PointerDataDispatcherMaker ShellTestPlatformViewGL::GetDispatcherMaker() {
  return [](DefaultPointerDataDispatcher::Delegate& delegate) {
    return std::make_unique<SmoothPointerDataDispatcher>(delegate);
  };
}

// |GPUSurfaceGLDelegate|
std::unique_ptr<GLContextResult>
ShellTestPlatformViewGL::GLContextMakeCurrent() {
  return std::make_unique<GLContextDefaultResult>(gl_surface_.MakeCurrent());
}

// |GPUSurfaceGLDelegate|
bool ShellTestPlatformViewGL::GLContextClearCurrent() {
  return gl_surface_.ClearCurrent();
}

// |GPUSurfaceGLDelegate|
bool ShellTestPlatformViewGL::GLContextPresent(
    const GLPresentInfo& present_info) {
  return gl_surface_.Present();
}

// |GPUSurfaceGLDelegate|
GLFBOInfo ShellTestPlatformViewGL::GLContextFBO(GLFrameInfo frame_info) const {
  return GLFBOInfo{
      .fbo_id = gl_surface_.GetFramebuffer(frame_info.width, frame_info.height),
  };
}

// |GPUSurfaceGLDelegate|
GPUSurfaceGLDelegate::GLProcResolver
ShellTestPlatformViewGL::GetGLProcResolver() const {
  return [surface = &gl_surface_](const char* name) -> void* {
    return surface->GetProcAddress(name);
  };
}

}  // namespace testing
}  // namespace flutter

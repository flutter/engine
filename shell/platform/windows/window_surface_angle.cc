#include "flutter/shell/platform/windows/window_surface_angle.h"

#include "flutter/shell/platform/windows/flutter_windows_engine.h"

namespace flutter {

// ID for the window frame buffer.
inline constexpr uint32_t kWindowFrameBufferID = 0;

WindowSurfaceAngle::WindowSurfaceAngle(AngleSurfaceManager* surface_manager) : surface_manager_(surface_manager) {}

WindowSurfaceAngle::~WindowSurfaceAngle() {}

void WindowSurfaceAngle::Init(PlatformWindow window, unsigned int width, unsigned int height) {
  window_ = window;
  auto rt = WindowsRenderTarget(window_);
  surface_manager_->CreateSurface(&rt, width, height);
}

void WindowSurfaceAngle::Destroy() {
  surface_manager_->DestroySurface();
  window_ = nullptr;
}

void WindowSurfaceAngle::GetSurfaceDimensions(unsigned int &width, unsigned int &height) {
  EGLint egl_width, egl_height;
  surface_manager_->GetSurfaceDimensions(&egl_width, &egl_height);
  width = egl_width;
  height = egl_height;
}

static WindowSurfaceAngle* GetWindowSurface(void* user_data) {
  auto host = static_cast<FlutterWindowsEngine*>(user_data);
  return static_cast<WindowSurfaceAngle*>(host->surface());
}

// Creates and returns a FlutterRendererConfig that renders to the view (if any)
// of a FlutterWindowsEngine, using OpenGL (via ANGLE).
// The user_data received by the render callbacks refers to the
// FlutterWindowsEngine.
void WindowSurfaceAngle::InitRendererConfig(FlutterRendererConfig& config) {
  config.type = kOpenGL;
  config.open_gl.struct_size = sizeof(config.open_gl);
  config.open_gl.make_current = [](void* user_data) -> bool {
    return GetWindowSurface(user_data)->surface_manager_->MakeCurrent();
  };
  config.open_gl.clear_current = [](void* user_data) -> bool {
    return GetWindowSurface(user_data)->surface_manager_->ClearContext();
  };
  config.open_gl.present = [](void* user_data) -> bool {
    auto surface = GetWindowSurface(user_data);
    auto hook = surface->GetWindowSurfaceHook();
    if (hook && !hook->OnPresentFramePre()) return false;
    bool result = surface->surface_manager_->SwapBuffers();
    if (hook) hook->OnPresentFramePost();
    return result;
  };
  config.open_gl.fbo_reset_after_present = true;
  config.open_gl.fbo_with_frame_info_callback =
      [](void* user_data, const FlutterFrameInfo* info) -> uint32_t {
    auto surface = GetWindowSurface(user_data);
    auto hook = surface->GetWindowSurfaceHook();
    if (hook) hook->OnAcquireFrame(info->size.width, info->size.height);
    return kWindowFrameBufferID;
  };
  config.open_gl.gl_proc_resolver = [](void* user_data,
                                       const char* what) -> void* {
    return reinterpret_cast<void*>(eglGetProcAddress(what));
  };
  config.open_gl.make_resource_current = [](void* user_data) -> bool {
    return GetWindowSurface(user_data)->surface_manager_->MakeResourceCurrent();
  };
  config.open_gl.gl_external_texture_frame_callback =
      [](void* user_data, int64_t texture_id, size_t width, size_t height,
         FlutterOpenGLTexture* texture) -> bool {
    auto host = static_cast<FlutterWindowsEngine*>(user_data);
    if (!host->texture_registrar()) {
      return false;
    }
    return host->texture_registrar()->PopulateTexture(texture_id, width, height,
                                                      texture);
  };
}

void WindowSurfaceAngle::CleanUpRendererConfig(FlutterRendererConfig& config) {}

void WindowSurfaceAngle::OnResize(unsigned int width, unsigned int height) {
  auto rt = WindowsRenderTarget(window_);
  surface_manager_->ResizeSurface(&rt, width, height);
  surface_manager_->MakeCurrent();
}

}  // namespace flutter

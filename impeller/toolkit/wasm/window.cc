// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/impeller/toolkit/wasm/window.h"

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <emscripten.h>

#include "impeller/base/validation.h"
#include "impeller/fixtures/gles/fixtures_shaders_gles.h"
#include "impeller/renderer/backend/gles/surface_gles.h"

namespace impeller::wasm {

static void ClearDepthEmulated(float depth) {}

static void DepthRangeEmulated(float nearVal, float farVal) {}

static std::vector<std::shared_ptr<fml::Mapping>>
ShaderLibraryMappingsForPlayground() {
  return {
      std::make_shared<fml::NonOwnedMapping>(
          impeller_fixtures_shaders_gles_data,
          impeller_fixtures_shaders_gles_length),
  };
}

Window::Window() {
  if (!egl_display_.IsValid()) {
    VALIDATION_LOG << "Could not create EGL display connection.";
    return;
  }

  emscripten_set_window_title("Impeller on Wasm");
  emscripten_set_canvas_size(1280, 800);

  egl::ConfigDescriptor egl_desc;
  egl_desc.api = egl::API::kOpenGLES2;
  egl_desc.samples = egl::Samples::kOne;
  egl_desc.color_format = egl::ColorFormat::kRGBA8888;
  egl_desc.stencil_bits = egl::StencilBits::kZero;
  egl_desc.depth_bits = egl::DepthBits::kZero;
  egl_desc.surface_type = egl::SurfaceType::kWindow;

  auto egl_config = egl_display_.ChooseConfig(egl_desc);
  if (!egl_config) {
    VALIDATION_LOG << "Could not choose an EGL config.";
    return;
  }

  auto egl_context = egl_display_.CreateContext(*egl_config, nullptr);
  if (!egl_context) {
    VALIDATION_LOG << "Could not create EGL context.";
    return;
  }

  // The native window type is NULL for emscripten as documented in
  // https://emscripten.org/docs/porting/multimedia_and_graphics/EGL-Support-in-Emscripten.html
  auto egl_surface = egl_display_.CreateWindowSurface(*egl_config, NULL);
  if (!egl_surface) {
    VALIDATION_LOG << "Could not create EGL surface.";
    return;
  }

  if (!egl_context->MakeCurrent(*egl_surface)) {
    VALIDATION_LOG << "Could not make context current.";
    return;
  }

  std::map<std::string, void*> gl_procs;

  gl_procs["glGetError"] = (void*)&glGetError;
  gl_procs["glClearDepthf"] = (void*)&ClearDepthEmulated;
  gl_procs["glDepthRangef"] = (void*)&DepthRangeEmulated;

#define IMPELLER_PROC(name) gl_procs["gl" #name] = (void*)&gl##name;
  FOR_EACH_IMPELLER_PROC(IMPELLER_PROC);
#undef IMPELLER_PROC

  auto gl = std::make_unique<ProcTableGLES>(
      [&gl_procs](const char* function_name) -> void* {
        auto found = gl_procs.find(function_name);
        if (found == gl_procs.end()) {
          return nullptr;
        }
        return found->second;
      });

  if (!gl->IsValid()) {
    VALIDATION_LOG << "Could not setup GL proc table.";
    return;
  }

  auto context = ContextGLES::Create(
      std::move(gl),                         // proc table
      ShaderLibraryMappingsForPlayground(),  // shader libraries
      false                                  // enable tracing
  );
  if (!context) {
    VALIDATION_LOG << "Could not create GL context.";
    return;
  }

  auto worker = std::make_shared<ReactorWorker>();

  if (!context->AddReactorWorker(worker).has_value()) {
    VALIDATION_LOG << "Could not add reactor worker.";
    return;
  }

  auto renderer = std::make_shared<Renderer>(context);
  if (!renderer || !renderer->IsValid()) {
    VALIDATION_LOG << "Could not create renderer.";
    return;
  }

  egl_surface_ = std::move(egl_surface);
  egl_context_ = std::move(egl_context);
  context_ = std::move(context);
  renderer_ = std::move(renderer);
  worker_ = std::move(worker);
  is_valid_ = true;
}

Window::~Window() = default;

bool Window::IsValid() const {
  return is_valid_;
}

bool Window::RenderFrame() {
  if (!IsValid()) {
    VALIDATION_LOG << "Context was invalid.";
    return false;
  }

  if (!egl_context_->MakeCurrent(*egl_surface_)) {
    VALIDATION_LOG << "Could not make the context current.";
    return false;
  }

  SurfaceGLES::SwapCallback swap_callback =
      [surface = egl_surface_, renderer_context = context_]() -> bool {
    if (!renderer_context->GetReactor()->React()) {
      VALIDATION_LOG << "Could not commit reactions.";
      return false;
    }
    return surface->Present();
  };
  auto surface =
      SurfaceGLES::WrapFBO(context_,                        // context
                           swap_callback,                   // swap callback
                           0u,                              // fbo
                           PixelFormat::kR8G8B8A8UNormInt,  // pixel format
                           GetWindowSize()                  // surface size
      );

  if (!surface || !surface->IsValid()) {
    VALIDATION_LOG << "Could not warp onscreen surface.";
    return false;
  }

  if (!renderer_->Render(std::move(surface),
                         [&](RenderTarget& render_target) -> bool {
                           return Render(render_target);
                         })) {
    VALIDATION_LOG << "Could not render.";
    return false;
  }

  return true;
}

ISize Window::GetWindowSize() const {
  int width = 0;
  int height = 0;
  int fullscreen = 0;
  emscripten_get_canvas_size(&width, &height, &fullscreen);
  return ISize::MakeWH(std::max<uint32_t>(0u, width),
                       std::max<uint32_t>(0u, height));
}

bool Window::Render(RenderTarget& target) {
  auto context = renderer_->GetContext();
  auto cmd_buffer = context->CreateCommandBuffer();
  auto pass = cmd_buffer->CreateRenderPass(target);
  pass->SetLabel("Root Render Pass");
  if (scene_ && !scene_->Render(*context, *pass)) {
    VALIDATION_LOG << "Could not render scene.";
    return false;
  }
  if (!pass->EncodeCommands()) {
    VALIDATION_LOG << "Could not encode commands.";
    return false;
  }
  if (!context->GetCommandQueue()->Submit({cmd_buffer}).ok()) {
    VALIDATION_LOG << "Could not submit command queue.";
    return false;
  }
  return true;
}

void Window::SetScene(std::unique_ptr<Scene> new_scene) {
  if (!IsValid()) {
    VALIDATION_LOG << "Could not set a scene on an invalid window.";
    return;
  }
  auto old_scene = std::move(scene_);
  if (old_scene) {
    if (!old_scene->Teardown(*renderer_->GetContext())) {
      VALIDATION_LOG << "Could not tear down old scene.";
      return;
    }
  }
  if (new_scene) {
    if (!new_scene->Setup(*renderer_->GetContext())) {
      VALIDATION_LOG << "Could not setup new scene.";
      return;
    }
    scene_ = std::move(new_scene);
  }
}

}  // namespace impeller::wasm

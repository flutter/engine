// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/impeller/toolkit/wasm/context.h"

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <emscripten.h>

#include "impeller/base/validation.h"
#include "impeller/renderer/backend/gles/surface_gles.h"

namespace impeller::wasm {

static void ClearDepthEmulated(float depth) {}

static void DepthRangeEmulated(float nearVal, float farVal) {}

Context::Context() {
  if (!display_.IsValid()) {
    VALIDATION_LOG << "Could not create EGL display connection.";
    return;
  }

  egl::ConfigDescriptor desc;
  desc.api = egl::API::kOpenGLES2;
  desc.samples = egl::Samples::kOne;
  desc.color_format = egl::ColorFormat::kRGBA8888;
  desc.stencil_bits = egl::StencilBits::kZero;
  desc.depth_bits = egl::DepthBits::kZero;
  desc.surface_type = egl::SurfaceType::kWindow;

  auto config = display_.ChooseConfig(desc);
  if (!config) {
    VALIDATION_LOG << "Could not choose an EGL config.";
    return;
  }

  auto context = display_.CreateContext(*config, nullptr);
  if (!context) {
    VALIDATION_LOG << "Could not create EGL context.";
    return;
  }

  // The native window type is NULL for emscripten as documented in
  // https://emscripten.org/docs/porting/multimedia_and_graphics/EGL-Support-in-Emscripten.html
  auto surface = display_.CreateWindowSurface(*config, NULL);
  if (!surface) {
    VALIDATION_LOG << "Could not create EGL surface.";
    return;
  }

  if (!context->MakeCurrent(*surface)) {
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

  auto renderer_context = ContextGLES::Create(std::move(gl),  // proc table
                                              {},    // shader libraries
                                              false  // enable tracing
  );
  if (!renderer_context) {
    VALIDATION_LOG << "Could not create GL context.";
    return;
  }

  auto worker = std::make_shared<ReactorWorker>();

  if (!renderer_context->AddReactorWorker(worker).has_value()) {
    VALIDATION_LOG << "Could not add reactor worker.";
    return;
  }

  auto renderer = std::make_shared<Renderer>(renderer_context);
  if (!renderer || !renderer->IsValid()) {
    VALIDATION_LOG << "Could not create renderer.";
    return;
  }

  surface_ = std::move(surface);
  context_ = std::move(context);
  renderer_context_ = std::move(renderer_context);
  renderer_ = std::move(renderer);
  reactor_worker_ = std::move(worker);
  is_valid_ = true;
}

Context::~Context() = default;

bool Context::IsValid() const {
  return is_valid_;
}

bool Context::RenderFrame() {
  if (!IsValid()) {
    VALIDATION_LOG << "Context was invalid.";
    return false;
  }

  if (!context_->MakeCurrent(*surface_)) {
    VALIDATION_LOG << "Could not make the context current.";
    return false;
  }

  SurfaceGLES::SwapCallback swap_callback =
      [surface = surface_, renderer_context = renderer_context_]() -> bool {
    if (!renderer_context->GetReactor()->React()) {
      VALIDATION_LOG << "Could not commit reactions.";
      return false;
    }
    return surface->Present();
  };
  auto surface =
      SurfaceGLES::WrapFBO(renderer_context_,               // context
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

ISize Context::GetWindowSize() const {
  int width = 0;
  int height = 0;
  int fullscreen = 0;
  emscripten_get_canvas_size(&width, &height, &fullscreen);
  return ISize::MakeWH(std::max<uint32_t>(0u, width),
                       std::max<uint32_t>(0u, height));
}

bool Context::Render(RenderTarget& target) {
  auto context = renderer_->GetContext();
  auto cmd_buffer = context->CreateCommandBuffer();
  auto pass = cmd_buffer->CreateRenderPass(target);
  pass->SetLabel("Root Render Pass");
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

}  // namespace impeller::wasm

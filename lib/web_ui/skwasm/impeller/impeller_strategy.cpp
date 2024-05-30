// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller_strategy.h"

#include "impeller/aiks/aiks_context.h"
#include "impeller/aiks/picture.h"
#include "impeller/display_list/dl_dispatcher.h"
#include "impeller/entity/gles/entity_shaders_gles.h"
#include "impeller/renderer/backend/gles/context_gles.h"
#include "impeller/renderer/backend/gles/surface_gles.h"
#include "impeller/renderer/renderer.h"

namespace {
std::vector<std::shared_ptr<fml::Mapping>>
ShaderLibraryMappingsForApplication() {
  return {
      std::make_shared<fml::NonOwnedMapping>(
          impeller_entity_shaders_gles_data,
          impeller_entity_shaders_gles_length),
  };
}
}  // namespace

namespace Skwasm {
class ReactorWorker : public impeller::ReactorGLES::Worker {
 public:
  ReactorWorker() { emscripten_console_log("hi worker\n"); }

  ~ReactorWorker() override { emscripten_console_log("bye worker\n"); }

  ReactorWorker(const ReactorWorker&) = delete;

  ReactorWorker& operator=(const ReactorWorker&) = delete;

  virtual bool CanReactorReactOnCurrentThreadNow(
      const impeller::ReactorGLES& reactor) const override {
    return true;
  }
};

sk_sp<GraphicsContext> createGraphicsContext() {
  auto clearDepthEmulated = [](float depth) {};
  auto depthRangeEmulated = [](float nearVal, float farVal) {};

  static std::map<std::string, void*> gl_procs;

  gl_procs["glGetError"] = (void*)&glGetError;
  gl_procs["glClearDepthf"] = (void*)&clearDepthEmulated;
  gl_procs["glDepthRangef"] = (void*)&depthRangeEmulated;

#define IMPELLER_PROC(name) gl_procs["gl" #name] = (void*)&gl##name;
  FOR_EACH_IMPELLER_PROC(IMPELLER_PROC);
  FOR_EACH_IMPELLER_ES_ONLY_PROC(IMPELLER_PROC);
  FOR_EACH_IMPELLER_GLES3_PROC(IMPELLER_PROC);
  // IMPELLER_PROC(DebugMessageControlKHR);
  // IMPELLER_PROC(DiscardFramebufferEXT);
  // IMPELLER_PROC(FramebufferTexture2DMultisampleEXT);
  // IMPELLER_PROC(PushDebugGroupKHR);
  // IMPELLER_PROC(PopDebugGroupKHR);
  // IMPELLER_PROC(ObjectLabelKHR);
  // IMPELLER_PROC(RenderbufferStorageMultisampleEXT);
  IMPELLER_PROC(GenQueriesEXT);
  IMPELLER_PROC(DeleteQueriesEXT);
  IMPELLER_PROC(GetQueryObjectui64vEXT);
  IMPELLER_PROC(BeginQueryEXT);
  IMPELLER_PROC(EndQueryEXT);
  IMPELLER_PROC(GetQueryObjectuivEXT);
#undef IMPELLER_PROC

  auto gl = std::make_unique<impeller::ProcTableGLES>(
      [](const char* function_name) -> void* {
        auto found = gl_procs.find(function_name);
        if (found == gl_procs.end()) {
          emscripten_console_log("failed to find");
          emscripten_console_log(function_name);
          return nullptr;
        }
        emscripten_console_log(function_name);
        return found->second;
      });

  auto context = impeller::ContextGLES::Create(
      std::move(gl), ShaderLibraryMappingsForApplication(), false);
  emscripten_console_log(
      static_cast<std::shared_ptr<impeller::Context>>(context)->IsValid()
          ? "context valid"
          : "context invalid");

  auto worker = std::make_shared<ReactorWorker>();
  context->AddReactorWorker(worker);
  emscripten_console_log("added worker\n");
  auto renderer = std::make_shared<impeller::Renderer>(context);
  return sk_make_sp<GraphicsContext>(std::move(context), std::move(renderer),
                                     std::move(worker));
};

GraphicsContext::GraphicsContext(std::shared_ptr<impeller::ContextGLES> context,
                                 std::shared_ptr<impeller::Renderer> renderer,
                                 std::shared_ptr<ReactorWorker> worker)
    : _context(std::move(context)),
      _renderer(std::move(renderer)),
      _worker(std::move(worker)) {
  emscripten_console_log("RETAINING worker\n");
}

sk_sp<GraphicsSurface> GraphicsContext::createSurface(int width, int height) {
  return sk_make_sp<GraphicsSurface>(_context, _renderer, width, height);
}

GraphicsSurface::GraphicsSurface(std::shared_ptr<impeller::ContextGLES> context,
                                 std::shared_ptr<impeller::Renderer> renderer,
                                 int width,
                                 int height)
    : _context(std::move(context)),
      _renderer(std::move(renderer)),
      _width(width),
      _height(height) {}

GraphicsSurface::~GraphicsSurface() = default;

void GraphicsSurface::renderPicture(const impeller::Picture& picture) {
  emscripten_console_log("inside render picture\n");
  auto surface = impeller::SurfaceGLES::WrapFBO(
      _context,                                  // context
      []() { return true; },                     // swap callback
      0u,                                        // fbo
      impeller::PixelFormat::kR8G8B8A8UNormInt,  // pixel format
      {_width, _height}                          // surface size
  );
  emscripten_console_log(surface->IsValid() ? "valid surface\n"
                                            : "invalid surface\n");
  bool result = _renderer->Render(
      std::move(surface), [this, &picture](impeller::RenderTarget& target) {
        emscripten_console_log("inside render callback\n");
        impeller::AiksContext context(_context, nullptr);
        emscripten_console_log("inside render callback\n");
        bool result = context.Render(picture, target, true);
        emscripten_console_log(result ? "rendered true\n" : "rendered false\n");
        return result;
      });
  emscripten_console_log(result ? "after render: true\n"
                                : "after render: false");
}

void drawPictureToSurface(Picture* picture,
                          GraphicsSurface* surface,
                          Scalar offsetX,
                          Scalar offsetY) {
  emscripten_console_log("inside draaw picture\n");
  impeller::DlDispatcher dispatcher;
  picture->Dispatch(dispatcher);
  auto impellerPicture = dispatcher.EndRecordingAsPicture();

  surface->renderPicture(impellerPicture);
}
}  // namespace Skwasm

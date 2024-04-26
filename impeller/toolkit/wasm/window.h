// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_TOOLKIT_WASM_CONTEXT_H_
#define FLUTTER_IMPELLER_TOOLKIT_WASM_CONTEXT_H_

#include "impeller/renderer/backend/gles/context_gles.h"
#include "impeller/renderer/renderer.h"
#include "impeller/toolkit/egl/context.h"
#include "impeller/toolkit/egl/display.h"
#include "impeller/toolkit/egl/surface.h"
#include "impeller/toolkit/wasm/reactor_worker.h"
#include "impeller/toolkit/wasm/scene.h"

namespace impeller::wasm {

class Window {
 public:
  Window();

  ~Window();

  Window(const Window&) = delete;

  Window& operator=(const Window&) = delete;

  bool IsValid() const;

  bool RenderFrame();

  void SetScene(std::unique_ptr<Scene> scene);

 private:
  egl::Display egl_display_;
  std::shared_ptr<egl::Context> egl_context_;
  std::shared_ptr<egl::Surface> egl_surface_;
  std::shared_ptr<ContextGLES> context_;
  std::shared_ptr<ReactorWorker> worker_;
  std::shared_ptr<Renderer> renderer_;
  std::unique_ptr<Scene> scene_;
  bool is_valid_ = false;

  ISize GetWindowSize() const;

  bool Render(RenderTarget& target);
};

}  // namespace impeller::wasm

#endif  // FLUTTER_IMPELLER_TOOLKIT_WASM_CONTEXT_H_

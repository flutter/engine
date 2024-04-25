// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_TOOLKIT_WASM_CONTEXT_H_
#define FLUTTER_IMPELLER_TOOLKIT_WASM_CONTEXT_H_

#include "impeller/renderer/backend/gles/context_gles.h"
#include "impeller/toolkit/egl/context.h"
#include "impeller/toolkit/egl/display.h"
#include "impeller/toolkit/egl/surface.h"
#include "reactor_worker.h"

namespace impeller::wasm {

class Context {
 public:
  Context();

  ~Context();

  Context(const Context&) = delete;

  Context& operator=(const Context&) = delete;

  bool IsValid() const;

  bool RenderFrame();

 private:
  egl::Display display_;
  std::shared_ptr<egl::Context> context_;
  std::shared_ptr<egl::Surface> surface_;
  std::shared_ptr<ContextGLES> renderer_context_;
  std::shared_ptr<ReactorWorker> reactor_worker_;
  bool is_valid_ = false;

  ISize GetWindowSize() const;
};

}  // namespace impeller::wasm

#endif  // FLUTTER_IMPELLER_TOOLKIT_WASM_CONTEXT_H_

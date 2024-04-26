// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_TOOLKIT_WASM_SCENE_H_
#define FLUTTER_IMPELLER_TOOLKIT_WASM_SCENE_H_

#include "impeller/renderer/context.h"
#include "impeller/renderer/render_pass.h"

namespace impeller::wasm {

class Scene {
 public:
  Scene();

  virtual ~Scene();

  virtual bool Setup(const Context& context) = 0;

  virtual bool Render(const Context& context, RenderPass& pass) = 0;

  virtual bool Teardown(const Context& context) = 0;

  Scene(const Scene&) = delete;

  Scene& operator=(const Scene&) = delete;
};

}  // namespace impeller::wasm

#endif  // FLUTTER_IMPELLER_TOOLKIT_WASM_SCENE_H_

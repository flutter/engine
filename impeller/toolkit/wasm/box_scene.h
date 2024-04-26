// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "impeller/toolkit/wasm/scene.h"

namespace impeller::wasm {

class BoxScene final : public Scene {
 public:
  BoxScene();

  ~BoxScene();

  BoxScene(const BoxScene&) = delete;

  BoxScene& operator=(const BoxScene&) = delete;

  // |Scene|
  bool Setup(const Context& context) override;

  // |Scene|
  bool Render(const Context& context, RenderPass& pass) override;

  // |Scene|
  bool Teardown(const Context& context) override;
};

}  // namespace impeller::wasm

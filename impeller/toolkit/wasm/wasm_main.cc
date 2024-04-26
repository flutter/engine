// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <emscripten.h>

#include <memory>

#include "flutter/fml/message_loop.h"
#include "impeller/toolkit/wasm/box_scene.h"
#include "impeller/toolkit/wasm/fixtures_store.h"
#include "impeller/toolkit/wasm/impeller_scene.h"
#include "impeller/toolkit/wasm/swapchain.h"
#include "impeller/toolkit/wasm/window.h"

static std::shared_ptr<impeller::wasm::Swapchain> gSwapchain;
static std::shared_ptr<impeller::wasm::Window> gWindow;

int main(int argc, char const* argv[]) {
  fml::MessageLoop::EnsureInitializedForCurrentThread();
  static std::shared_ptr<impeller::wasm::FixturesStore> gStore =
      std::make_shared<impeller::wasm::FixturesStore>([]() {
        impeller::wasm::SetDefaultStore(gStore);
        gWindow = std::make_shared<impeller::wasm::Window>();
        FML_CHECK(gWindow->IsValid());
        gWindow->SetScene(std::make_unique<impeller::wasm::ImpellerScene>());
        // gWindow->SetScene(std::make_unique<impeller::wasm::BoxScene>());
        gSwapchain = std::make_shared<impeller::wasm::Swapchain>(
            []() { return gWindow->RenderFrame(); });
      });
  return 0;
}

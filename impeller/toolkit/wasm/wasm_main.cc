// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <emscripten.h>

#include <memory>

#include "flutter/fml/message_loop.h"
#include "impeller/toolkit/wasm/context.h"
#include "impeller/toolkit/wasm/swapchain.h"

static std::shared_ptr<impeller::wasm::Swapchain> gSwapchain;
static std::shared_ptr<impeller::wasm::Context> gContext;

int main(int argc, char const* argv[]) {
  fml::MessageLoop::EnsureInitializedForCurrentThread();
  gContext = std::make_shared<impeller::wasm::Context>();
  FML_CHECK(gContext->IsValid());
  gSwapchain = std::make_shared<impeller::wasm::Swapchain>(
      []() { return gContext->RenderFrame(); });

  return 0;
}

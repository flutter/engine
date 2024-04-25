// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/impeller/toolkit/wasm/swapchain.h"

#include <emscripten.h>

namespace impeller::wasm {

Swapchain::Swapchain(SwapchainCallback callback)
    : callback_(std::move(callback)) {
  AwaitVsync();
}

Swapchain::~Swapchain() {
  Terminate();
}

void Swapchain::Terminate() {
  terminated_ = true;
}

void Swapchain::AwaitVsync() {
  emscripten_async_call(
      [](void* arg) {
        auto swapchain = reinterpret_cast<Swapchain*>(arg);
        if (swapchain->InvokeCallback()) {
          swapchain->AwaitVsync();
        }
      },
      this, -1);
}

bool Swapchain::InvokeCallback() const {
  return callback_ ? callback_() : false;
}

}  // namespace impeller::wasm

// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_TOOLKIT_WASM_SWAPCHAIN_H_
#define FLUTTER_IMPELLER_TOOLKIT_WASM_SWAPCHAIN_H_

#include <functional>

namespace impeller::wasm {

class Swapchain {
 public:
  using SwapchainCallback = std::function<bool(void)>;
  explicit Swapchain(SwapchainCallback callback);

  ~Swapchain();

  void Terminate();

  void SetCallback();

  Swapchain(const Swapchain&) = delete;

  Swapchain& operator=(const Swapchain&) = delete;

 private:
  SwapchainCallback callback_;
  bool terminated_ = false;

  void AwaitVsync();

  bool InvokeCallback() const;
};

}  // namespace impeller::wasm

#endif  // FLUTTER_IMPELLER_TOOLKIT_WASM_SWAPCHAIN_H_

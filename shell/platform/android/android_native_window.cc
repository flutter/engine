// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/android/android_native_window.h"

namespace shell {

AndroidNativeWindow::AndroidNativeWindow(Handle window,
                                         double rasterization_scale)
    : window_(window), rasterization_scale_(rasterization_scale) {}

AndroidNativeWindow::~AndroidNativeWindow() {
  if (window_ != nullptr) {
    ANativeWindow_release(window_);
    window_ = nullptr;
  }
}

bool AndroidNativeWindow::IsValid() const {
  return window_ != nullptr;
}

AndroidNativeWindow::Handle AndroidNativeWindow::handle() const {
  return window_;
}

bool AndroidNativeWindow::SetSize(const SkISize& size) {
  int32_t width = std::max<int32_t>(1, size.fWidth * rasterization_scale_);
  int32_t height = std::max<int32_t>(1, size.fHeight * rasterization_scale_);

  return ANativeWindow_setBuffersGeometry(
             window_,                          // window handle
             width,                            // width
             height,                           // height
             ANativeWindow_getFormat(window_)  // format (same as before)
             ) >= 0;
}

SkISize AndroidNativeWindow::GetSize() const {
  return window_ == nullptr ? SkISize::Make(0, 0)
                            : SkISize::Make(ANativeWindow_getWidth(window_),
                                            ANativeWindow_getHeight(window_));
}

}  // namespace shell

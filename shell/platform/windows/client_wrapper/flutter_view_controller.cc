// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "include/flutter/flutter_view_controller.h"

#include <algorithm>
#include <iostream>

namespace flutter {

FlutterViewController::FlutterViewController(int width,
                                             int height,
                                             const DartProject& project) {
  FlutterDesktopViewControllerProperties properties = {};
  properties.width = width;
  properties.height = height;

  engine_ = std::make_unique<FlutterEngine>(project);
  controller_ =
      FlutterDesktopEngineCreateViewController(engine_->engine(), &properties);

  if (!controller_) {
    std::cerr << "Failed to create view controller." << std::endl;
    return;
  }

  view_ = std::make_unique<FlutterView>(
      FlutterDesktopViewControllerGetView(controller_));
}

FlutterViewController::~FlutterViewController() {
  if (controller_) {
    FlutterDesktopViewControllerDestroy(controller_);
  }
}

void FlutterViewController::ForceRedraw() {
  FlutterDesktopViewControllerForceRedraw(controller_);
}

std::optional<LRESULT> FlutterViewController::HandleTopLevelWindowProc(
    HWND hwnd,
    UINT message,
    WPARAM wparam,
    LPARAM lparam) {
  LRESULT result;
  bool handled = FlutterDesktopViewControllerHandleTopLevelWindowProc(
      controller_, hwnd, message, wparam, lparam, &result);
  return handled ? result : std::optional<LRESULT>(std::nullopt);
}

}  // namespace flutter

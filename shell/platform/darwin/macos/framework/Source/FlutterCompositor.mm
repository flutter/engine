// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterCompositor.h"
#include "flutter/fml/logging.h"

namespace flutter {

FlutterCompositor::FlutterCompositor(FlutterViewController* view_controller) {
  FML_CHECK(view_controller != nullptr) << "FlutterViewController* cannot be nullptr";

  view_controller_ = view_controller;
}

void FlutterCompositor::SetPresentCallback(
    const FlutterCompositor::PresentCallback& present_callback) {
  present_callback_ = present_callback;
}

void FlutterCompositor::StartFrame() {
  frame_started_ = true;
}

size_t FlutterCompositor::CreateCALayer() {
  if (!view_controller_) {
    return 0;
  }

  // FlutterCompositor manages the lifecycle of content layers.
  // The id for a CALayer starts at 0 and increments by 1 for
  // any given frame.
  CALayer* content_layer = [[CALayer alloc] init];
  [view_controller_.flutterView.layer addSublayer:content_layer];
  ca_layer_map_[ca_layer_count_] = content_layer;
  return ca_layer_count_++;
}

}  // namespace flutter

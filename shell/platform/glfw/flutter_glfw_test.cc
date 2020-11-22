// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/glfw/public/flutter_glfw.h"

#include "gtest/gtest.h"

Test(FlutterGlfwTest, CreateWindow) {
  FlutterDesktopInit();
  FlutterDesktopWindowProperties window_properties;
  window_properties.title = "foo";
  window_properties.width = 100;
  window_properties.height = 100;
  FlutterDesktopEngineProperties engine_properties;
  engine_properties.assets_path = "";
  engine_properties.icu_data_path = "";
  auto ref = FlutterDesktopCreateWindow(window_properties, engine_properties);
  EXPECT_NE(ref, nullptr);
  FlutterDesktopDestroyWindow(ref);
  FlutterDesktopTerminate();
}

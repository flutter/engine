// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iostream>

#include "flutter/shell/platform/windows/flutter_windows_engine.h"
#include "flutter/shell/platform/windows/flutter_windows_texture_registrar.h"
#include "gtest/gtest.h"

namespace flutter {
namespace testing {

TEST(FlutterWindowsTextureRegistrarTest, CreateDestroy) {
  FlutterDesktopEngineProperties props = {};
  props.assets_path = L"FakeAssets";
  props.icu_data_path = L"icudtl.dat";

  FlutterProjectBundle bundle(props);
  FlutterWindowsEngine engine(bundle);

  FlutterWindowsTextureRegistrar registrar(&engine);

  EXPECT_TRUE(true);
}

TEST(FlutterWindowsTextureRegistrarTest, PopulateInvalidTexture) {
  FlutterDesktopEngineProperties props = {};
  props.assets_path = L"FakeAssets";
  props.icu_data_path = L"icudtl.dat";

  FlutterProjectBundle bundle(props);
  FlutterWindowsEngine engine(bundle);

  FlutterWindowsTextureRegistrar registrar(&engine);
  auto result = registrar.PopulateTexture(1, 640, 480, nullptr);
  EXPECT_FALSE(result);
}

// TODO Add additional tests for testing actual texture registration using
// FlutterEngineRegisterExternalTexture

}  // namespace testing
}  // namespace flutter

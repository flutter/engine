// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/painting/matrix.h"

#include "third_party/tonic/converter/dart_converter.h"

#include "flutter/shell/common/shell_test.h"
#include "flutter/testing/testing.h"

namespace flutter {
namespace testing {

using MatrixTest = ShellTest;

TEST_F(MatrixTest, ConvertsToSkM44Correctly) {
  Settings settings = CreateSettingsForFixture();
  std::unique_ptr<Shell> shell = CreateShell(settings);

  std::promise<bool> checkFinished[2];

  auto nativeCheckM44Conversion =
      [&finished = checkFinished[0]](Dart_NativeArguments args) {
        Dart_Handle handle = Dart_GetNativeArgument(args, 0);
        tonic::Float64List matrix4(handle);
        SkM44 m44 = flutter::ToSkM44(matrix4);
        for (int r = 0; r < 4; r += 1) {
          for (int c = 0; c < 4; c += 1) {
            ASSERT_EQ(m44.rc(r, c), r * 4 + c);
          }
        }
        finished.set_value(true);
      };
  AddNativeCallback("CheckM44Conversion",
                    CREATE_NATIVE_ENTRY(nativeCheckM44Conversion));

  auto nativeCheckIncompleteM44Conversion =
      [&finished = checkFinished[1]](Dart_NativeArguments args) {
        Dart_Handle handle = Dart_GetNativeArgument(args, 0);
        tonic::Float64List list1(handle);
        SkM44 m44 = flutter::ToSkM44(list1);
        for (int r = 0; r < 4; r += 1) {
          for (int c = 0; c < 4; c += 1) {
            if (r == 0 && c == 0) {
              ASSERT_EQ(m44.rc(r, c), 1);
            } else {
              ASSERT_EQ(m44.rc(r, c), 0);
            }
          }
        }
        finished.set_value(true);
      };
  AddNativeCallback("CheckIncompleteM44Conversion",
                    CREATE_NATIVE_ENTRY(nativeCheckIncompleteM44Conversion));

  auto configuration = RunConfiguration::InferFromSettings(settings);
  configuration.SetEntrypoint("testMatrix");
  RunEngine(shell.get(), std::move(configuration));

  checkFinished[0].get_future().wait();
  checkFinished[1].get_future().wait();

  DestroyShell(std::move(shell));
}

}  // namespace testing
}  // namespace flutter

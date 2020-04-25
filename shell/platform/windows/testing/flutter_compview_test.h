// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <windowsx.h>

#include "flutter/shell/platform/windows/flutter_comp_view.h"

namespace flutter {
namespace testing {

/// Test class for Win32FlutterWindow.
class FlutterCompViewTest : public FlutterCompView {
 public:
  FlutterCompViewTest(int width, int height);
  virtual ~FlutterCompViewTest();

  // Prevent copying.
  FlutterCompViewTest(FlutterCompViewTest const&) = delete;
  FlutterCompViewTest& operator=(FlutterCompViewTest const&) = delete;

  //// |Win32Window|
  //void OnFontChange() override;

  bool OnFontChangeWasCalled();

 private:
  bool on_font_change_called_ = false;
};

}  // namespace testing
}  // namespace flutter

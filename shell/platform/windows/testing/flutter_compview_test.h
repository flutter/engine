// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <windowsx.h>

#include "flutter/shell/platform/windows/flutter_windows_view.h"

namespace flutter {
namespace testing {

/// Test class for Win32FlutterWindow.
class FlutterWindowsViewTest : public FlutterWindowsView {
 public:
  FlutterWindowsViewTest(int width, int height);
  virtual ~FlutterWindowsViewTest();

  // Prevent copying.
  FlutterWindowsViewTest(FlutterWindowsViewTest const&) = delete;
  FlutterWindowsViewTest& operator=(FlutterWindowsViewTest const&) = delete;

  //// |Win32Window|
  //void OnFontChange() override;

  bool OnFontChangeWasCalled();

 private:
  bool on_font_change_called_ = false;
};

}  // namespace testing
}  // namespace flutter

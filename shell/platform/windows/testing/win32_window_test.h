// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <windowsx.h>

#include "flutter/fml/macros.h"
#include "flutter/shell/platform/windows/win32_window.h"

namespace flutter {
namespace testing {

class Win32WindowTest : public Win32Window {
 public:
  Win32WindowTest();
  ~Win32WindowTest();

  UINT GetDpi();

  // |Win32Window|
  void OnDpiScale(unsigned int dpi) override;

  // |Win32Window|
  void OnResize(unsigned int width, unsigned int height) override;

  // |Win32Window|
  void OnPointerMove(double x, double y) override;

  // |Win32Window|
  void OnPointerDown(double x, double y, UINT button) override;

  // |Win32Window|
  void OnPointerUp(double x, double y, UINT button) override;

  // |Win32Window|
  void OnPointerLeave() override;

  // |Win32Window|
  void OnChar(char32_t code_point) override;

  // |Win32Window|
  void OnKey(int key, int scancode, int action, int mods) override;

  // |Win32Window|
  void OnScroll(double delta_x, double delta_y) override;

  // |Win32Window|
  void OnClose();

  // |Win32Window|
  void OnFontChange() override;

 private:
  FML_DISALLOW_COPY_AND_ASSIGN(Win32WindowTest);
};

}  // namespace testing
}  // namespace flutter

#include "flutter/shell/platform/windows/testing/win32_flutter_window_test.h"

namespace flutter {
namespace testing {

Win32FlutterWindowTest::Win32FlutterWindowTest(int left,
                                               int top,
                                               int width,
                                               int height)
    : Win32FlutterWindow(left, top, width, height){};

Win32FlutterWindowTest::~Win32FlutterWindowTest() = default;

void Win32FlutterWindowTest::OnFontChange() {
  on_font_change_called_ = true;
}

bool Win32FlutterWindowTest::OnFontChangeWasCalled() {
  return on_font_change_called_;
}
}  // namespace testing
}  // namespace flutter

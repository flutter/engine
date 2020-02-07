#include "flutter/shell/platform/windows/testing/win32_window_test.h"
#include "gtest/gtest.h"

namespace flutter {
namespace testing {

TEST(Win32WindowTest, CreateDestroy) {
  Win32WindowTest window;
  ASSERT_TRUE(TRUE);
}

<<<<<<< HEAD
TEST(Win32WindowTest, GetDpiAfterCreate) {
  Win32WindowTest window;
  ASSERT_TRUE(window.GetDpi() > 0);
=======
TEST(Win32FlutterWindowTest, CanFontChange) {
  Win32FlutterWindowTest window(800, 600);
  HWND hwnd = window.GetWindowHandle();
  LRESULT result = SendMessage(hwnd, WM_FONTCHANGE, NULL, NULL);
  ASSERT_EQ(result, 0);
  ASSERT_TRUE(window.OnFontChangeWasCalled());
>>>>>>> master
}
}  // namespace testing
}  // namespace flutter

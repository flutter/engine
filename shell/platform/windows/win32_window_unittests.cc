#include "flutter/shell/platform/windows/testing/win32_window_test.h"
#include "gtest/gtest.h"

namespace flutter {
namespace testing {

TEST(Win32WindowTest, CreateDestroy) {
  Win32WindowTest window;
  ASSERT_TRUE(TRUE);
}

TEST(Win32WindowTest, CanFontChange) {
  Win32WindowTest window;
  window.InitializeChild("FLUTTERVIEW", 10, 10);

  HWND hwnd = window.GetWindowHandle();
  LRESULT result = SendMessage(hwnd, WM_FONTCHANGE, NULL, NULL);
  ASSERT_EQ(result, 0);
  ASSERT_TRUE(window.OnFontChangeWasCalled());
}

TEST(Win32WindowTest, GetDpiAfterCreate) {
  Win32WindowTest window;
  ASSERT_TRUE(window.GetDpi() > 0);
}
}  // namespace testing
}  // namespace flutter

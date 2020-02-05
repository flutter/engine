#include "flutter/shell/platform/windows/testing/win32_window_unittest.h"

#include "gtest/gtest.h"

namespace flutter {
namespace testing {

TEST(Win32WindowTest, GetDpiAfterCreate) {
  Win32WindowTest window;
  ASSERT_TRUE(window.GetDpi() > 0);
}

}  // namespace testing
}  // namespace flutter

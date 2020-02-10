#include "flutter/shell/platform/windows/dpi_utils.h"
#include "gtest/gtest.h"

namespace flutter {
namespace testing {

TEST(DpiUtilsTest, NonZero) {
  ASSERT_GT(GetDpiForHWND(nullptr),0);
  ASSERT_GT(GetDpiForMonitor(nullptr),0);
};

TEST(DpiUtilsTest, EqualDpis) {
  ASSERT_EQ(GetDpiForHWND(nullptr), GetDpiForMonitor(nullptr));
};

}  // namespace testing
}  // namespace flutter

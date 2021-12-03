#include "display.h"
#include "display_manager.h"

#include "gtest/gtest.h"

namespace flutter {
namespace testing {

TEST(DisplayManagerTest, SetsDisplayOnStartup) {
  DisplayManager manager;
  const double refresh_rate = 60;
  std::vector<std::unique_ptr<flutter::Display>> displays;
  displays.push_back(std::make_unique<flutter::Display>(refresh_rate));
  ASSERT_TRUE(manager.GetMainDisplayRefreshRate(), refresh_rate);
}

}
}
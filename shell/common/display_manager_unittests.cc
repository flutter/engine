#include "display.h"
#include "display_manager.h"

#include "flutter/fml/time/time_point.h"

#include "gtest/gtest.h"

namespace flutter {
namespace testing {

TEST(DisplayManagerTest, SetsDisplayOnStartup) {
  DisplayManager manager;
  const double refresh_rate = 60;
  std::vector<std::unique_ptr<flutter::Display>> displays;
  displays.push_back(std::make_unique<flutter::Display>(refresh_rate));
  manager.HandleDisplayUpdates(flutter::DisplayUpdateType::kStartup,
                               std::move(displays));
  ASSERT_EQ(manager.GetMainDisplayRefreshRate(), refresh_rate);
}

TEST(DisplayManagerTest, UpdatesDisplayIfTypeIsUpdateRefreshRate) {
  DisplayManager manager;
  const double refresh_rate = 60;
  std::vector<std::unique_ptr<flutter::Display>> displays1;
  displays1.push_back(std::make_unique<flutter::Display>(refresh_rate));
  manager.HandleDisplayUpdates(flutter::DisplayUpdateType::kStartup,
                               std::move(displays1));

  const double new_refresh_rate = 120;
  std::vector<std::unique_ptr<flutter::Display>> displays2;
  displays2.push_back(std::make_unique<flutter::Display>(new_refresh_rate));
  manager.HandleDisplayUpdates(flutter::DisplayUpdateType::kUpdateRefreshRate,
                               std::move(displays2));
  ASSERT_EQ(manager.GetMainDisplayRefreshRate(), new_refresh_rate);
}

TEST(DisplayManagerTest, UpdatesDisplayIfNewlyCalculatedRefreshRateChanged) {
  DisplayManager manager;
  const double refresh_rate = 60;
  std::vector<std::unique_ptr<flutter::Display>> displays;
  displays.push_back(std::make_unique<flutter::Display>(refresh_rate));
  manager.HandleDisplayUpdates(flutter::DisplayUpdateType::kStartup,
                               std::move(displays));
  fml::TimePoint vsync_start =
      fml::TimePoint::FromEpochDelta(fml::TimeDelta::FromMilliseconds(0));
  fml::TimePoint vsync_target =
      fml::TimePoint::FromEpochDelta(fml::TimeDelta::FromMilliseconds(8));

  manager.UpdateRefreshRateIfNecessary(
      flutter::DisplayUpdateType::kUpdateRefreshRate, vsync_start,
      vsync_target);

  const double kRefreshRateCompareEpsilon = 0.1;
  ASSERT_NEAR(manager.GetMainDisplayRefreshRate(),
              round(1 / (vsync_target - vsync_start).ToSecondsF()),
              kRefreshRateCompareEpsilon);
}

}  // namespace testing
}  // namespace flutter

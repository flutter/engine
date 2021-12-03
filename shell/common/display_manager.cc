// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/common/display_manager.h"

#include "flutter/fml/logging.h"
#include "flutter/fml/macros.h"

namespace flutter {

DisplayManager::DisplayManager() = default;

DisplayManager::~DisplayManager() = default;

double DisplayManager::GetMainDisplayRefreshRate() const {
  std::scoped_lock lock(displays_mutex_);
  if (displays_.empty()) {
    return kUnknownDisplayRefreshRate;
  } else {
    return displays_[0]->GetRefreshRate();
  }
}

void DisplayManager::HandleDisplayUpdates(
    DisplayUpdateType update_type,
    std::vector<std::unique_ptr<Display>> displays) {
  std::scoped_lock lock(displays_mutex_);
  CheckDisplayConfiguration(displays);
  switch (update_type) {
    case DisplayUpdateType::kStartup:
      FML_CHECK(displays_.empty());
      displays_ = std::move(displays);
      return;
    case DisplayUpdateType::kUpdateRefreshRate:
      displays_.insert(displays_.begin(),
                       std::make_move_iterator(displays.begin()),
                       std::make_move_iterator(displays.end()));
      break;
    default:
      FML_CHECK(false) << "Unknown DisplayUpdateType.";
  }
}

void DisplayManager::CheckDisplayConfiguration(
    const std::vector<std::unique_ptr<Display>>& displays) const {
  FML_CHECK(!displays.empty());
  if (displays.size() > 1) {
    for (auto& display : displays) {
      FML_CHECK(display->GetDisplayId().has_value());
    }
  }
}

void DisplayManager::UpdateRefreshRateIfNecessary(
    DisplayUpdateType update_type,
    fml::TimePoint vsync_start_time,
    fml::TimePoint frame_target_time) {
  auto is_frame_rate_same = [](double fr1, double fr2) {
    const double kRefreshRateCompareEpsilon = 1;
    return fabs(fr1 - fr2) < kRefreshRateCompareEpsilon;
  };

  double new_frame_rate =
      round((1 / (frame_target_time - vsync_start_time).ToSecondsF()));
  if (!is_frame_rate_same(GetMainDisplayRefreshRate(), new_frame_rate)) {
    std::vector<std::unique_ptr<flutter::Display>> displays;
    displays.push_back(std::make_unique<flutter::Display>(new_frame_rate));
    HandleDisplayUpdates(update_type, std::move(displays));
  }
}

}  // namespace flutter

// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/common/display_manager.h"

#include "flutter/fml/logging.h"
#include "flutter/fml/macros.h"

namespace flutter {

DisplayManager::DisplayManager() = default;

DisplayManager::~DisplayManager() = default;

double DisplayManager::GetMainDisplayRefreshRate() {
  std::scoped_lock lock(displays_mutex_);
  if (displays_.empty()) {
    return kUnknownDisplayRefreshRate;
  } else {
    return displays_[0].GetRefreshRate();
  }
}

void DisplayManager::HandleDisplayUpdates(DisplayUpdateType update_type,
                                          std::vector<Display> displays) {
  std::scoped_lock lock(displays_mutex_);
  switch (update_type) {
    case DisplayUpdateType::kStartup:
      FML_CHECK(displays_.empty());
      displays_ = displays;
    default:
      FML_LOG(ERROR) << "Unknown DisplayUpdateType.";
  }
}

}  // namespace flutter

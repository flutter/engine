// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_COMMON_DISPLAY_MANAGER_H_
#define FLUTTER_SHELL_COMMON_DISPLAY_MANAGER_H_

#include <mutex>
#include <vector>

#include "flutter/shell/common/display.h"

namespace flutter {

/// The update type parameter that is passed to
/// `DisplayManager::HandleDisplayUpdates`.
enum class DisplayUpdateType {
  /// `flutter::Display`s that were active during start-up. A display is
  /// considered active if:
  ///    1. The frame buffer hardware is connected.
  ///    2. The display is drawable, e.g. it isn't being mirrored from another
  ///    connected display or sleeping.
  kStartup
};

/// Manages lifecycle of the connected displays. This class is thread-safe.
class DisplayManager {
 public:
  DisplayManager();

  ~DisplayManager();

  /// Returns the display refresh rate of the main display. In cases where there
  /// is only one display connected, it will return that. We do not yer support
  /// cases where there are multiple displays.
  ///
  /// When there are no registered displays, it returns
  /// `kUnknownDisplayRefreshRate`.
  double GetMainDisplayRefreshRate();

  /// Handles the display updates.
  void HandleDisplayUpdates(DisplayUpdateType update_type,
                            std::vector<Display> displays);

 private:
  /// Guards `displays_` vector.
  std::mutex displays_mutex_;
  std::vector<Display> displays_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_COMMON_DISPLAY_MANAGER_H_

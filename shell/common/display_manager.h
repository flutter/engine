// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_COMMON_DISPLAY_MANAGER_H_
#define FLUTTER_SHELL_COMMON_DISPLAY_MANAGER_H_

#include <math.h>
#include <mutex>
#include <vector>

#include "flutter/fml/time/time_point.h"
#include "flutter/shell/common/display.h"

namespace flutter {

/// The update type parameter that is passed to
/// `DisplayManager::HandleDisplayUpdates`.
enum class DisplayUpdateType {
  /// `flutter::Display`s that were active during start-up. A display is
  /// considered active if:
  ///    1. The frame buffer hardware is connected.
  ///    2. The display is drawable, e.g. it isn't being mirrored from another
  ///       connected display or sleeping.
  kStartup,

  /// The `flutter::Display` that were active and a new frame indicates that the
  /// refresh rate might be updated.
  kUpdateRefreshRate,
};

/// Manages lifecycle of the connected displays. This class is thread-safe.
class DisplayManager {
 public:
  DisplayManager();

  ~DisplayManager();

  /// Returns the display refresh rate of the main display. In cases where there
  /// is only one display connected, it will return that. We do not yet support
  /// cases where there are multiple displays.
  ///
  /// When there are no registered displays, it returns
  /// `kUnknownDisplayRefreshRate`.
  double GetMainDisplayRefreshRate() const;

  /// Handles the display updates.
  void HandleDisplayUpdates(DisplayUpdateType update_type,
                            std::vector<std::unique_ptr<Display>> displays);

  /// Reports the current frame timing to the |DisplayManager|
  void UpdateRefreshRate(DisplayUpdateType update_type,
                         fml::TimePoint vsync_start_time,
                         fml::TimePoint frame_target_time);

 private:
  /// Guards `displays_` vector.
  mutable std::mutex displays_mutex_;
  std::vector<std::unique_ptr<Display>> displays_;

  /// Checks that the provided display configuration is valid. Currently this
  /// ensures that all the displays have an id in the case there are multiple
  /// displays. In case where there is a single display, it is valid for the
  /// display to not have an id.
  void CheckDisplayConfiguration(
      const std::vector<std::unique_ptr<Display>>& displays) const;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_COMMON_DISPLAY_MANAGER_H_

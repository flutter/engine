// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_COMMON_DISPLAY_H_
#define FLUTTER_SHELL_COMMON_DISPLAY_H_

namespace flutter {

/// Unique IO per display that is stable until the Flutter application restarts.
/// See also: `flutter::Display`
typedef uint64_t DisplayId;

/// Some devices do not support multiple displays, in such cases the embedding
/// might not provide a device identifier. This value is used as the device id
/// in such cases. There must only be one screen in such cases.
static constexpr DisplayId kSingleScreenDeviceId = 0;

/// To be used when the display refresh rate is unknown. This is in frames per
/// second.
static constexpr double kUnknownDisplayRefreshRate = 0;

/// Display refers to a graphics hardware system consisting of a framebuffer,
/// typically a monitor or a screen. This class holds the various display
/// settings.
class Display {
 public:
  Display(DisplayId display_id, double refresh_rate)
      : display_id_(display_id), refresh_rate_(refresh_rate) {}

  ~Display() = default;

  // Get the display's maximum refresh rate in the unit of frame per second.
  // Return `kUnknownDisplayRefreshRate` if the refresh rate is unknown.
  double GetRefreshRate() const { return refresh_rate_; }

  /// Returns the `DisplayId` of the display.
  DisplayId GetDisplayId() const { return display_id_; }

 private:
  DisplayId display_id_;
  double refresh_rate_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_COMMON_DISPLAY_H_

// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_ANDROID_DISPLAY_H_
#define FLUTTER_SHELL_PLATFORM_ANDROID_DISPLAY_H_

#include <cstdint>

#include "flutter/fml/macros.h"
#include "flutter/shell/common/display.h"
#include "flutter/shell/platform/android/display_refresh_listener.h"

namespace flutter {

/// A |Display| that listens to refresh rate changes.
class AndroidDisplay : public Display, public DisplayRefreshListener::Delegate {
 public:
  explicit AndroidDisplay(double refresh_rate);
  ~AndroidDisplay() = default;

  // |Display|
  double GetRefreshRate() const override { return refresh_rate_; };

 private:
  // |DisplayRefreshListener::Delegate|
  void OnDisplayRefreshUpdated(int64_t vsync_period_nanos) override;

  DisplayRefreshListener listener_;
  double refresh_rate_;

  FML_DISALLOW_COPY_AND_ASSIGN(AndroidDisplay);
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_ANDROID_DISPLAY_H_

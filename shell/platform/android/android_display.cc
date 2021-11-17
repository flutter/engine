// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/android/android_display.h"

namespace flutter {

AndroidDisplay::AndroidDisplay(double refresh_rate)
    : Display(refresh_rate), listener_(*this), refresh_rate_(refresh_rate) {}

void AndroidDisplay::OnDisplayRefreshUpdated(int64_t vsync_period_nanos) {
  refresh_rate_ = 1000000000 / vsync_period_nanos;
}

}  // namespace flutter

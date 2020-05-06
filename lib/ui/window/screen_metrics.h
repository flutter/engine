// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_WINDOW_SCREEN_METRICS_H_
#define FLUTTER_LIB_UI_WINDOW_SCREEN_METRICS_H_

#include <stdint.h>

#include <string>

namespace flutter {

struct ScreenMetrics {
  ScreenMetrics() = default;
  ScreenMetrics(const ScreenMetrics& other) = default;

  // Create a ScreenMetrics instance.
  ScreenMetrics(int64_t screen_id,
                double p_device_pixel_ratio,
                double p_physical_left,
                double p_physical_top,
                double p_physical_width,
                double p_physical_height,
                double p_physical_padding_top,
                double p_physical_padding_right,
                double p_physical_padding_bottom,
                double p_physical_padding_left,
                double p_physical_view_inset_top,
                double p_physical_view_inset_right,
                double p_physical_view_inset_bottom,
                double p_physical_view_inset_left,
                double p_physical_system_gesture_inset_top,
                double p_physical_system_gesture_inset_right,
                double p_physical_system_gesture_inset_bottom,
                double p_physical_system_gesture_inset_left);

  // Create a ScreenMetrics instance without system gesture insets.
  ScreenMetrics(int64_t screen_id,
                double p_device_pixel_ratio,
                double p_physical_left,
                double p_physical_top,
                double p_physical_width,
                double p_physical_height,
                double p_physical_padding_top,
                double p_physical_padding_right,
                double p_physical_padding_bottom,
                double p_physical_padding_left,
                double p_physical_view_inset_top,
                double p_physical_view_inset_right,
                double p_physical_view_inset_bottom,
                double p_physical_view_inset_left);

  ScreenMetrics(int64_t screen_id,
                double p_device_pixel_ratio,
                double p_physical_left,
                double p_physical_top,
                double p_physical_width,
                double p_physical_height);

  int64_t screen_id = -1;
  double device_pixel_ratio = 1.0;
  double physical_left = 0;
  double physical_top = 0;
  double physical_width = 0;
  double physical_height = 0;
  double physical_padding_top = 0;
  double physical_padding_right = 0;
  double physical_padding_bottom = 0;
  double physical_padding_left = 0;
  double physical_view_inset_top = 0;
  double physical_view_inset_right = 0;
  double physical_view_inset_bottom = 0;
  double physical_view_inset_left = 0;
  double physical_system_gesture_inset_top = 0;
  double physical_system_gesture_inset_right = 0;
  double physical_system_gesture_inset_bottom = 0;
  double physical_system_gesture_inset_left = 0;
};

}  // namespace flutter

#endif  // FLUTTER_LIB_UI_WINDOW_SCREEN_METRICS_H_

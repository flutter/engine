// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/window/screen_metrics.h"

#include "flutter/fml/logging.h"

namespace flutter {

ScreenMetrics::ScreenMetrics(int64_t p_screen_id,
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
                             double p_physical_system_gesture_inset_left)
    : screen_id(p_screen_id),
      device_pixel_ratio(p_device_pixel_ratio),
      physical_left(p_physical_left),
      physical_top(p_physical_top),
      physical_width(p_physical_width),
      physical_height(p_physical_height),
      physical_padding_top(p_physical_padding_top),
      physical_padding_right(p_physical_padding_right),
      physical_padding_bottom(p_physical_padding_bottom),
      physical_padding_left(p_physical_padding_left),
      physical_view_inset_top(p_physical_view_inset_top),
      physical_view_inset_right(p_physical_view_inset_right),
      physical_view_inset_bottom(p_physical_view_inset_bottom),
      physical_view_inset_left(p_physical_view_inset_left),
      physical_system_gesture_inset_top(p_physical_system_gesture_inset_top),
      physical_system_gesture_inset_right(
          p_physical_system_gesture_inset_right),
      physical_system_gesture_inset_bottom(
          p_physical_system_gesture_inset_bottom),
      physical_system_gesture_inset_left(p_physical_system_gesture_inset_left) {
  // Ensure we don't have nonsensical dimensions.
  FML_DCHECK(device_pixel_ratio > 0);
  FML_DCHECK(physical_width >= 0);
  FML_DCHECK(physical_height >= 0);
}

ScreenMetrics::ScreenMetrics(int64_t p_screen_id,
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
                             double p_physical_view_inset_left)
    : screen_id(p_screen_id),
      device_pixel_ratio(p_device_pixel_ratio),
      physical_left(p_physical_left),
      physical_top(p_physical_top),
      physical_width(p_physical_width),
      physical_height(p_physical_height),
      physical_padding_top(p_physical_padding_top),
      physical_padding_right(p_physical_padding_right),
      physical_padding_bottom(p_physical_padding_bottom),
      physical_padding_left(p_physical_padding_left),
      physical_view_inset_top(p_physical_view_inset_top),
      physical_view_inset_right(p_physical_view_inset_right),
      physical_view_inset_bottom(p_physical_view_inset_bottom),
      physical_view_inset_left(p_physical_view_inset_left) {
  // Ensure we don't have nonsensical dimensions.
  FML_DCHECK(device_pixel_ratio > 0);
  FML_DCHECK(physical_width >= 0);
  FML_DCHECK(physical_height >= 0);
}

ScreenMetrics::ScreenMetrics(int64_t p_screen_id,
                             double p_device_pixel_ratio,
                             double p_physical_left,
                             double p_physical_top,
                             double p_physical_width,
                             double p_physical_height)
    : screen_id(p_screen_id),
      device_pixel_ratio(p_device_pixel_ratio),
      physical_left(p_physical_left),
      physical_top(p_physical_top),
      physical_width(p_physical_width),
      physical_height(p_physical_height) {
  // Ensure we don't have nonsensical dimensions.
  FML_DCHECK(device_pixel_ratio > 0);
  FML_DCHECK(physical_width >= 0);
  FML_DCHECK(physical_height >= 0);
}

}  // namespace flutter

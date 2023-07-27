// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/window/window.h"

#include <utility>

#include "third_party/tonic/converter/dart_converter.h"
#include "third_party/tonic/dart_args.h"
#include "third_party/tonic/logging/dart_invoke.h"
#include "third_party/tonic/typed_data/dart_byte_data.h"

namespace flutter {

Window::Window(tonic::DartPersistentValue& library,
               int64_t window_id,
               ViewportMetrics metrics)
    : library_(library),
      window_id_(window_id),
      viewport_metrics_(std::move(metrics)) {}

Window::~Window() {}

void Window::AddView() {
  std::shared_ptr<tonic::DartState> dart_state = library_.dart_state().lock();
  if (!dart_state) {
    return;
  }
  tonic::DartState::Scope scope(dart_state);
  tonic::CheckAndHandleError(tonic::DartInvokeField(
      library_.value(), "_addView",
      {
          tonic::ToDart(window_id_),
          tonic::ToDart(viewport_metrics_.device_pixel_ratio),
          tonic::ToDart(viewport_metrics_.physical_width),
          tonic::ToDart(viewport_metrics_.physical_height),
          tonic::ToDart(viewport_metrics_.physical_padding_top),
          tonic::ToDart(viewport_metrics_.physical_padding_right),
          tonic::ToDart(viewport_metrics_.physical_padding_bottom),
          tonic::ToDart(viewport_metrics_.physical_padding_left),
          tonic::ToDart(viewport_metrics_.physical_view_inset_top),
          tonic::ToDart(viewport_metrics_.physical_view_inset_right),
          tonic::ToDart(viewport_metrics_.physical_view_inset_bottom),
          tonic::ToDart(viewport_metrics_.physical_view_inset_left),
          tonic::ToDart(viewport_metrics_.physical_system_gesture_inset_top),
          tonic::ToDart(viewport_metrics_.physical_system_gesture_inset_right),
          tonic::ToDart(viewport_metrics_.physical_system_gesture_inset_bottom),
          tonic::ToDart(viewport_metrics_.physical_system_gesture_inset_left),
          tonic::ToDart(viewport_metrics_.physical_touch_slop),
          tonic::ToDart(viewport_metrics_.physical_display_features_bounds),
          tonic::ToDart(viewport_metrics_.physical_display_features_type),
          tonic::ToDart(viewport_metrics_.physical_display_features_state),
          tonic::ToDart(viewport_metrics_.display_id),
      }));
}

void Window::UpdateWindowMetrics(const ViewportMetrics& metrics) {
  viewport_metrics_ = metrics;

  std::shared_ptr<tonic::DartState> dart_state = library_.dart_state().lock();
  if (!dart_state) {
    return;
  }
  tonic::DartState::Scope scope(dart_state);
  tonic::CheckAndHandleError(tonic::DartInvokeField(
      library_.value(), "_updateWindowMetrics",
      {
          tonic::ToDart(window_id_),
          tonic::ToDart(viewport_metrics_.device_pixel_ratio),
          tonic::ToDart(viewport_metrics_.physical_width),
          tonic::ToDart(viewport_metrics_.physical_height),
          tonic::ToDart(viewport_metrics_.physical_padding_top),
          tonic::ToDart(viewport_metrics_.physical_padding_right),
          tonic::ToDart(viewport_metrics_.physical_padding_bottom),
          tonic::ToDart(viewport_metrics_.physical_padding_left),
          tonic::ToDart(viewport_metrics_.physical_view_inset_top),
          tonic::ToDart(viewport_metrics_.physical_view_inset_right),
          tonic::ToDart(viewport_metrics_.physical_view_inset_bottom),
          tonic::ToDart(viewport_metrics_.physical_view_inset_left),
          tonic::ToDart(viewport_metrics_.physical_system_gesture_inset_top),
          tonic::ToDart(viewport_metrics_.physical_system_gesture_inset_right),
          tonic::ToDart(viewport_metrics_.physical_system_gesture_inset_bottom),
          tonic::ToDart(viewport_metrics_.physical_system_gesture_inset_left),
          tonic::ToDart(viewport_metrics_.physical_touch_slop),
          tonic::ToDart(viewport_metrics_.physical_display_features_bounds),
          tonic::ToDart(viewport_metrics_.physical_display_features_type),
          tonic::ToDart(viewport_metrics_.physical_display_features_state),
          tonic::ToDart(viewport_metrics_.display_id),
      }));
}

}  // namespace flutter

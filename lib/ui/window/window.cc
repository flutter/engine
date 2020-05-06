// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/window/window.h"

#include "lib/ui/window/viewport_metrics.h"
#include "third_party/tonic/converter/dart_converter.h"
#include "third_party/tonic/dart_args.h"
#include "third_party/tonic/logging/dart_invoke.h"

namespace flutter {

Window::Window(ViewportMetrics metrics) : viewport_metrics_(metrics) {}

Window::~Window() {}

void Window::UpdateWindowMetrics(const tonic::DartPersistentValue& library,
                                 const ViewportMetrics& metrics) {
  viewport_metrics_ = metrics;

  std::shared_ptr<tonic::DartState> dart_state = library.dart_state().lock();
  if (!dart_state) {
    return;
  }
  tonic::DartState::Scope scope(dart_state);
  tonic::LogIfError(tonic::DartInvokeField(
      library.value(), "_updateWindowMetrics",
      {
          tonic::ToDart(metrics.device_pixel_ratio),
          tonic::ToDart(metrics.physical_width),
          tonic::ToDart(metrics.physical_height),
          tonic::ToDart(metrics.physical_depth),
          tonic::ToDart(metrics.physical_padding_top),
          tonic::ToDart(metrics.physical_padding_right),
          tonic::ToDart(metrics.physical_padding_bottom),
          tonic::ToDart(metrics.physical_padding_left),
          tonic::ToDart(metrics.physical_view_inset_top),
          tonic::ToDart(metrics.physical_view_inset_right),
          tonic::ToDart(metrics.physical_view_inset_bottom),
          tonic::ToDart(metrics.physical_view_inset_left),
          tonic::ToDart(metrics.physical_system_gesture_inset_top),
          tonic::ToDart(metrics.physical_system_gesture_inset_right),
          tonic::ToDart(metrics.physical_system_gesture_inset_bottom),
          tonic::ToDart(metrics.physical_system_gesture_inset_left),
      }));
}

}  // namespace flutter

// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/window/window.h"

#include "third_party/tonic/converter/dart_converter.h"
#include "third_party/tonic/dart_args.h"
#include "third_party/tonic/logging/dart_invoke.h"

namespace flutter {

Window::Window(int window_id, int screen_id)
    : window_id_(window_id), screen_id_(screen_id) {}

Window::~Window() {}

void Window::DidCreateIsolate() {
  library_.Set(tonic::DartState::Current(),
               Dart_LookupLibrary(tonic::ToDart("dart:ui")));
  UpdateWindowMetrics(viewport_metrics_);
}

void Window::UpdateWindowMetrics(const ViewportMetrics& metrics) {
  viewport_metrics_ = metrics;

  std::shared_ptr<tonic::DartState> dart_state = library_.dart_state().lock();
  if (!dart_state)
    return;
  tonic::DartState::Scope scope(dart_state);
  tonic::LogIfError(tonic::DartInvokeField(
      library_.value(), "_updateWindowMetrics",
      {
          tonic::ToDart(window_id_),
          tonic::ToDart(screen_id_),
          tonic::ToDart(metrics.physical_top),
          tonic::ToDart(metrics.physical_left),
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

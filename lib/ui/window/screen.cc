// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/window/screen.h"

#include "lib/ui/window/screen_metrics.h"
#include "third_party/tonic/converter/dart_converter.h"
#include "third_party/tonic/dart_args.h"
#include "third_party/tonic/logging/dart_invoke.h"

namespace flutter {
Screen::Screen(ScreenMetrics metrics) : screen_metrics_(metrics) {}

Screen::~Screen() {}

void Screen::DidCreateIsolate() {
  library_.Set(tonic::DartState::Current(),
               Dart_LookupLibrary(tonic::ToDart("dart:ui")));
  UpdateScreenMetrics(screen_metrics_);
}

void Screen::UpdateScreenMetrics(const ScreenMetrics& metrics) {
  screen_metrics_ = metrics;

  std::shared_ptr<tonic::DartState> dart_state = library_.dart_state().lock();
  if (!dart_state) {
    return;
  }
  tonic::DartState::Scope scope(dart_state);
  tonic::LogIfError(tonic::DartInvokeField(
      library_.value(), "_updateScreenMetrics",
      {
          tonic::ToDart(metrics.screen_id),
          tonic::ToDart(metrics.screen_name),
          tonic::ToDart(metrics.physical_left),
          tonic::ToDart(metrics.physical_top),
          tonic::ToDart(metrics.physical_width),
          tonic::ToDart(metrics.physical_height),
          tonic::ToDart(metrics.device_pixel_ratio),
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

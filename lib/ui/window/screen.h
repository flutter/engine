// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_WINDOW_SCREEN_H_
#define FLUTTER_LIB_UI_WINDOW_SCREEN_H_

#include <string>
#include <unordered_map>
#include <vector>

#include "flutter/lib/ui/window/screen_metrics.h"
#include "third_party/tonic/dart_persistent_value.h"

namespace flutter {

class Screen final {
 public:
  explicit Screen(ScreenMetrics metrics);

  ~Screen();

  const ScreenMetrics& screen_metrics() { return screen_metrics_; }

  int screen_id() { return screen_metrics_.screen_id; }

  void UpdateScreenMetrics(const tonic::DartPersistentValue& library,
                           const ScreenMetrics& metrics);

 private:
  ScreenMetrics screen_metrics_;
};

}  // namespace flutter

#endif  // FLUTTER_LIB_UI_WINDOW_SCREEN_H_

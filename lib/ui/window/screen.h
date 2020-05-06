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
  explicit Screen(int id);

  ~Screen();

  void DidCreateIsolate();

  const ScreenMetrics& screen_metrics() { return screen_metrics_; }

  int screen_id() { return screen_id_; }

  void UpdateScreenMetrics(const ScreenMetrics& metrics);

 private:
  tonic::DartPersistentValue library_;
  int screen_id_;
  ScreenMetrics screen_metrics_;
};

}  // namespace flutter

#endif  // FLUTTER_LIB_UI_WINDOW_SCREEN_H_

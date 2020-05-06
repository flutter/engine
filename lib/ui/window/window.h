// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_WINDOW_WINDOW_H_
#define FLUTTER_LIB_UI_WINDOW_WINDOW_H_

#include <string>
#include <unordered_map>
#include <vector>

#include "flutter/lib/ui/window/platform_message.h"
#include "flutter/lib/ui/window/pointer_data_packet.h"
#include "flutter/lib/ui/window/viewport_metrics.h"
#include "third_party/skia/include/gpu/GrContext.h"
#include "third_party/tonic/dart_persistent_value.h"

namespace flutter {
class Window final {
 public:
  explicit Window(ViewportMetrics metrics);

  ~Window();

  int64_t window_id() const { return viewport_metrics_.view_id; }

  const ViewportMetrics& viewport_metrics() const { return viewport_metrics_; }

  void UpdateWindowMetrics(const tonic::DartPersistentValue& library,
                           const ViewportMetrics& metrics);

 private:
  ViewportMetrics viewport_metrics_;
};

}  // namespace flutter

#endif  // FLUTTER_LIB_UI_WINDOW_WINDOW_H_

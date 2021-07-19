// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/android/surface/snapshot_surface_producer.h"

namespace flutter {

AndroidSnapshotSurfaceProducer::AndroidSnapshotSurfaceProducer(
    std::shared_ptr<AndroidSurface> android_surface)
    : android_surface_(std::move(android_surface)) {}

std::unique_ptr<Surface>
AndroidSnapshotSurfaceProducer::CreateSnapshotSurface() {
  if (!android_surface_) {
    return nullptr;
  }
  return android_surface_->CreatePbufferSurface();
}

}  // namespace flutter

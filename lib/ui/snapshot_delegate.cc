// Copyright 2021 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/snapshot_delegate.h"

#include "flutter/fml/trace_event.h"
#include "third_party/skia/include/core/SkSurface.h"
#include "third_party/skia/include/gpu/GrDirectContext.h"

namespace flutter {

sk_sp<SkImage> SnapshotDelegate::TextureToImage(
    GrDirectContext* context,
    std::shared_ptr<SnapshotDelegate::GpuSnapshot> gpu_snapshot) {
  sk_sp<SkImage> new_device_snapshot;
  {
    TRACE_EVENT0("flutter", "MoveBackendTexture");
    new_device_snapshot = SkImage::MakeFromTexture(
        context, gpu_snapshot->backing_texture, kTopLeft_GrSurfaceOrigin,
        gpu_snapshot->color_type, gpu_snapshot->alpha_type,
        gpu_snapshot->color_space);
  }

  if (new_device_snapshot == nullptr) {
    return nullptr;
  }

  {
    TRACE_EVENT0("flutter", "DeviceHostTransfer");
    if (auto raster_image = new_device_snapshot->makeRasterImage()) {
      return raster_image;
    }
  }

  return nullptr;
}

}  // namespace flutter

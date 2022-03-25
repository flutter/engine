// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/offscreen_utils.h"

#include "flutter/fml/logging.h"
#include "include/core/SkSize.h"

namespace flutter {
sk_sp<SkData> GetRasterData(sk_sp<SkSurface> offscreen_surface,
                            bool compressed) {
  // Prepare an image from the surface, this image may potentially be on th GPU.
  auto potentially_gpu_snapshot = offscreen_surface->makeImageSnapshot();
  if (!potentially_gpu_snapshot) {
    FML_LOG(ERROR) << "Screenshot: unable to make image screenshot";
    return nullptr;
  }

  // Copy the GPU image snapshot into CPU memory.
  auto cpu_snapshot = potentially_gpu_snapshot->makeRasterImage();
  if (!cpu_snapshot) {
    FML_LOG(ERROR) << "Screenshot: unable to make raster image";
    return nullptr;
  }

  // If the caller want the pixels to be compressed, there is a Skia utility to
  // compress to PNG. Use that.
  if (compressed) {
    return cpu_snapshot->encodeToData();
  }

  // Copy it into a bitmap and return the same.
  SkPixmap pixmap;
  if (!cpu_snapshot->peekPixels(&pixmap)) {
    FML_LOG(ERROR) << "Screenshot: unable to obtain bitmap pixels";
    return nullptr;
  }
  return SkData::MakeWithCopy(pixmap.addr32(), pixmap.computeByteSize());
}

OffscreenSurface::OffscreenSurface(const SkISize size) {
  // The caveat here is that if the app is backgrounded this might not
  // be the ideal approach: https://github.com/flutter/flutter/issues/73675.
  // Given that `enable_leaf_layer_tracing` is a debug mode only feature
  // and is expected to run only when the app is in foreground this is ok.
  SkImageInfo image_info = SkImageInfo::MakeN32Premul(
      size.width(), size.height(), SkColorSpace::MakeSRGB());
  offscreen_surface_ = SkSurface::MakeRaster(image_info);
}

sk_sp<SkData> OffscreenSurface::GetRasterData(bool compressed) {
  return flutter::GetRasterData(offscreen_surface_, compressed);
}

sk_sp<SkData> OffscreenSurface::GetRasterDataAsBase64(bool compressed) {
  sk_sp<SkData> data = GetRasterData(compressed);
  size_t size = data->size();
  size_t b64_size = SkBase64::Encode(data->data(), size + 1, nullptr);
  sk_sp<SkData> b64_data = SkData::MakeUninitialized(b64_size);
  SkBase64::Encode(data->data(), data->size(), b64_data->writable_data());
  static_cast<char*>(b64_data->writable_data())[size] = 0;
  return b64_data;
}

SkCanvas* OffscreenSurface::GetCanvas() {
  return offscreen_surface_->getCanvas();
}

}  // namespace flutter

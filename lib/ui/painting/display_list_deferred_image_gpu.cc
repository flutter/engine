// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/painting/display_list_deferred_image_gpu.h"

namespace flutter {

sk_sp<DlDeferredImageGPU> DlDeferredImageGPU::Make(
    SkISize size,
    fml::RefPtr<fml::TaskRunner> raster_task_runner) {
  return sk_sp<DlDeferredImageGPU>(
      new DlDeferredImageGPU(size, std::move(raster_task_runner)));
}

DlDeferredImageGPU::DlDeferredImageGPU(
    SkISize size,
    fml::RefPtr<fml::TaskRunner> raster_task_runner)
    : size_(size), raster_task_runner_(std::move(raster_task_runner)) {}

// |DlImage|
DlDeferredImageGPU::~DlDeferredImageGPU() {
  fml::TaskRunner::RunNowOrPostTask(raster_task_runner_,
                                    [image = std::move(image_)]() {});
}

// |DlImage|
sk_sp<SkImage> DlDeferredImageGPU::skia_image() const {
  return image_;
};

// |DlImage|
std::shared_ptr<impeller::Texture> DlDeferredImageGPU::impeller_texture()
    const {
  return nullptr;
}

// |DlImage|
bool DlDeferredImageGPU::isTextureBacked() const {
  if (auto image = skia_image()) {
    return image->isTextureBacked();
  }
  return false;
}

// |DlImage|
SkISize DlDeferredImageGPU::dimensions() const {
  return size_;
}

// |DlImage|
size_t DlDeferredImageGPU::GetApproximateByteSize() const {
  // This call is accessed on the UI thread, and image_ may not be available
  // yet. The image is not mipmapped and it's created using N32 pixels, so this
  // is safe.
  if (size_.isEmpty()) {
    return sizeof(this);
  }
  return sizeof(this) + size_.width() * size_.height() * 4;
}

void DlDeferredImageGPU::set_image(sk_sp<SkImage> image) {
  FML_DCHECK(image);
  FML_DCHECK(image->dimensions() == size_);
  image_ = std::move(image);
}

void DlDeferredImageGPU::set_error(const std::string& error) {
  FML_DCHECK(!image_);
  std::scoped_lock lock(error_mutex_);
  error_ = std::move(error);
}

std::optional<std::string> DlDeferredImageGPU::get_error() const {
  std::scoped_lock lock(error_mutex_);
  return error_;
}

void DlDeferredImageGPU::OnGrContextDestroyed() {
  fml::TaskRunner::RunNowOrPostTask(raster_task_runner_,
                                    [image = std::move(image_)]() {});
  set_error("context destroyed");
}

}  // namespace flutter

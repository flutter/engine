// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/painting/display_list_deferred_image_gpu.h"

namespace flutter {

sk_sp<DlDeferredImageGPU> DlDeferredImageGPU::Make(
    SkISize size,
    fml::RefPtr<fml::TaskRunner> raster_task_runner,
    fml::RefPtr<SkiaUnrefQueue> unref_queue) {
  return sk_sp<DlDeferredImageGPU>(new DlDeferredImageGPU(
      size, std::move(raster_task_runner), std::move(unref_queue)));
}

DlDeferredImageGPU::DlDeferredImageGPU(
    SkISize size,
    fml::RefPtr<fml::TaskRunner> raster_task_runner,
    fml::RefPtr<SkiaUnrefQueue> unref_queue)
    : size_(size),
      raster_task_runner_(std::move(raster_task_runner)),
      unref_queue_(std::move(unref_queue)) {}

// |DlImage|
DlDeferredImageGPU::~DlDeferredImageGPU() {
  fml::TaskRunner::RunNowOrPostTask(
      raster_task_runner_,
      [image = std::move(image_), unref_queue = std::move(unref_queue_),
       texture_registry = std::move(texture_registry_)]() {
        if (image) {
          if (texture_registry) {
            texture_registry->UnregisterContextDestroyedListener(
                image->uniqueID());
          }

          GrBackendTexture texture = image->releaseBackendTexture(true);
          if (!texture.isValid()) {
            return;
          }

          unref_queue->DeleteTexture(std::move(texture));
        }
      });
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

void DlDeferredImageGPU::set_image(sk_sp<SkImage> image,
                                   TextureRegistry* texture_registry) {
  FML_DCHECK(raster_task_runner_->RunsTasksOnCurrentThread());
  FML_DCHECK(image);
  FML_DCHECK(image->dimensions() == size_);
  image_ = std::move(image);
  texture_registry_ = texture_registry;
}

void DlDeferredImageGPU::set_error(const std::string& error) {
  std::scoped_lock lock(error_mutex_);
  error_ = std::move(error);
}

std::optional<std::string> DlDeferredImageGPU::get_error() const {
  std::scoped_lock lock(error_mutex_);
  return error_;
}

void DlDeferredImageGPU::OnGrContextDestroyed() {
  FML_DCHECK(raster_task_runner_->RunsTasksOnCurrentThread());
  if (image_) {
    set_error("context destroyed");
    GrBackendTexture texture = image_->releaseBackendTexture(true);
    image_.reset();
    if (!texture.isValid()) {
      return;
    }
    unref_queue_->DeleteTexture(std::move(texture));
  }
  unref();  // balance call in Picture::RasterizeToImageSync. Might delete this.
}

}  // namespace flutter

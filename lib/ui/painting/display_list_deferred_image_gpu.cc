// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/painting/display_list_deferred_image_gpu.h"
#include "display_list_deferred_image_gpu.h"

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
  std::scoped_lock lock(image_wrapper_mutex_);
  fml::TaskRunner::RunNowOrPostTask(
      raster_task_runner_, [image_wrapper = std::move(image_wrapper_),
                            unref_queue = std::move(unref_queue_)]() {
        if (!image_wrapper) {
          return;
        }
        auto image = image_wrapper->image;
        if (image) {
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
  std::scoped_lock lock(image_wrapper_mutex_);
  return image_wrapper_ ? image_wrapper_->image : nullptr;
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

void DlDeferredImageGPU::set_image(
    sk_sp<SkImage> image,
    std::shared_ptr<TextureRegistry> texture_registry) {
  FML_DCHECK(raster_task_runner_->RunsTasksOnCurrentThread());
  FML_DCHECK(image);
  FML_DCHECK(image->dimensions() == size_);
  std::scoped_lock lock(image_wrapper_mutex_);
  image_wrapper_ = std::make_shared<ImageWrapper>(
      std::move(image), raster_task_runner_, unref_queue_);
  texture_registry_ = std::move(texture_registry);
  texture_registry_->RegisterContextDestroyedListener(image_wrapper_);
}

void DlDeferredImageGPU::set_error(const std::string& error) {
  std::scoped_lock lock(error_mutex_);
  error_ = std::move(error);
}

std::optional<std::string> DlDeferredImageGPU::get_error() const {
  std::scoped_lock lock(error_mutex_);
  return error_;
}

DlDeferredImageGPU::ImageWrapper::ImageWrapper(
    sk_sp<SkImage> p_image,
    fml::RefPtr<fml::TaskRunner> p_raster_task_runner,
    fml::RefPtr<SkiaUnrefQueue> p_unref_queue)
    : image(std::move(p_image)),
      raster_task_runner(std::move(p_raster_task_runner)),
      unref_queue(std::move(p_unref_queue)) {}

void DlDeferredImageGPU::ImageWrapper::OnGrContextDestroyed() {
  FML_DCHECK(raster_task_runner->RunsTasksOnCurrentThread());
  if (image) {
    GrBackendTexture texture = image->releaseBackendTexture(true);
    image.reset();
    if (!texture.isValid()) {
      return;
    }
    unref_queue->DeleteTexture(std::move(texture));
  }
}

}  // namespace flutter

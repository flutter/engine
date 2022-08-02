// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/painting/display_list_deferred_image_gpu.h"

#include <atomic>

#include "third_party/skia/include/core/SkColorSpace.h"

namespace flutter {

sk_sp<DlDeferredImageGPU> DlDeferredImageGPU::Make(
    const SkImageInfo& image_info,
    fml::RefPtr<fml::TaskRunner> raster_task_runner,
    fml::RefPtr<SkiaUnrefQueue> unref_queue) {
  return sk_sp<DlDeferredImageGPU>(new DlDeferredImageGPU(
      image_info, std::move(raster_task_runner), std::move(unref_queue)));
}

DlDeferredImageGPU::DlDeferredImageGPU(
    const SkImageInfo& image_info,
    fml::RefPtr<fml::TaskRunner> raster_task_runner,
    fml::RefPtr<SkiaUnrefQueue> unref_queue)
    : image_info_(image_info),
      raster_task_runner_(std::move(raster_task_runner)),
      unref_queue_(std::move(unref_queue)) {}

// |DlImage|
DlDeferredImageGPU::~DlDeferredImageGPU() {
  auto image_wrapper = std::atomic_load(&image_wrapper_);
  fml::TaskRunner::RunNowOrPostTask(
      raster_task_runner_, [image_wrapper = std::move(image_wrapper),
                            unref_queue = std::move(unref_queue_),
                            texture_registry = std::move(texture_registry_)]() {
        if (!image_wrapper) {
          return;
        }
        if (texture_registry) {
          texture_registry->UnregisterContextDestroyedListener(
              reinterpret_cast<uintptr_t>(image_wrapper.get()));
        }
        auto texture = image_wrapper->texture();
        if (texture.isValid()) {
          unref_queue->DeleteTexture(std::move(texture));
        }
      });
}

// |DlImage|
sk_sp<SkImage> DlDeferredImageGPU::skia_image() const {
  auto image_wrapper = std::atomic_load(&image_wrapper_);
  return image_wrapper ? image_wrapper->CreateSkiaImage(image_info_) : image_;
};

// |DlImage|
std::shared_ptr<impeller::Texture> DlDeferredImageGPU::impeller_texture()
    const {
  return nullptr;
}

// |DlImage|
bool DlDeferredImageGPU::isTextureBacked() const {
  if (auto image_wrapper = std::atomic_load(&image_wrapper_)) {
    return image_wrapper->isTextureBacked();
  }
  return false;
}

// |DlImage|
SkISize DlDeferredImageGPU::dimensions() const {
  return image_info_.dimensions();
}

// |DlImage|
size_t DlDeferredImageGPU::GetApproximateByteSize() const {
  // This call is accessed on the UI thread, and image_ may not be available
  // yet. The image is not mipmapped.
  return sizeof(this) + image_info_.computeMinByteSize();
}

void DlDeferredImageGPU::set_texture(
    const GrBackendTexture& texture,
    sk_sp<GrDirectContext> context,
    std::shared_ptr<TextureRegistry> texture_registry) {
  FML_DCHECK(raster_task_runner_->RunsTasksOnCurrentThread());
  FML_DCHECK(texture.isValid());
  FML_DCHECK(context);

  image_ = nullptr;
  auto image_wrapper = std::make_shared<ImageWrapper>(
      texture, std::move(context), raster_task_runner_, unref_queue_);
  texture_registry_ = std::move(texture_registry);
  texture_registry_->RegisterContextDestroyedListener(
      reinterpret_cast<uintptr_t>(image_wrapper.get()), image_wrapper);
  std::atomic_store(&image_wrapper_, image_wrapper);
}

void DlDeferredImageGPU::set_image(sk_sp<SkImage> image) {
  FML_DCHECK(image && !image->isTextureBacked());
  image_ = std::move(image);
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
    const GrBackendTexture& texture,
    sk_sp<GrDirectContext> context,
    fml::RefPtr<fml::TaskRunner> raster_task_runner,
    fml::RefPtr<SkiaUnrefQueue> unref_queue)
    : texture_(texture),
      context_(std::move(context)),
      raster_task_runner_(std::move(raster_task_runner)),
      unref_queue_(std::move(unref_queue)) {}

void DlDeferredImageGPU::ImageWrapper::OnGrContextDestroyed() {
  FML_DCHECK(raster_task_runner_->RunsTasksOnCurrentThread());

  if (texture_.isValid()) {
    unref_queue_->DeleteTexture(std::move(texture_));
    texture_ = GrBackendTexture();
  }
  context_.reset();
}

sk_sp<SkImage> DlDeferredImageGPU::ImageWrapper::CreateSkiaImage(
    const SkImageInfo& image_info) {
  FML_DCHECK(raster_task_runner_->RunsTasksOnCurrentThread());
  FML_DCHECK(texture_.isValid());
  FML_DCHECK(context_.get());

  return SkImage::MakeFromTexture(
      context_.get(), texture_, kTopLeft_GrSurfaceOrigin,
      image_info.colorType(), image_info.alphaType(),
      image_info.refColorSpace());
}

bool DlDeferredImageGPU::ImageWrapper::isTextureBacked() {
  return texture_.isValid();
}

}  // namespace flutter

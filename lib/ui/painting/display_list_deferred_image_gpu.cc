// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/painting/display_list_deferred_image_gpu.h"

namespace flutter {

sk_sp<DlDeferredImageGPU> DlDeferredImageGPU::Make(
    SkISize size,
    fml::RefPtr<fml::TaskRunner> raster_task_runner,
    fml::RefPtr<fml::TaskRunner> io_task_runner,
    fml::WeakPtr<IOManager> io_manager) {
  return sk_sp<DlDeferredImageGPU>(
      new DlDeferredImageGPU(size, std::move(raster_task_runner),
                             std::move(io_task_runner), std::move(io_manager)));
}

DlDeferredImageGPU::DlDeferredImageGPU(
    SkISize size,
    fml::RefPtr<fml::TaskRunner> raster_task_runner,
    fml::RefPtr<fml::TaskRunner> io_task_runner,
    fml::WeakPtr<IOManager> io_manager)
    : size_(size),
      raster_task_runner_(std::move(raster_task_runner)),
      io_task_runner_(std::move(io_task_runner)),
      io_manager_(std::move(io_manager)) {}

namespace {
// Call on the Raster task runner.
void DeleteImage(sk_sp<SkImage> image,
                 fml::RefPtr<fml::TaskRunner> io_task_runner,
                 fml::WeakPtr<IOManager> io_manager) {
  FML_DCHECK(io_task_runner);
  if (!image) {
    return;
  }

  GrBackendTexture texture = image->releaseBackendTexture(true);
  if (!texture.isValid()) {
    return;
  }
  // Need to latch on the IO runner so that during shutdown, the IO Manager
  // and its resource context do not get collected.
  fml::AutoResetWaitableEvent latch;
  fml::TaskRunner::RunNowOrPostTask(
      io_task_runner, [&latch, texture = std::move(texture),
                       io_manager = std::move(io_manager)]() {
        if (!io_manager) {
          FML_DCHECK(false);
          latch.Signal();
          return;
        }
        auto resource_context = io_manager->GetResourceContext();
        if (!resource_context) {
          FML_DCHECK(false);
          latch.Signal();
          return;
        }
        resource_context.get()->deleteBackendTexture(texture);
        latch.Signal();
      });
  latch.Wait();
}
}  // namespace

// |DlImage|
DlDeferredImageGPU::~DlDeferredImageGPU() {
  fml::TaskRunner::RunNowOrPostTask(
      raster_task_runner_,
      [image = std::move(image_), io_task_runner = std::move(io_task_runner_),
       io_manager = std::move(io_manager_),
       texture_registry = std::move(texture_registry_)]() {
        if (texture_registry && image) {
          texture_registry->UnregisterContextDestroyedListener(
              image->uniqueID());
        }

        DeleteImage(std::move(image), std::move(io_task_runner),
                    std::move(io_manager));
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
  set_error("context destroyed");
  DeleteImage(std::move(image_), io_task_runner_, io_manager_);
  unref();  // balance call in Picture::RasterizeToImageSync. Might delete this.
}

}  // namespace flutter

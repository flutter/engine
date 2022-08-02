// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_PAINTING_DISPLAY_LIST_DEFERRED_IMAGE_GPU_H_
#define FLUTTER_LIB_UI_PAINTING_DISPLAY_LIST_DEFERRED_IMAGE_GPU_H_

#include <mutex>

#include "flutter/common/graphics/texture.h"
#include "flutter/display_list/display_list_image.h"
#include "flutter/flow/skia_gpu_object.h"
#include "flutter/fml/macros.h"
#include "flutter/fml/memory/weak_ptr.h"
#include "flutter/lib/ui/io_manager.h"

namespace flutter {

class DlDeferredImageGPU final : public DlImage {
 public:
  static sk_sp<DlDeferredImageGPU> Make(
      const SkImageInfo& image_info,
      fml::RefPtr<fml::TaskRunner> raster_task_runner,
      fml::RefPtr<SkiaUnrefQueue> unref_queue);

  // |DlImage|
  ~DlDeferredImageGPU() override;

  // |DlImage|
  // This method is only safe to call from the raster thread.
  // The image created here must not be added to the unref queue.
  sk_sp<SkImage> skia_image() const override;

  // |DlImage|
  std::shared_ptr<impeller::Texture> impeller_texture() const override;

  // |DlImage|
  bool isTextureBacked() const override;

  // |DlImage|
  SkISize dimensions() const override;

  // |DlImage|
  virtual size_t GetApproximateByteSize() const override;

  // This method must only be called from the raster thread.
  void set_texture(const GrBackendTexture& texture,
                   sk_sp<GrDirectContext> context,
                   std::shared_ptr<TextureRegistry> texture_registry);

  void set_image(sk_sp<SkImage> image);

  // This method is safe to call from any thread.
  void set_error(const std::string& error);

  // |DlImage|
  // This method is safe to call from any thread.
  std::optional<std::string> get_error() const override;

  const SkImageInfo& image_info() { return image_info_; }

  // |DlImage|
  OwningContext owning_context() const override {
    return OwningContext::kRaster;
  }

 private:
  class ImageWrapper final : public ContextDestroyedListener {
   public:
    ImageWrapper(const GrBackendTexture& texture,
                 sk_sp<GrDirectContext> context,
                 fml::RefPtr<fml::TaskRunner> raster_task_runner,
                 fml::RefPtr<SkiaUnrefQueue> unref_queue);

    sk_sp<SkImage> CreateSkiaImage(const SkImageInfo& image_info);

    const GrBackendTexture& texture() { return texture_; }

    bool isTextureBacked();

   private:
    GrBackendTexture texture_;
    sk_sp<GrDirectContext> context_;
    fml::RefPtr<fml::TaskRunner> raster_task_runner_;
    fml::RefPtr<SkiaUnrefQueue> unref_queue_;

    // |ContextDestroyedListener|
    void OnGrContextDestroyed() override;
  };

  const SkImageInfo image_info_;
  fml::RefPtr<fml::TaskRunner> raster_task_runner_;
  fml::RefPtr<SkiaUnrefQueue> unref_queue_;
  // Must be accessed using atomics.
  // TODO(dnfield): When c++20 is available use std::atomic<std::shared_ptr>
  std::shared_ptr<ImageWrapper> image_wrapper_;
  std::shared_ptr<TextureRegistry> texture_registry_;
  // May be used if this image is not texture backed.
  sk_sp<SkImage> image_;
  mutable std::mutex error_mutex_;
  std::optional<std::string> error_;

  DlDeferredImageGPU(const SkImageInfo& image_info,
                     fml::RefPtr<fml::TaskRunner> raster_task_runner,
                     fml::RefPtr<SkiaUnrefQueue> unref_queue);

  FML_DISALLOW_COPY_AND_ASSIGN(DlDeferredImageGPU);
};

}  // namespace flutter

#endif  // FLUTTER_LIB_UI_PAINTING_DISPLAY_LIST_DEFERRED_IMAGE_GPU_H_

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

namespace flutter {

class DlDeferredImageGPU final : public DlImage,
                                 public ContextDestroyedListener {
 public:
  static sk_sp<DlDeferredImageGPU> Make(
      SkISize size,
      fml::RefPtr<fml::TaskRunner> raster_task_runner);

  // |DlImage|
  ~DlDeferredImageGPU() override;

  // |DlImage|
  // This method is only safe to call from the raster thread.
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
  void set_image(sk_sp<SkImage> image);

  // This method is safe to call from any thread.
  void set_error(const std::string& error);

  // |DlImage|
  // This method is safe to call from any thread.
  std::optional<std::string> get_error() const override;

  // |DlImage|
  OwningContext owning_context() const override {
    return OwningContext::kRaster;
  }

  void OnGrContextDestroyed();

 private:
  SkISize size_;
  fml::RefPtr<fml::TaskRunner> raster_task_runner_;
  sk_sp<SkImage> image_;
  mutable std::mutex error_mutex_;
  std::optional<std::string> error_;

  DlDeferredImageGPU(SkISize size,
                     fml::RefPtr<fml::TaskRunner> raster_task_runner);

  FML_DISALLOW_COPY_AND_ASSIGN(DlDeferredImageGPU);
};

}  // namespace flutter

#endif  // FLUTTER_LIB_UI_PAINTING_DISPLAY_LIST_DEFERRED_IMAGE_GPU_H_

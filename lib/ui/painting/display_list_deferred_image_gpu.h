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

class DlDeferredImageGPU final : public DlImage,
                                 public ContextDestroyedListener {
 public:
  static sk_sp<DlDeferredImageGPU> Make(
      SkISize size,
      fml::RefPtr<fml::TaskRunner> raster_task_runner,
      fml::RefPtr<fml::TaskRunner> io_task_runner,
      fml::WeakPtr<IOManager> io_manager);

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
  void set_image(sk_sp<SkImage> image, TextureRegistry* texture_registry);

  // This method is safe to call from any thread.
  void set_error(const std::string& error);

  // |DlImage|
  // This method is safe to call from any thread.
  std::optional<std::string> get_error() const override;

  // |DlImage|
  OwningContext owning_context() const override {
    return OwningContext::kRaster;
  }

  // |ContextDestroyedListener|
  void OnGrContextDestroyed() override;

 private:
  SkISize size_;
  fml::RefPtr<fml::TaskRunner> raster_task_runner_;
  fml::RefPtr<fml::TaskRunner> io_task_runner_;
  fml::WeakPtr<IOManager> io_manager_;
  sk_sp<SkImage> image_;
  TextureRegistry* texture_registry_ = nullptr;
  mutable std::mutex error_mutex_;
  std::optional<std::string> error_;

  DlDeferredImageGPU(SkISize size,
                     fml::RefPtr<fml::TaskRunner> raster_task_runner,
                     fml::RefPtr<fml::TaskRunner> io_task_runner,
                     fml::WeakPtr<IOManager> io_manager);

  FML_DISALLOW_COPY_AND_ASSIGN(DlDeferredImageGPU);
};

}  // namespace flutter

#endif  // FLUTTER_LIB_UI_PAINTING_DISPLAY_LIST_DEFERRED_IMAGE_GPU_H_

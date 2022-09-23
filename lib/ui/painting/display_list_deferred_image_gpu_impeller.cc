// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/painting/display_list_deferred_image_gpu_impeller.h"

namespace flutter {

sk_sp<DlDeferredImageGPUImpeller> DlDeferredImageGPUImpeller::Make(
    std::shared_ptr<LayerTree> layer_tree,
    const SkISize& size,
    fml::WeakPtr<SnapshotDelegate> snapshot_delegate,
    fml::RefPtr<fml::TaskRunner> raster_task_runner) {
  sk_sp<DlDeferredImageGPUImpeller> image(new DlDeferredImageGPUImpeller(size));
  fml::TaskRunner::RunNowOrPostTask(
      raster_task_runner, [image, size, layer_tree = std::move(layer_tree),
                           snapshot_delegate = std::move(snapshot_delegate)] {
        if (!snapshot_delegate) {
          return;
        }

        auto snapshot = snapshot_delegate->MakeRasterSnapshot(
            layer_tree->Flatten(SkRect::MakeWH(size.width(), size.height()),
                                snapshot_delegate->GetTextureRegistry()),
            size);
        image->set_texture(snapshot->impeller_texture());
      });
  return image;
}

sk_sp<DlDeferredImageGPUImpeller> DlDeferredImageGPUImpeller::Make(
    sk_sp<DisplayList> display_list,
    const SkISize& size,
    fml::WeakPtr<SnapshotDelegate> snapshot_delegate,
    fml::RefPtr<fml::TaskRunner> raster_task_runner) {
  sk_sp<DlDeferredImageGPUImpeller> image(new DlDeferredImageGPUImpeller(size));
  fml::TaskRunner::RunNowOrPostTask(
      raster_task_runner, [image, size, display_list = std::move(display_list),
                           snapshot_delegate = std::move(snapshot_delegate)] {
        if (!snapshot_delegate) {
          return;
        }

        auto snapshot =
            snapshot_delegate->MakeRasterSnapshot(display_list, size);
        image->set_texture(snapshot->impeller_texture());
      });
  return image;
}

DlDeferredImageGPUImpeller::DlDeferredImageGPUImpeller(const SkISize& size)
    : size_(size) {}

// |DlImage|
DlDeferredImageGPUImpeller::~DlDeferredImageGPUImpeller() = default;

// |DlImage|
sk_sp<SkImage> DlDeferredImageGPUImpeller::skia_image() const {
  return nullptr;
};

// |DlImage|
std::shared_ptr<impeller::Texture>
DlDeferredImageGPUImpeller::impeller_texture() const {
  return texture_;
}

void DlDeferredImageGPUImpeller::set_texture(
    std::shared_ptr<impeller::Texture> texture) {
  FML_DCHECK(!texture_);
  texture_ = std::move(texture);
}

// |DlImage|
bool DlDeferredImageGPUImpeller::isOpaque() const {
  // Impeller doesn't currently implement opaque alpha types.
  return false;
}

// |DlImage|
bool DlDeferredImageGPUImpeller::isTextureBacked() const {
  return true;
}

// |DlImage|
SkISize DlDeferredImageGPUImpeller::dimensions() const {
  return size_;
}

// |DlImage|
size_t DlDeferredImageGPUImpeller::GetApproximateByteSize() const {
  auto size = sizeof(DlDeferredImageGPUImpeller);
  if (texture_) {
    size += texture_->GetTextureDescriptor().GetByteSizeOfBaseMipLevel();
  } else {
    size += size_.width() * size_.height() * 4;
  }
  return size;
}

}  // namespace flutter

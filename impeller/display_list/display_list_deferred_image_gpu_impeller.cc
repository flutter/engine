// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/display_list/display_list_deferred_image_gpu_impeller.h"
#include "display_list_deferred_image_gpu_impeller.h"

namespace impeller {

sk_sp<DlDeferredImageGPUImpeller> DlDeferredImageGPUImpeller::Make(
    const SkISize& size) {
  return sk_sp<DlDeferredImageGPUImpeller>(
      new DlDeferredImageGPUImpeller(size));
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

}  // namespace impeller

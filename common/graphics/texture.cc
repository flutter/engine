// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/common/graphics/texture.h"

namespace flutter {

Texture::Texture(int64_t id) : id_(id) {}

Texture::~Texture() = default;

TextureRegistry::TextureRegistry() = default;

void TextureRegistry::RegisterTexture(std::shared_ptr<Texture> texture) {
  if (!texture) {
    return;
  }
  mapping_[texture->Id()] = texture;
}

void TextureRegistry::RegisterGpuImage(sk_sp<DlDeferredImageGPU> image) {
  if (!image) {
    return;
  }
  auto* skia_image = image->skia_image();
  if (!skia_image) {
    return;
  }
  images_[skia_image->uniqueID()] = skia_image;
}

void TextureRegistry::UnregisterTexture(int64_t id) {
  auto found = mapping_.find(id);
  if (found == mapping_.end()) {
    return;
  }
  found->second->OnTextureUnregistered();
  mapping_.erase(found);
}

void TextureRegistry::UnregisterGpuImage(uint32_t id) {
  auto found = images_.find(id);
  if (found == images_.end()) {
    return;
  }
  images_.erase(found);
}

void TextureRegistry::OnGrContextCreated() {
  for (auto& it : mapping_) {
    it.second->OnGrContextCreated();
  }
}

void TextureRegistry::OnGrContextDestroyed() {
  for (auto& it : mapping_) {
    it.second->OnGrContextDestroyed();
  }

  for (auto& it : images_) {
    it.second->OnGrContextCreated();
  }
}

std::shared_ptr<Texture> TextureRegistry::GetTexture(int64_t id) {
  auto it = mapping_.find(id);
  return it != mapping_.end() ? it->second : nullptr;
}

}  // namespace flutter

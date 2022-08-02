// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/common/graphics/texture.h"

namespace flutter {

ContextDestroyedListener::ContextDestroyedListener() = default;

ContextDestroyedListener::~ContextDestroyedListener() = default;

Texture::Texture(int64_t id) : id_(id) {}

Texture::~Texture() = default;

TextureRegistry::TextureRegistry() = default;

void TextureRegistry::RegisterTexture(std::shared_ptr<Texture> texture) {
  if (!texture) {
    return;
  }
  mapping_[texture->Id()] = texture;
}

void TextureRegistry::RegisterContextDestroyedListener(
    uintptr_t id,
    std::weak_ptr<ContextDestroyedListener> image) {
  images_[id] = std::move(image);
}

void TextureRegistry::UnregisterTexture(int64_t id) {
  auto found = mapping_.find(id);
  if (found == mapping_.end()) {
    return;
  }
  found->second->OnTextureUnregistered();
  mapping_.erase(found);
}

void TextureRegistry::UnregisterContextDestroyedListener(uintptr_t id) {
  images_.erase(id);
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

  for (const auto& [id, weak_image] : images_) {
    if (auto image = weak_image.lock()) {
      image->OnGrContextDestroyed();
    }
  }
  images_.clear();
}

std::shared_ptr<Texture> TextureRegistry::GetTexture(int64_t id) {
  auto it = mapping_.find(id);
  return it != mapping_.end() ? it->second : nullptr;
}

}  // namespace flutter

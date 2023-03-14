// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/common/graphics/texture.h"

namespace flutter {

ContextListener::ContextListener() = default;

ContextListener::~ContextListener() = default;

Texture::Texture(int64_t id) : id_(id) {}

Texture::~Texture() = default;

TextureRegistry::TextureRegistry() : image_counter_(0) {}

void TextureRegistry::RegisterTexture(const std::shared_ptr<Texture>& texture) {
  if (!texture) {
    return;
  }
  mapping_[texture->Id()] = texture;
}

void TextureRegistry::RegisterContextListener(
    uintptr_t id,
    std::weak_ptr<ContextListener> image) {
  size_t next_id = image_counter_.fetch_add(1);
  auto const result = image_indices_.insert({id, next_id});
  if (!result.second) {
    ordered_images_.erase(result.first->second);
    result.first->second = next_id;
  }
  ordered_images_[next_id] = {next_id, std::move(image)};
}

void TextureRegistry::UnregisterTexture(int64_t id) {
  auto found = mapping_.find(id);
  if (found == mapping_.end()) {
    return;
  }
  found->second->OnTextureUnregistered();
  mapping_.erase(found);
}

void TextureRegistry::UnregisterContextListener(uintptr_t id) {
  ordered_images_.erase(image_indices_[id]);
  image_indices_.erase(id);
}

void TextureRegistry::OnGrContextCreated() {
  for (auto& it : mapping_) {
    it.second->OnGrContextCreated();
  }

  // Calling OnGrContextCreated may result in a subsequent call to
  // RegisterContextListener from the listener, which may modify the map.
  std::vector<
      std::pair<size_t, std::pair<uintptr_t, std::weak_ptr<ContextListener>>>>
      ordered_images(ordered_images_.begin(), ordered_images_.end());

  for (const auto& [id, pair] : ordered_images) {
    auto index_id = pair.first;
    auto weak_image = pair.second;
    if (auto image = weak_image.lock()) {
      image->OnGrContextCreated();
    } else {
      image_indices_.erase(index_id);
      ordered_images_.erase(id);
    }
  }
}

void TextureRegistry::OnGrContextDestroyed() {
  for (auto& it : mapping_) {
    it.second->OnGrContextDestroyed();
  }

  for (const auto& [id, pair] : ordered_images_) {
    auto index_id = pair.first;
    auto weak_image = pair.second;
    if (auto image = weak_image.lock()) {
      image->OnGrContextDestroyed();
    } else {
      image_indices_.erase(index_id);
      ordered_images_.erase(id);
    }
  }
}

std::shared_ptr<Texture> TextureRegistry::GetTexture(int64_t id) {
  auto it = mapping_.find(id);
  return it != mapping_.end() ? it->second : nullptr;
}

}  // namespace flutter

// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "impeller/renderer/render_target.h"

namespace impeller {

/// @brief An implementation of the [RenderTargetAllocator] that caches all
///        allocated texture data for one frame.
///
///        Any textures unused after a frame are immediately discarded. Textures
///        may be used multiple times in the same frame if it is safe to do
///        so. Currently, Only the Vulkan backend has correct tracking to use
///        this feature.
class RenderTargetCache : public RenderTargetAllocator {
 public:
  explicit RenderTargetCache(std::shared_ptr<Allocator> allocator,
                             bool allow_intraframe_use = false);

  ~RenderTargetCache() = default;

  // |RenderTargetAllocator|
  void Start() override;

  // |RenderTargetAllocator|
  void End() override;

  // |RenderTargetAllocator|
  std::shared_ptr<Texture> CreateTexture(
      const TextureDescriptor& desc) override;

  // visible for testing.
  size_t CachedTextureCount() const;

 private:
  struct TextureData {
    bool used_this_frame;
    std::shared_ptr<Texture> texture;
  };

  std::vector<TextureData> texture_data_;
  bool allow_intraframe_use_ = false;

  FML_DISALLOW_COPY_AND_ASSIGN(RenderTargetCache);
};

}  // namespace impeller

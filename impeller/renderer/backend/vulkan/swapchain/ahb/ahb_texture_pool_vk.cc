// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/vulkan/swapchain/ahb/ahb_texture_pool_vk.h"

#include "flutter/fml/trace_event.h"

namespace impeller {

AHBTexturePoolVK::AHBTexturePoolVK(const std::weak_ptr<Context>& context,
                                   android::HardwareBufferDescriptor desc,
                                   size_t max_entries)
    : context_(std::move(context)), desc_(desc), max_entries_(max_entries) {
  if (!desc_.IsAllocatable()) {
    VALIDATION_LOG << "Swapchain image is not allocatable.";
    return;
  }
  is_valid_ = true;
}

AHBTexturePoolVK::~AHBTexturePoolVK() = default;

std::shared_ptr<AHBTextureSourceVK> AHBTexturePoolVK::Pop() {
  {
    Lock lock(pool_mutex_);
    if (!pool_.empty()) {
      auto texture = pool_.front().item;
      pool_.pop_front();
      return texture;
    }
  }
  return CreateTexture();
}

void AHBTexturePoolVK::Push(
    const std::shared_ptr<AHBTextureSourceVK>& texture) {
  if (!texture) {
    return;
  }
  Lock lock(pool_mutex_);
  pool_.push_back(PoolEntry{texture});
  PerformGCLocked();
}

std::shared_ptr<AHBTextureSourceVK> AHBTexturePoolVK::CreateTexture() const {
  TRACE_EVENT0("impeller", "CreateSwapchainTexture");
  auto context = context_.lock();
  if (!context) {
    VALIDATION_LOG << "Context died before image could be created.";
    return nullptr;
  }

  auto ahb = std::make_unique<android::HardwareBuffer>(desc_);
  if (!ahb->IsValid()) {
    VALIDATION_LOG << "Could not create hardware buffer of size: "
                   << desc_.size;
    return nullptr;
  }

  auto ahb_texture_source = std::make_shared<AHBTextureSourceVK>(
      context, std::move(ahb), /*is_swapchain_image=*/true);
  if (!ahb_texture_source->IsValid()) {
    VALIDATION_LOG << "Could not create hardware buffer texture source for "
                      "swapchain image of size: "
                   << desc_.size;
    return nullptr;
  }

  return ahb_texture_source;
}

void AHBTexturePoolVK::PerformGC() {
  Lock lock(pool_mutex_);
  PerformGCLocked();
}

void AHBTexturePoolVK::PerformGCLocked() {
  while (!pool_.empty() && pool_.size() > max_entries_) {
    pool_.pop_front();
  }
}

bool AHBTexturePoolVK::IsValid() const {
  return is_valid_;
}

}  // namespace impeller

// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/core/allocator.h"

#include "impeller/base/validation.h"
#include "impeller/core/device_buffer.h"
#include "impeller/core/formats.h"
#include "impeller/core/range.h"

namespace impeller {

Allocator::Allocator() = default;

Allocator::~Allocator() = default;

std::shared_ptr<DeviceBuffer> Allocator::CreateBufferWithCopy(
    const uint8_t* buffer,
    size_t length) {
  DeviceBufferDescriptor desc;
  desc.size = length;
  desc.storage_mode = StorageMode::kHostVisible;
  auto new_buffer = CreateBuffer(desc);

  if (!new_buffer) {
    return nullptr;
  }

  auto entire_range = Range{0, length};

  if (!new_buffer->CopyHostBuffer(buffer, entire_range)) {
    return nullptr;
  }

  return new_buffer;
}

void Allocator::DidAcquireSurfaceFrame() {
  for (auto& td : data_to_recycle_) {
    td.used_this_frame = false;
  }
}

void Allocator::DidFinishSurfaceFrame() {
  std::vector<TextureData> retain;

  for (auto td : data_to_recycle_) {
    if (td.used_this_frame) {
      retain.push_back(td);
    }
  }
  data_to_recycle_.clear();
  data_to_recycle_.insert(data_to_recycle_.end(), retain.begin(), retain.end());
}

void Allocator::SetEnableRenderTargetTextureCache(bool value) {
  enable_render_target_texture_cache_ = value;
}

std::shared_ptr<DeviceBuffer> Allocator::CreateBufferWithCopy(
    const fml::Mapping& mapping) {
  return CreateBufferWithCopy(mapping.GetMapping(), mapping.GetSize());
}

std::shared_ptr<DeviceBuffer> Allocator::CreateBuffer(
    const DeviceBufferDescriptor& desc) {
  return OnCreateBuffer(desc);
}

std::shared_ptr<Texture> Allocator::CreateTexture(
    const TextureDescriptor& desc) {
  const auto max_size = GetMaxTextureSizeSupported();
  if (desc.size.width > max_size.width || desc.size.height > max_size.height) {
    VALIDATION_LOG << "Requested texture size " << desc.size
                   << " exceeds maximum supported size of " << max_size;
    return nullptr;
  }

  if (enable_render_target_texture_cache_ && !desc.ignore_cache &&
      desc.storage_mode != StorageMode::kHostVisible &&
      (desc.usage &
       static_cast<TextureUsageMask>(TextureUsage::kRenderTarget))) {
    for (auto& td : data_to_recycle_) {
      const auto other_desc = td.texture->GetTextureDescriptor();
      if (!td.used_this_frame && desc.IsCompatibleWith(other_desc)) {
        td.used_this_frame = true;
        return td.texture;
      }
    }
    auto result = OnCreateTexture(desc);
    data_to_recycle_.push_back({.used_this_frame = true, .texture = result});
    return result;
  }

  return OnCreateTexture(desc);
}

uint16_t Allocator::MinimumBytesPerRow(PixelFormat format) const {
  return BytesPerPixelForPixelFormat(format);
}

}  // namespace impeller
